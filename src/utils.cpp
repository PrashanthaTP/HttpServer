#include "utils.hpp"

void exitWithMsg(const string& t_msg) {
    perror(t_msg.c_str());
    exit(EXIT_FAILURE);
}
