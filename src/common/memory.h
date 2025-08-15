#ifndef FERRYBOOT_MEMORY_H
#define FERRYBOOT_MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Memory management functions
void* memory_alloc(size_t size);
void memory_free(void* ptr);
void memory_set(void* ptr, uint8_t value, size_t len);
void memory_copy(void* dest, const void* src, size_t len);
int memory_compare(const void* ptr1, const void* ptr2, size_t len);

#endif // FERRYBOOT_MEMORY_H