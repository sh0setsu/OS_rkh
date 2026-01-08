#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <algorithm>

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

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool setEnvironmentVariable(const std::string& name, const std::string& value) {
    if (SetEnvironmentVariableA(name.c_str(), value.c_str())) {
        std::cout << "Environment variable '" << name << "' set to: " << value << std::endl;
        return true;
    }
    return false;
}

bool deleteEnvironmentVariable(const std::string& name) {
    if (SetEnvironmentVariableA(name.c_str(), nullptr)) {
        std::cout << "Environment variable '" << name << "' deleted successfully" << std::endl;
        return true;
    }
    return false;
}

bool checkProcessExists(const std::string& processName, DWORD* processId = nullptr) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create process snapshot" << std::endl;
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "Failed to get first process" << std::endl;
        CloseHandle(hSnapshot);
        return false;
    }

    std::string targetName = toLower(processName);
    if (targetName.find(".exe") == std::string::npos) {
        targetName += ".exe";
    }

    bool found = false;
    do {
        std::string currentProcessName = toLower(wcharToString(pe32.szExeFile));
        if (currentProcessName == targetName) {
            found = true;
            if (processId != nullptr) {
                *processId = pe32.th32ProcessID;
            }
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return found;
}

bool runKillerProcess(const std::string& arguments) {
    std::string commandLine = "killer.exe " + arguments;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::cout << "\nRunning: " << commandLine << std::endl;

    if (!CreateProcessA(nullptr,
        const_cast<char*>(commandLine.c_str()),
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi)) {
        std::cerr << "Failed to create killer process. Error code: " << GetLastError() << std::endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    Sleep(500);
    std::cout << std::endl;

    return true;
}

bool startTestProcess(const std::string& processPath, DWORD* outProcessId = nullptr) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(processPath.c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi)) {
        std::cerr << "Failed to start process: " << processPath << ". Error code: " << GetLastError() << std::endl;
        return false;
    }

    if (outProcessId != nullptr) {
        *outProcessId = pi.dwProcessId;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Started process: " << processPath;
    if (outProcessId != nullptr) {
        std::cout << " (PID: " << *outProcessId << ")";
    }
    std::cout << std::endl;
    Sleep(1500);

    return true;
}

int main() {
    setEnvironmentVariable("PROC_TO_KILL", "CalculatorApp");
    
    std::cout << "------------------------------\n";
    startTestProcess("C:\\Windows\\System32\\calc.exe");

    std::cout << "CalculatorApp.exe exists: " << checkProcessExists("CalculatorApp.exe") << std::endl;
    runKillerProcess("");
    std::cout << "CalculatorApp.exe exists: " << checkProcessExists("CalculatorApp.exe") << std::endl;

    std::cout << "------------------------------\n";
        
    startTestProcess("C:\\Windows\\System32\\notepad.exe");
    std::cout << "notepad.exe exists: " << checkProcessExists("notepad.exe") << std::endl;
    runKillerProcess("--name notepad");
    std::cout << "notepad.exe exists: " << checkProcessExists("notepad.exe") << std::endl;

    std::cout << "------------------------------\n";

    DWORD processId = 0;
    startTestProcess("C:\\Windows\\System32\\notepad.exe");
    std::cout<< "notepad.exe exists: " << checkProcessExists("notepad.exe", &processId) << ", with pid = " << processId << std::endl;
    runKillerProcess("--id " + std::to_string(processId));
    std::cout << "notepad.exe exists: " << checkProcessExists("notepad.exe") << std::endl;

    std::cout << "------------------------------\n";

    deleteEnvironmentVariable("PROC_TO_KILL");
    return 0;
}