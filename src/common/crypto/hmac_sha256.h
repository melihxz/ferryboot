#ifndef FERRYBOOT_HMAC_SHA256_H
#define FERRYBOOT_HMAC_SHA256_H

#include <stdint.h>
#include <stddef.h>

// HMAC-SHA256 digest size
#define HMAC_SHA256_DIGEST_SIZE 32

// Function prototypes
int hmac_sha256(const uint8_t* key, size_t key_len,
                const uint8_t* data, size_t data_len,
                uint8_t digest[HMAC_SHA256_DIGEST_SIZE]);

#endif // FERRYBOOT_HMAC_SHA256_H