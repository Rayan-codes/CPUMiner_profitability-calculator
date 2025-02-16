
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;
using namespace std;
using namespace std::chrono;

// Global settings
bool runInBackground = false; // User choice: Background execution
int minerRuntime = 30;        // Default runtime in seconds (30 sec)
string walletAddress = "";    // Wallet address for mining

// Function to display the menu
void menu() {
    cout << "------------------------------------------------------" << endl;
    cout << "Welcome to The Automated Hashrate Calculator" << endl;
    cout << "*Tip: Run as admin to get the best possible estimate" << endl;
    cout << "------------------------------------------------------" << endl;
    cout << endl;
}
int main(){

  
}
