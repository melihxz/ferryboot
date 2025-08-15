#include "string.h"

size_t string_length(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

int string_compare(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int string_compare_n(const char* str1, const char* str2, size_t n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    
    if (n == 0) return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* string_copy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

char* string_copy_n(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n && (*dest++ = *src++)) {
        n--;
    }
    
    while (n--) {
        *dest++ = '\0';
    }
    
    return ret;
}

char* string_duplicate(const char* src) {
    size_t len = string_length(src) + 1;
    char* dup = memory_alloc(len);
    if (dup) {
        string_copy(dup, src);
    }
    return dup;
}