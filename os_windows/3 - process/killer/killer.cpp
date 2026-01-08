#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

std::string wcharToString(const WCHAR* wstr) {
    if (wstr == nullptr) {
        return "";
    }

    int size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size == 0) {
        return "";
    }

    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], size, nullptr, nullptr);

    return result;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool killProcessById(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        std::cerr << "Can't open process with id " << processId << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 0)) {
        std::cerr << "Couldn't terminate process with id " << processId << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    std::cout << "Process with id " << processId << " killed" << std::endl;
    CloseHandle(hProcess);
    return true;
}

int killProcessesByName(const std::string& processName) {
    int killedCount = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot" << std::endl;
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Failed to get first process" << std::endl;
        CloseHandle(hSnapshot);
        return 0;
    }

    std::string targetName = toLower(processName);
    if (targetName.find(".exe") == std::string::npos) {
        targetName += ".exe";
    }

    do {
        std::string currentProcessName = toLower(wcharToString(pe32.szExeFile));

        if (currentProcessName == targetName) {
            if (killProcessById(pe32.th32ProcessID)) {
                killedCount++;
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (killedCount == 0) {
        std::cout << "Process with name " << processName << " isn't found" << std::endl;
    }

    return killedCount;
}

std::string getEnvironmentVariable(const std::string& name) {
    char buffer[32767];
    DWORD result = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));

    if (result == 0 || result > sizeof(buffer)) {
        return "";
    }

    return std::string(buffer);
}

void killProcessesFromEnvironment() {
    std::string procToKill = getEnvironmentVariable("PROC_TO_KILL");

    if (procToKill.empty()) {
        return;
    }

    std::cout << "PROC_TO_KILL:" << procToKill << std::endl;

    std::vector<std::string> processNames = split(procToKill, ',');

    for (const auto& name : processNames) {
        killProcessesByName(name);
    }
}

int main(int argc, char* argv[]) {
    killProcessesFromEnvironment();

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--id") {
            if (i + 1 < argc) {
                try {
                    DWORD processId = std::stoul(argv[i + 1]);
                    std::cout << "Killing process with id " << processId << " -> ";
                    killProcessById(processId);
                    i++;
                }
                catch (const std::exception& e) {
                    std::cerr << "Invalid id: " << argv[i + 1] << std::endl;
                    return 1;
                }
            }
            else {
                std::cerr << "Error: no id after --id" << std::endl;
                return 1;
            }
        }
        else if (arg == "--name") {
            if (i + 1 < argc) {
                std::string processName = argv[i + 1];
                std::cout << "Killing process with name " << processName << " -> ";
                killProcessesByName(processName);
                i++;
            }
            else {
                std::cerr << "Error: no name after --name" << std::endl;
                return 1;
            }
        }
    }

    return 0;
}