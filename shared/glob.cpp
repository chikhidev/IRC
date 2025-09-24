#include "libs.hpp"

std::string glob::to_string(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}

bool *glob::server_running() {
    static bool is_running;
    return &is_running;
}

void glob::stop_running(int signal) {
    (void)signal;
    std::cout << "Triggered stop running server!" << std::endl;
    *glob::server_running() = false;
}
