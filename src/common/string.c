#include "../common/string.h"

size_t string_length(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}