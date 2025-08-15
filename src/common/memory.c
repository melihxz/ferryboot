#include "memory.h"

// Simple memory allocation (placeholder implementation)
static uint8_t memory_pool[65536]; // 64KB memory pool
static size_t memory_pool_offset = 0;

void* memory_alloc(size_t size) {
    if (memory_pool_offset + size > sizeof(memory_pool)) {
        return NULL; // Not enough memory
    }
    
    void* ptr = &memory_pool[memory_pool_offset];
    memory_pool_offset += size;
    return ptr;
}

void memory_free(void* ptr) {
    // Simple implementation - no actual freeing
    // In a real implementation, we would track allocations
}

void memory_set(void* ptr, uint8_t value, size_t len) {
    uint8_t* p = (uint8_t*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = value;
    }
}

void memory_copy(void* dest, const void* src, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
}

int memory_compare(const void* ptr1, const void* ptr2, size_t len) {
    const uint8_t* p1 = (const uint8_t*)ptr1;
    const uint8_t* p2 = (const uint8_t*)ptr2;
    
    for (size_t i = 0; i < len; i++) {
        if (p1[i] < p2[i]) return -1;
        if (p1[i] > p2[i]) return 1;
    }
    
    return 0;
}