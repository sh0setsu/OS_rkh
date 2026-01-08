#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::string line;
    std::getline(std::cin, line);
    std::stringstream ss(line);

    double x;
    double sum = 0;
    while (ss >> x) {
        sum += x;
    }
    std::cout << sum << std::endl;
    return 0;
}
