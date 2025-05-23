#include<iostream>           // For input/output operations like cout
#include<fstream>            // For file handling (to read from /proc/stat)
#include<sstream>            // For parsing strings using stringstream
#include<thread>             // To add delay (sleep) using std::this_thread
#include<chrono>             // For specifying time durations (like seconds)
using namespace std;         // Avoid prefixing standard library classes/functions with std::

// A structure to hold various CPU time fields extracted from /proc/stat
struct CPUData {
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};

CPUData getCPUData() {
    ifstream file("/proc/stat");  // Open the /proc/stat file which holds CPU usage stats
    string line;                  
    CPUData cpu = {};             // Initialize all members of the struct to 0

    if(file.is_open()) {         // If file opened successfully
        getline(file, line);     // Read the first line (starts with "cpu")
        istringstream ss(line);  // Convert the line into a stream for parsing
        string cpuLabel;         // Variable to hold the "cpu" label

        // Extract the fields into the struct (1st token is "cpu", then numeric values)
        ss >> cpuLabel >> cpu.user >> cpu.nice >> cpu.system >> cpu.idle >> cpu.iowait
           >> cpu.irq >> cpu.softirq >> cpu.steal >> cpu.guest >> cpu.guest_nice;
    }
    return cpu;  // Return the filled CPUData struct
}

double calculateCPUUsage(CPUData prev, CPUData current) {
    // Calculate idle time in both snapshots
    long prevldle = prev.idle + prev.iowait;
    long currldle = current.idle + current.iowait;

    // Total CPU time includes all fields
    long prevTotal = prev.user + prev.nice + prev.system + prev.idle + prev.iowait +
                     prev.irq + prev.softirq + prev.steal + prev.guest + prev.guest_nice;

    long currTotal = current.user + current.nice + current.system + current.idle +
                     current.iowait + current.softirq + current.steal; // Missing irq, guest, guest_nice (minor issue)

    long totaldiff = currTotal - prevTotal;  // Difference in total CPU time
    long ideldiff = currldle - prevldle;     // Difference in idle time

    // CPU usage = (active time / total time) * 100
    return (totaldiff - ideldiff) * 100.0 / totaldiff;
}

int main() {
    CPUData cpu = getCPUData();  // Read the current CPU stats

    // Display each field from the CPU stats
    cout << "User Time: " << cpu.user << endl;
    cout << "Nice Time: " << cpu.nice << endl;
    cout << "System Time: " << cpu.system << endl;
    cout << "Idle Time: " << cpu.idle << endl;
    cout << "Iowait Time: " << cpu.iowait << endl;
    cout << "Irq Time: " << cpu.irq << endl;
    cout << "Softirq Time: " << cpu.softirq << endl;
    cout << "Steal Time: " << cpu.steal << endl;
    cout << "Guest Time: " << cpu.guest << endl;
    cout << "Guest Nice Time: " << cpu.guest_nice << endl;

    // Total CPU time (excluding guest_nice in this example)
    long totalcputime = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait +
                        cpu.irq + cpu.softirq + cpu.steal;
    cout << "Total CPU Time: " << totalcputime << endl;

    // Calculate idle and active times separately
    long idel_time = cpu.idle + cpu.iowait;
    long Active_time = totalcputime - cpu.idle;  // Slight inconsistency here: should subtract idle + iowait

    // Take first CPU snapshot
    CPUData prevData = getCPUData();
    this_thread::sleep_for(chrono::seconds(1));  // Wait 1 second to measure time difference
    // Take second CPU snapshot after 1 second
    CPUData currData = getCPUData();

    // Calculate CPU usage based on the difference
    double cpu_usage = calculateCPUUsage(prevData, currData);

    // Print the CPU usage percentage
    cout << cpu_usage << "%" << endl;

    return 0;  // Indicate successful program end
}