#include "../../include/fix/GWRunner.h"

GWRunner::GWRunner(const SimpleConfig& config) : config_(config) {
}

void GWRunner::run() {
    while (true) {
        std::string value;
        std::cin >> value;

        if (value == "q") {
            break;
        }

        std::cout << std::endl;
    }
}