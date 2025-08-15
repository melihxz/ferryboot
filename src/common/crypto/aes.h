#ifndef FERRYBOOT_AES_H
#define FERRYBOOT_AES_H

#include <stdint.h>
#include <stddef.h>

// AES key sizes
#define AES_KEY_SIZE_128 16
#define AES_KEY_SIZE_192 24
#define AES_KEY_SIZE_256 32

// AES block size
#define AES_BLOCK_SIZE 16

// AES context
typedef struct {
    uint32_t round_key[60];
    int rounds;
} aes_context_t;

// Function prototypes
int aes_set_key(aes_context_t* context, const uint8_t* key, size_t key_len);
int aes_decrypt(const aes_context_t* context, const uint8_t* ciphertext,
                uint8_t* plaintext, size_t blocks);

#endif // FERRYBOOT_AES_H