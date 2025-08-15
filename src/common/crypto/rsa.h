#ifndef FERRYBOOT_RSA_H
#define FERRYBOOT_RSA_H

#include <stdint.h>
#include <stddef.h>

// RSA public key
typedef struct {
    uint8_t* modulus;
    size_t modulus_size;
    uint32_t exponent;
} rsa_public_key_t;

// Function prototypes
int rsa_verify(const uint8_t* signature, size_t sig_len,
               const uint8_t* data, size_t data_len,
               const rsa_public_key_t* pubkey);

#endif // FERRYBOOT_RSA_H