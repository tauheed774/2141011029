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
#include <iostream>             // For input and output operations (e.g., std::cout)
#include <fstream>              // For reading files like /proc/[pid]/stat and /proc/uptime
#include <sstream>              // For parsing strings (e.g., line from /proc/[pid]/stat)
#include <vector>               // For storing multiple Processinfo objects
#include <algorithm>            // For sorting the vector of processes
#include <filesystem>           // For iterating through the /proc directory to list PIDs
#include <unistd.h>             // For sysconf, which is used to get clock ticks per second

namespace fs = std::filesystem; // Namespace alias to shorten code for filesystem operations

// Struct to store information about each process
struct Processinfo {
    int pid;                    // Process ID
    std::string name;           // Process name
    double cpuUsage = 0;        // CPU usage in percentage
    long memoryUsage = 0;       // Memory usage in kB (from VmRSS)
};

// Utility function to read a single-line value from a file
std::string readFileValue(const std::string &path) {
    std::ifstream file(path);   // Open file at given path
    std::string value;
    if (file.is_open()) {
        std::getline(file, value);  // Read the first line
    }
    return value;               // Return the read value
}

// Function to get system uptime from /proc/uptime
double getSystemUptime() {
    std::ifstream file("/proc/uptime"); // /proc/uptime contains system uptime in seconds
    double uptime = 0;
    if (file.is_open()) {
        file >> uptime;         // Read the first value (uptime)
    }
    return uptime;
}

// Function to extract process information for a given PID
Processinfo getProcessInfo(int pid, double systemUptime) {
    Processinfo proc;
    proc.pid = pid;             // Set process ID

    // Open /proc/[pid]/stat to extract CPU usage and start time
    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    std::string line;

    long utime = 0, stime = 0, starttime = 0; // Fields to extract from stat file

    if (statFile.is_open()) {
        std::getline(statFile, line); // Read entire line
        std::istringstream ss(line);  // Create stream to parse the line
        std::string token;
        for (int i = 1; ss >> token; ++i) {
            if (i == 2) proc.name = token;              // Process name (with parentheses)
            else if (i == 14) utime = std::stol(token); // User mode time
            else if (i == 15) stime = std::stol(token); // Kernel mode time
            else if (i == 22) starttime = std::stol(token); // Start time after boot
        }
    }

    // Read memory usage (VmRSS) from /proc/[pid]/status
    std::ifstream memFile("/proc/" + std::to_string(pid) + "/status");
    if (memFile.is_open()) {
        std::string key, value, unit;
        while (memFile >> key >> value >> unit) { // Tokenize key, value, unit
            if (key == "VmRSS:") {                // VmRSS = Resident Set Size (actual RAM usage)
                proc.memoryUsage = std::stol(value);
                break;
            }
        }
    }

    // Calculate total CPU time (utime + stime)
    long total_time = utime + stime;

    // Calculate seconds the process has been running
    double seconds = systemUptime - (starttime / sysconf(_SC_CLK_TCK));

    // Calculate CPU usage as a percentage
    if (seconds > 0) {
        proc.cpuUsage = ((total_time / (double)sysconf(_SC_CLK_TCK)) / seconds) * 100.0;
    }

    return proc;
}

// Function to get all running processes by scanning /proc for numeric directory names (PIDs)
std::vector<Processinfo> getAllprocess() {
    std::vector<Processinfo> processes;
    double systemUptime = getSystemUptime(); // Get current system uptime

    // Iterate over directories in /proc
    for (const auto &entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();

            // Only consider directories whose names are all digits (i.e., valid PIDs)
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                int pid = std::stoi(filename); // Convert string to int
                processes.push_back(getProcessInfo(pid, systemUptime)); // Get and store info
            }
        }
    }
    return processes;
}

// Function to sort processes either by CPU or memory usage
void sortProcesses(std::vector<Processinfo> &processes, bool sortByCPU) {
    if (sortByCPU) {
        // Sort in descending order of CPU usage
        std::sort(processes.begin(), processes.end(), [](const Processinfo &a, const Processinfo &b) {
            return a.cpuUsage > b.cpuUsage;
        });
    } else {
        // Sort in descending order of memory usage
        std::sort(processes.begin(), processes.end(), [](const Processinfo &a, const Processinfo &b) {
            return a.memoryUsage > b.memoryUsage;
        });
    }
}

// Main function
int main() {
    // Get all processes
    std::vector<Processinfo> processes = getAllprocess();

    // Sort them based on CPU usage; pass false to sort by memory instead
    sortProcesses(processes, true);

    // Print table header
    std::cout << "PID\tCPU%\tMemory (kB)\tName\n";

    // Print top 10 processes (by CPU or memory depending on above)
    for (size_t i = 0; i < std::min(processes.size(), size_t(10)); ++i) {
        std::cout << processes[i].pid << "\t"
                  << processes[i].cpuUsage << "\t"
                  << processes[i].memoryUsage << "\t"
                  << processes[i].name << "\n";
    }

    return 0;
}

