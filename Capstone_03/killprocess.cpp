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
#include <iostream>              // For input/output (cout, cin)
#include <fstream>              // For reading files like /proc/[pid]/stat
#include <sstream>              // For parsing strings using stringstream
#include <vector>               // To store the list of all processes
#include <algorithm>            // For sorting processes based on CPU or memory usage
#include <filesystem>           // To iterate over directories in /proc
#include <unistd.h>             // For sysconf() and system-related constants like _SC_CLK_TCK
#include <csignal>              // For using kill() function and signal constants like SIGKILL

namespace fs = std::filesystem; // Alias for filesystem namespace to shorten code
using namespace std;            // So we don’t need to prefix std:: repeatedly

// Structure to hold relevant info about a process
struct Processinfo {
    int pid;                    // Process ID
    std::string name;           // Process name
    double cpuUsage = 0;        // CPU usage in percentage
    long memoryUsage = 0;       // Memory usage in kB (VmRSS)
};

// Reads a single-line value from a given file path
std::string readFileValue(const std::string &path) {
    std::ifstream file(path);   // Open file
    std::string value;
    if (file.is_open()) {
        std::getline(file, value); // Read the first line of the file
    }
    return value;               // Return the content
}

// Reads and returns system uptime from /proc/uptime
double getSystemUptime() {
    std::ifstream file("/proc/uptime"); // Open uptime file
    double uptime = 0;
    if (file.is_open()) {
        file >> uptime;         // Read the first value which is uptime in seconds
    }
    return uptime;
}

// Extracts process information for a given PID
Processinfo getProcessInfo(int pid, double systemUptime) {
    Processinfo proc;
    proc.pid = pid;             // Set PID in struct

    std::ifstream statFile("/proc/" + std::to_string(pid) + "/stat");
    std::string line;

    long utime = 0, stime = 0, starttime = 0; // Fields used for CPU calculations

    if (statFile.is_open()) {
        std::getline(statFile, line);        // Read the entire line from stat
        std::istringstream ss(line);         // Stream to parse space-separated values
        std::string token;
        for (int i = 1; ss >> token; ++i) {
            if (i == 2) proc.name = token;           // Process name (in parentheses)
            else if (i == 14) utime = std::stol(token);     // Time in user mode
            else if (i == 15) stime = std::stol(token);     // Time in kernel mode
            else if (i == 22) starttime = std::stol(token); // Start time after system boot
        }
    }

    // Open status file to get memory usage (VmRSS)
    std::ifstream memFile("/proc/" + std::to_string(pid) + "/status");
    if (memFile.is_open()) {
        std::string key, value, unit;
        while (memFile >> key >> value >> unit) {
            if (key == "VmRSS:") {                   // VmRSS = physical memory usage in KB
                proc.memoryUsage = std::stol(value);
                break;
            }
        }
    }

    // Calculate CPU usage
    long total_time = utime + stime;
    double seconds = systemUptime - (starttime / sysconf(_SC_CLK_TCK)); // Time the process has run
    if (seconds > 0) {
        proc.cpuUsage = ((total_time / (double)sysconf(_SC_CLK_TCK)) / seconds) * 100.0;
    }

    return proc; // Return the collected process info
}

// Scans /proc directory to find all numeric directories (which represent PIDs)
std::vector<Processinfo> getAllprocess() {
    std::vector<Processinfo> processes;
    double systemUptime = getSystemUptime(); // Fetch system uptime once

    for (const auto &entry : fs::directory_iterator("/proc")) {
        if (entry.is_directory()) {
            std::string filename = entry.path().filename().string();

            // Check if directory name is all digits (valid PID)
            if (std::all_of(filename.begin(), filename.end(), ::isdigit)) {
                int pid = std::stoi(filename);
                processes.push_back(getProcessInfo(pid, systemUptime)); // Get and store info
            }
        }
    }
    return processes; // Return the list of processes
}

// Sorts the processes based on CPU or memory usage
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
    vector<Processinfo> processes = getAllprocess(); // Get all process info
    sortProcesses(processes, true);                  // Sort by CPU usage (pass false for memory)

    // Print the process table header
    cout << "PID\tCPU%\tMemory (kB)\tName\n";

    // Display top 10 processes
    for (size_t i = 0; i < min(processes.size(), size_t(10)); ++i) {
        cout << processes[i].pid << "\t"
             << processes[i].cpuUsage << "\t"
             << processes[i].memoryUsage << "\t"
             << processes[i].name << "\n";
    }

    // Ask user to enter a PID to terminate
    int targetPid;
    cout << "Enter PID to Kill : " << endl;
    cin >> targetPid;

    // If user enters a PID, attempt to terminate that process using kill()
    if (targetPid) {
        // kill(pid, signal) sends SIGKILL to forcefully terminate the process
        if (kill(targetPid, SIGKILL) == 0) {
            cout << "Process " << targetPid << " terminated successfully" << endl;
        } else {
            perror("Failed to kill Process"); // Print error if kill fails
        }
    }

    return 0; // End program
}
