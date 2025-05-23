//Dispaly CPU & memory usage per process
//sort processes by CPU/memory usages
// Total cputime = usertime+cputime(utime+ctime)
//Total CPUTime = utime + stime;
//Calculate process cpu usage
//CPU usage = (utime+stime)/systemupdate - starttime*100;
//g++ -std=c++17 activeprocess.cpp -o activeprocess

//Memory info is in /proc/[PID]/status
//cat /proc/1/status|grep VmRSS
// VmRSS(Resident Set Size) = Actual Ram used by the process.
//student@D001-37:~/Desktop/2141011082/LOS/Day03$ cat /proc/uptime
//2456.96(total system time) 29285.05(ideal time or total system unused)
#include <iostream>                 // For standard I/O like cout, cin
#include <fstream>                  // For file I/O (reading /proc filesystem)
#include <sstream>                  // For parsing strings using stringstreams
#include <vector>                   // To store list of processes
#include <algorithm>               // To sort the process list
#include <filesystem>              // To iterate over files in /proc directory
#include <unistd.h>                // For sysconf (to get clock ticks per second)
#include <csignal>                 // For signal handling like kill() and SIGKILL
#include <thread>                  // For sleep functionality (std::this_thread::sleep_for)
#include <chrono>                  // For time durations (std::chrono::seconds)

namespace fs = std::filesystem;    // Alias for filesystem to use shorter `fs::` instead of `std::filesystem`
using namespace std;

// Structure to store process information
struct Processinfo {
    int pid;                       // Process ID
    std::string name;              // Process name
    double cpuUsage = 0;           // CPU usage percentage
    long memoryUsage = 0;          // Memory usage in kilobytes
};

// Utility function to read a single line value from a file
std::string readFileValue(const std::string &path) {
    std::ifstream file(path);     // Open the file at the given path
    std::string value;
    if (file.is_open()) {
        std::getline(file, value); // Read first line into value
    }
    return value;                  // Return the string read
}

// Get the system uptime from /proc/uptime
double getSystemUptime() {
    std::ifstream file("/proc/uptime"); // Open uptime file
    double uptime = 0;
    if (file.is_open()) {
        file >> uptime;            // Read first number which is uptime in seconds
    }
    return uptime;
}

// Get information about a single process
Processinfo getProcessInfo(int pid, double systemUptime) {
    Processinfo proc;
    proc.pid = pid;

    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat"); // Read process stat file
    std::string line;
    long utime = 0, stime = 0, starttime = 0;

    if (statFile.is_open()) {
        std::getline(statFile, line);
        std::istringstream ss(line);
        std::string token;
        // Parse tokens to extract specific stat values
        for (int i = 1; ss >> token; ++i) {
            if (i == 2) proc.name = token;                  // Process name
            else if (i == 14) utime = std::stol(token);     // User mode CPU time
            else if (i == 15) stime = std::stol(token);     // Kernel mode CPU time
            else if (i == 22) starttime = std::stol(token); // Start time since boot
        }
    }

    // Read memory usage from /proc/[pid]/status
    std::ifstream memFile("/proc/" + std::to_string(pid) + "/status");
    if (memFile.is_open()) {
        std::string key, value, unit;
        while (memFile >> key >> value >> unit) {
            if (key == "VmRSS:") { // VmRSS is Resident Set Size (actual memory)
                proc.memoryUsage = std::stol(value); // Memory in KB
                break;
            }
        }
    }

    // Calculate CPU usage %
    long total_time = utime + stime;
    double seconds = systemUptime - (starttime / sysconf(_SC_CLK_TCK)); // Process lifetime
    if (seconds > 0) {
        proc.cpuUsage = ((total_time / (double)sysconf(_SC_CLK_TCK)) / seconds) * 100.0;
    }

    return proc;
}

// Get information for all processes in /proc
std::vector<Processinfo> getAllprocess() {
    std::vector<Processinfo> processes;
    double systemUptime = getSystemUptime();

    // Iterate over all directories in /proc
    for (const auto &entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();
            // Check if directory name is a number (i.e., it's a process)
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                int pid = std::stoi(filename);
                processes.push_back(getProcessInfo(pid, systemUptime)); // Get process info
            }
        }
    }
    return processes;
}

// Sort processes either by CPU or memory usage
void sortProcesses(std::vector<Processinfo> &processes, bool sortByCPU) {
    if (sortByCPU) {
        std::sort(processes.begin(), processes.end(), [](const Processinfo &a, const Processinfo &b) {
            return a.cpuUsage > b.cpuUsage;
        });
    } else {
        std::sort(processes.begin(), processes.end(), [](const Processinfo &a, const Processinfo &b) {
            return a.memoryUsage > b.memoryUsage;
        });
    }
}

int main() {
    char input;
    while(true){
        system("clear"); // Clear terminal for live display effect

        vector<Processinfo> processes = getAllprocess(); // Get all running processes
        sortProcesses(processes, true); // true = sort by CPU usage

        // Display top 10 processes
        cout << "PID\tCPU%\tMemory (kB)\tName\n";
        for (size_t i = 0; i < min(processes.size(), size_t(10)); ++i) {
            cout << processes[i].pid << "\t"
            << processes[i].cpuUsage << "\t"
            << processes[i].memoryUsage << "\t"
            << processes[i].name << "\n";
        }

        int targetPid;
        cout <<"Enter PID to Kill : " << endl;
        cin >> targetPid; // Take input from user to terminate a process

        if(targetPid){
            // Send SIGKILL to target process
            if(kill(targetPid,SIGKILL)==0){
                cout << "Process " << targetPid << " terminated successfully" << endl;
            }
            else{
                perror("Failed to kill Process"); // Print error if kill fails
            }
        }

        // Refresh or quit prompt
        cout <<"\nPress 'q' to quit or Enter to refresh : ";
        cin.ignore();          // Ignore leftover newline
        input = getchar();     // Read single character
        if(input == 'q' || input == 'Q')
            break;             // Exit loop if user presses q or Q

        std::this_thread::sleep_for(std::chrono::seconds(0)); // Optional delay (set to 0 here)
    }    

    return 0;
}

