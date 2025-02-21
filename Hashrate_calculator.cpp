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

// Function to ask user preferences
void askUserPreferences() {
    char choice;
    cout << "Do you want to run miners in the background? (y/n): ";
    cin >> choice;
    runInBackground = (choice == 'y' || choice == 'Y');

    cout << "Enter miner runtime in minutes (default 0.5 min / 30 sec): ";
    double inputMinutes;
    cin >> inputMinutes;
    minerRuntime = static_cast<int>(inputMinutes * 60); // Convert to seconds

    if (minerRuntime < 30) {
        cout << "Runtime too short, setting to 30 seconds." << endl;
        minerRuntime = 30;
    }

    cout << "Enter your wallet address for mining (leave blank to skip): ";
    cin.ignore(); // Clear input buffer
    getline(cin, walletAddress);

    if (!walletAddress.empty()) {
        cout << "Mining will use wallet address: " << walletAddress << endl;
    } else {
        cout << "No wallet address provided. Mining will not occur." << endl;
    }
}

// Function to extract hashrate from miner output
double extractHashrate(const string &line) {
    regex hashRegex(R"((\d+(\.\d+)?)\s*(H\/s|KH\/s|MH\/s|GH\/s))");
    smatch match;

    if (regex_search(line, match, hashRegex)) {
        double hashrate = stod(match[1]); // Extract number
        string unit = match[3];           // Extract unit

        // Convert to MH/s
        if (unit == "H/s") hashrate /= 1e6;
        else if (unit == "KH/s") hashrate /= 1e3;
        else if (unit == "GH/s") hashrate *= 1e3;

        return hashrate;
    }
    return -1; // Return -1 if no hashrate found
}

// Function to validate .bat file content
bool validateBatFile(const string &filePath) {
    ifstream batFile(filePath);
    if (!batFile) {
        cerr << "Error: Could not open .bat file for validation: " << filePath << endl;
        return false;
    }

    string line;
    while (getline(batFile, line)) {
        // Basic validation: Check for suspicious commands
        if (line.find("format") != string::npos || line.find("del") != string::npos) {
            cerr << "Warning: Suspicious command found in .bat file: " << filePath << endl;
            return false;
        }
    }

    return true;
}

// Function to run a miner and capture its output
double runMiner(const wstring &batFile) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    // Choose execution mode (background or foreground)
    DWORD creationFlags = runInBackground ? CREATE_NO_WINDOW : 0;

    if (!CreateProcessW(NULL, (LPWSTR)batFile.c_str(), NULL, NULL, FALSE, creationFlags, NULL, NULL, &si, &pi)) {
        wcerr << L"Failed to start miner: " << batFile << endl;
        return -1;
    }

    wcout << L"Started miner: " << batFile << (runInBackground ? L" (Background)" : L" (Foreground)") << endl;

    vector<double> hashrates;
    auto start = high_resolution_clock::now();
    bool startedRecording = false;
    long elapsed = 0;

    // Capture output and extract hashrate
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(500)); // Check output every 500ms

        auto now = high_resolution_clock::now();
        elapsed = duration_cast<seconds>(now - start).count();

        // Simulate reading miner output (replace with actual output capture)
        string simulatedOutput = "Hashrate: 500 H/s"; // Change this to test different formats

        double hashrate = extractHashrate(simulatedOutput);
        if (hashrate > 0) {
            if (elapsed >= 30) { // Only record after 30 seconds
                hashrates.push_back(hashrate);
                cout << "Recorded Hashrate: " << hashrate << " MH/s" << endl;
                startedRecording = true;
            }
        }

        if (startedRecording && elapsed >= minerRuntime) break; // Stop after user-defined runtime
    }

    // Compute average hashrate
    double avgHashrate = 0;
    for (double h : hashrates) avgHashrate += h;
    avgHashrate /= hashrates.size();

    // Close the miner
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    wcout << L"Closed miner: " << batFile << endl;

    return avgHashrate;
}

int main() {
    menu();
    wstring path = fs::current_path().wstring(); // Get the directory where the program is run

    ofstream minerList("available_miners.txt");
    if (!minerList) {
        cerr << "Error: Cannot open available_miners.txt for writing!" << endl;
        return 1;
    }

    // Scan for .bat files and save to available_miners.txt
    for (const auto &entry : fs::directory_iterator(path)) {
        if (entry.path().extension() == L".bat") {
            string batFilePath = entry.path().string();

            // Validate .bat file before adding to the list
            if (validateBatFile(batFilePath)) {
                minerList << batFilePath << endl;
            } else {
                cerr << "Skipping invalid .bat file: " << batFilePath << endl;
            }
        }
    }
    minerList.close();

    // Ask user for execution mode and runtime
    askUserPreferences();

    // Open miner list and process each .bat file
    ifstream minerFile("available_miners.txt");
    ofstream hashOut("hashrates.txt");

    if (!minerFile || !hashOut) {
        cerr << "Error: Could not open necessary files!" << endl;
        return 1;
    }

    string minerPath;
    while (getline(minerFile, minerPath)) {
        // Convert string to wstring for Unicode compatibility
        wstring wideMinerPath(minerPath.begin(), minerPath.end());

        double avgHashrate = runMiner(wideMinerPath);
        if (avgHashrate > 0) {
            hashOut << "Miner: " << minerPath << " -> Average Hashrate: " << avgHashrate << " MH/s" << endl;
            hashOut.flush();
        }
    }

    cout << "All miners processed. Hashrates saved in hashrates.txt." << endl;
    return 0;
}
