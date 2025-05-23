#include<iostream>  
// Includes the input/output stream library for using cout and cin in C++. 
// This allows you to print messages to the console.

#include<sys/sysinfo.h>  
// This is a Linux-specific header file that gives access to system information such as memory, uptime, and CPU load.
// It provides the sysinfo structure and the sysinfo() function.

using namespace std;  
// So that we can use standard C++ functions like cout without prefixing them with "std::".

// Function to display memory information of the system
void displaymemoryinfo(){
    struct sysinfo info;  
    // Declares a variable 'info' of type struct sysinfo which will hold system information like total RAM, free RAM etc.

    if(sysinfo(&info)==0){  
    // Calls the sysinfo() function and passes the address of the 'info' structure.
    // If it returns 0, it means the call was successful.

        cout << "total ram:" << info.totalram/(1024*1024) << "MB\n";  
        // Displays total physical RAM in megabytes.
        // info.totalram is in bytes, so dividing by 1024*1024 converts it to MB.

        cout << "total ram:" << info.totalram/(1021*1024*1024) << "GB\n";  
        // Intends to display total RAM in gigabytes.
        // However, there's a small mistake here: it uses 1021 instead of 1024, which would result in slightly wrong output.

        cout << "total ram:" << info.freeram/(1024*1024) << "MB\n";  
        // Displays free (available) RAM in megabytes.

        cout << "total ram:" << info.freeram/(1021*1024*1024) << "GB\n";  
        // Again tries to show free RAM in gigabytes but uses 1021 instead of 1024.
        // Should be corrected to give accurate GB value.
    }
}

// Main function
int main(){
    displaymemoryinfo();  
    // Calls the function to print the system’s memory stats.

    return 0;  
    // Returns 0 from main to indicate that the program executed successfully.
}
