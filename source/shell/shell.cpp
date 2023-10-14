#include "../../include/shell/shell.h"

std::string Shell::shell() {
    std::string shell_buffer;
    char ch = 0;

    for (uint32_t i = 0; ((ch = getch()) != '\n'); i++)
        shell_buffer.push_back(ch);

    return shell_buffer;
}