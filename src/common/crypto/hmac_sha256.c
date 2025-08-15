#include "hmac_sha256.h"
#include "sha256.h"
#include "../memory.h"

// HMAC-SHA256 implementation
int hmac_sha256(const uint8_t* key, size_t key_len,
                const uint8_t* data, size_t data_len,
                uint8_t digest[HMAC_SHA256_DIGEST_SIZE]) {
    sha256_context_t ctx;
    uint8_t k_ipad[64];
    uint8_t k_opad[64];
    uint8_t temp[SHA256_DIGEST_SIZE];
    size_t i;
    
    // Reset pads
    memory_set(k_ipad, 0, sizeof(k_ipad));
    memory_set(k_opad, 0, sizeof(k_opad));
    
    // If key is longer than 64 bytes, hash it
    if (key_len > 64) {
        sha256_hash(key, key_len, temp);
        key = temp;
        key_len = SHA256_DIGEST_SIZE;
    }
    
    // Copy key to pads
    memory_copy(k_ipad, key, key_len);
    memory_copy(k_opad, key, key_len);
    
    // XOR pads with 0x36 and 0x5C
    for (i = 0; i < 64; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5C;
    }
    
    // Inner hash
    sha256_init(&ctx);
    sha256_update(&ctx, k_ipad, 64);
    sha256_update(&ctx, data, data_len);
    sha256_final(&ctx, temp);
    
    // Outer hash
    sha256_init(&ctx);
    sha256_update(&ctx, k_opad, 64);
    sha256_update(&ctx, temp, SHA256_DIGEST_SIZE);
    sha256_final(&ctx, digest);
    
    return 0;
}