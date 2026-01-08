#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <vector>
#include <sstream>

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool setEnvironmentVariable(const std::string& name, const std::string& value) {
    if (setenv(name.c_str(), value.c_str(), 1) == 0) {
        std::cout << "Environment variable '" << name << "' set to: " << value << std::endl;
        return true;
    }
    std::cerr << "Failed to set environment variable: " << strerror(errno) << std::endl;
    return false;
}

bool deleteEnvironmentVariable(const std::string& name) {
    if (unsetenv(name.c_str()) == 0) {
        std::cout << "Environment variable '" << name << "' deleted successfully" << std::endl;
        return true;
    }
    std::cerr << "Failed to delete environment variable: " << strerror(errno) << std::endl;
    return false;
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

bool checkProcessExists(const std::string& processName, pid_t* processId = nullptr) {
    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        std::cerr << "Failed to open /proc directory: " << strerror(errno) << std::endl;
        return false;
    }

    std::string targetName = toLower(processName);
    bool found = false;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        std::string pidStr = entry->d_name;

        bool isNumber = !pidStr.empty() && std::all_of(pidStr.begin(), pidStr.end(), ::isdigit);

        if (isNumber) {
            try {
                pid_t pid = std::stol(pidStr);
                std::string currentProcessName = toLower(getProcessName(pid));

                if (currentProcessName == targetName) {
                    found = true;
                    if (processId != nullptr) {
                        *processId = pid;
                    }
                    break;
                }
            } catch (const std::exception& e) {
                continue;
            }
        }
    }

    closedir(dir);
    return found;
}

void cleanupZombies() {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    }
}

bool runKillerProcess(const std::string& arguments) {
    std::string commandLine = "./killer " + arguments;
    std::cout << "\nRunning: " << commandLine << std::endl;

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork process: " << strerror(errno) << std::endl;
        return false;
    }

    if (pid == 0) {
        if (arguments.empty()) {
            execlp("./killer", "killer", (char*)nullptr);
        } else {
            std::vector<std::string> args;
            std::stringstream ss(arguments);
            std::string arg;

            while (ss >> arg) {
                args.push_back(arg);
            }

            std::vector<char*> argv;
            argv.push_back(const_cast<char*>("./killer"));
            for (auto& a : args) {
                argv.push_back(const_cast<char*>(a.c_str()));
            }
            argv.push_back(nullptr);

            execvp("./killer", argv.data());
        }

        std::cerr << "Failed to exec killer process: " << strerror(errno) << std::endl;
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    usleep(100000);
    cleanupZombies();
    usleep(400000);

    std::cout << std::endl;

    return true;
}



bool startTestProcess(const std::string& command, const std::vector<std::string>& args, pid_t* outProcessId = nullptr) {
    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Failed to fork process: " << strerror(errno) << std::endl;
        return false;
    }

    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(command.c_str()));

        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        execvp(command.c_str(), argv.data());

        std::cerr << "Failed to start process: " << command << " - " << strerror(errno) << std::endl;
        exit(1);
    }

    if (outProcessId != nullptr) {
        *outProcessId = pid;
    }

    std::cout << "Started process: " << command;
    for (const auto& arg : args) {
        std::cout << " " << arg;
    }
    if (outProcessId != nullptr) {
        std::cout << " (PID: " << *outProcessId << ")";
    }
    std::cout << std::endl;

    usleep(1500000);

    return true;
}

int main() {
    setEnvironmentVariable("PROC_TO_KILL", "sleep");

    std::cout << "------------------------------\n";
    startTestProcess("sleep", {"100"});
    std::cout << "sleep exists: " << checkProcessExists("sleep") << std::endl;
    runKillerProcess("");
    std::cout << "sleep exists: " << checkProcessExists("sleep") << std::endl;
    deleteEnvironmentVariable("PROC_TO_KILL");

    std::cout << "------------------------------\n";
    startTestProcess("sleep", {"100"});
    std::cout << "sleep exists: " << checkProcessExists("sleep") << std::endl;
    runKillerProcess("--name sleep");
    std::cout << "sleep exists: " << checkProcessExists("sleep") << std::endl;

    std::cout << "------------------------------\n";
    pid_t processId = 0;
    startTestProcess("sleep", {"100"});
    std::cout << "sleep exists: " << checkProcessExists("sleep", &processId) << ", with pid = " << processId;
    runKillerProcess("--id " + std::to_string(processId));
    std::cout << "sleep exists: " << checkProcessExists("sleep") << std::endl;

    return 0;
}
