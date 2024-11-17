#include "utils.hpp"
#include <iostream>

using std::cout;
using std::string;
void exit_with_msg(const string& t_msg) {
    perror(t_msg.c_str());
    //exit(EXIT_FAILURE);
    throw std::runtime_error(t_msg);
}

void log_msg(const string& t_msg) {
    cout << t_msg << "\n";
}

void log_err(const string& t_msg) {
    perror(t_msg.c_str());
}
