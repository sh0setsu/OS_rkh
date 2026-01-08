#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <signal.h>
#include <cstring>
#include <unistd.h>

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

std::string getProcessName(pid_t pid) {
    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(commPath);
    if (!commFile.is_open()) {
        return "";
    }

    std::string processName;
    std::getline(commFile, processName);
    commFile.close();

    if (!processName.empty() && processName.back() == '\n') {
        processName.pop_back();
    }

    return processName;
}

bool killProcessById(pid_t processId) {
    if (kill(processId, 0) != 0) {
        std::cerr << "Process with id " << processId << " doesn't exist" << std::endl;
        return false;
    }

    if (kill(processId, SIGKILL) == 0) {
        std::cout << "Process with id " << processId << " killed" << std::endl;
        return true;
    } else {
        std::cerr << "Couldn't terminate process with id " << processId << ": " << strerror(errno) << std::endl;
        return false;
    }
}

int killProcessesByName(const std::string& processName) {
    int killedCount = 0;

    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        std::cerr << "Failed to open /proc directory: " << strerror(errno) << std::endl;
        return 0;
    }

    std::string targetName = toLower(processName);
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        std::string pidStr = entry->d_name;

        bool isNumber = !pidStr.empty() && std::all_of(pidStr.begin(), pidStr.end(), ::isdigit);

        if (isNumber) {
            try {
                pid_t pid = std::stol(pidStr);
                std::string currentProcessName = toLower(getProcessName(pid));

                if (currentProcessName == targetName) {
                    if (killProcessById(pid)) {
                        killedCount++;
                    }
                }
            } catch (const std::exception& e) {
                continue;
            }
        }
    }

    closedir(dir);

    if (killedCount == 0) {
        std::cout << "Process with name " << processName << " isn't found" << std::endl;
    }

    return killedCount;
}

std::string getEnvironmentVariable(const std::string& name) {
    const char* value = getenv(name.c_str());
    if (value == nullptr) {
        return "";
    }
    return std::string(value);
}

void killProcessesFromEnvironment() {
    std::string procToKill = getEnvironmentVariable("PROC_TO_KILL");
    std::cout << "PROC_TO_KILL: ";

    if (procToKill.empty()) {
        std::cout << "empty\n";
        return;
    }

    std::cout << procToKill << std::endl;

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
                    pid_t processId = std::stol(argv[i + 1]);
                    std::cout << "Killing process with id " << processId << " -> ";
                    killProcessById(processId);
                    i++;
                } catch (const std::exception& e) {
                    std::cerr << "Invalid id: " << argv[i + 1] << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: no id after --id" << std::endl;
                return 1;
            }
        } else if (arg == "--name") {
            if (i + 1 < argc) {
                std::string processName = argv[i + 1];
                std::cout << "Killing process with name " << processName << " -> ";
                killProcessesByName(processName);
                i++;
            } else {
                std::cerr << "Error: no name after --name" << std::endl;
                return 1;
            }
        }
    }

    return 0;
}
