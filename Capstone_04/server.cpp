#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <cstring>
#include <vector>

#define PORT 8080
#define SHARED_DIR "./shared/"
#define AUTH_FILE "auth.txt"

void xor_encrypt(char* data, size_t size, char key = 'K') {
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= key;
    }
}

bool authenticate(int socket) {
    char buffer[1024] = {0};
    read(socket, buffer, 1024);
    std::string creds(buffer);

    std::ifstream authFile(AUTH_FILE);
    std::string line;
    while (std::getline(authFile, line)) {
        if (line == creds) {
            send(socket, "AUTH_SUCCESS", strlen("AUTH_SUCCESS"), 0);
            return true;
        }
    }
    send(socket, "AUTH_FAIL", strlen("AUTH_FAIL"), 0);
    return false;
}

std::vector<std::string> list_files(const std::string& folder) {
    std::vector<std::string> files;
    DIR* dir = opendir(folder.c_str());
    struct dirent* entry;
    if (dir) {
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_REG) {
                files.push_back(entry->d_name);
            }
        }
        closedir(dir);
    }
    return files;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1, addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    std::cout << "Server listening on port " << PORT << "...\n";

    new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);

    if (!authenticate(new_socket)) {
        std::cout << "Authentication failed. Connection closed.\n";
        close(new_socket);
        close(server_fd);
        return 0;
    }
    std::cout << "Client authenticated.\n";

    auto files = list_files(SHARED_DIR);
    std::string list;
    for (const auto& f : files) list += f + "\n";
    send(new_socket, list.c_str(), list.size(), 0);

    char filebuf[256] = {0};
    read(new_socket, filebuf, 256);
    std::string filename(filebuf);
    std::string filepath = std::string(SHARED_DIR) + filename;

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::string msg = "ERROR: File not found.";
        send(new_socket, msg.c_str(), msg.size(), 0);
    } else {
        char buffer[1024];
        while (!file.eof()) {
            file.read(buffer, sizeof(buffer));
            std::streamsize bytesRead = file.gcount();
            xor_encrypt(buffer, bytesRead);  // Encrypt before sending
            send(new_socket, buffer, bytesRead, 0);
        }
        std::cout << "File sent: " << filename << "\n";
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
