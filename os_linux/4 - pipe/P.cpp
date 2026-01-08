#include <iostream>
#include <string>
#include <sstream>

int main() {
    std::string line;
    std::getline(std::cin, line);
    std::stringstream ss(line);

    double x;
    while (ss >> x) {
        std::cout << x * x * x << " ";
    }
    std::cout << std::endl;
    return 0;
}
