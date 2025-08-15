#ifndef FERRYBOOT_STRING_H
#define FERRYBOOT_STRING_H

#include <stddef.h>

// String utility functions
size_t string_length(const char* str);
int string_compare(const char* str1, const char* str2);
int string_compare_n(const char* str1, const char* str2, size_t n);
char* string_copy(char* dest, const char* src);
char* string_copy_n(char* dest, const char* src, size_t n);
char* string_duplicate(const char* src);

#endif // FERRYBOOT_STRING_H