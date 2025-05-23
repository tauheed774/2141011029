#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

#define PORT 8080
#define OUTPUT_DIR "client_downloads/"

void xor_decrypt(char* data, size_t size, char key = 'K') {
    for (size_t i = 0; i < size; ++i) {
        data[i] ^= key;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    std::string user, pass;
    std::cout << "Username: "; std::getline(std::cin, user);
    std::cout << "Password: "; std::getline(std::cin, pass);
    std::string creds = user + ":" + pass;
    send(sock, creds.c_str(), creds.size(), 0);

    char response[1024] = {0};
    read(sock, response, 1024);
    if (std::string(response) != "AUTH_SUCCESS") {
        std::cout << "Auth failed.\n";
        close(sock);
        return 1;
    }
    std::cout << "Authenticated!\n";

    char filelist[2048] = {0};
    read(sock, filelist, sizeof(filelist));
    std::cout << "\nAvailable Files:\n" << filelist;

    std::string fname;
    std::cout << "\nEnter filename to download: ";
    std::getline(std::cin, fname);
    send(sock, fname.c_str(), fname.size(), 0);

    std::ofstream outfile(std::string(OUTPUT_DIR) + fname, std::ios::binary);
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = read(sock, buffer, sizeof(buffer))) > 0) {
        xor_decrypt(buffer, bytesRead);  
        outfile.write(buffer, bytesRead);
    }
    std::cout << "File downloaded as: " << OUTPUT_DIR << fname << "\n";

    close(sock);
    return 0;
}

