#ifndef FERRYBOOT_SHA256_H
#define FERRYBOOT_SHA256_H

#include <stdint.h>
#include <stddef.h>

// SHA-256 context
typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} sha256_context_t;

// SHA-256 digest size
#define SHA256_DIGEST_SIZE 32

// Function prototypes
void sha256_init(sha256_context_t* context);
void sha256_update(sha256_context_t* context, const void* data, size_t len);
void sha256_final(sha256_context_t* context, uint8_t digest[SHA256_DIGEST_SIZE]);
int sha256_hash(const void* data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]);

#endif // FERRYBOOT_SHA256_H