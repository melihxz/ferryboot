#ifndef FERRYBOOT_PBKDF2_H
#define FERRYBOOT_PBKDF2_H

#include <stdint.h>
#include <stddef.h>

// Function prototypes
int pbkdf2_sha256(const char* passphrase, const uint8_t* salt, size_t salt_len,
                  uint32_t iterations, uint8_t* derived_key, size_t key_len);

#endif // FERRYBOOT_PBKDF2_H