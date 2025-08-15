#include "pbkdf2.h"
#include "hmac_sha256.h"
#include "../memory.h"
#include "../string.h"

// PBKDF2 with HMAC-SHA256
int pbkdf2_sha256(const char* passphrase, const uint8_t* salt, size_t salt_len,
                  uint32_t iterations, uint8_t* derived_key, size_t key_len) {
    uint8_t u[HMAC_SHA256_DIGEST_SIZE];
    uint8_t t[HMAC_SHA256_DIGEST_SIZE];
    uint8_t* buffer;
    size_t buffer_len;
    uint32_t i, j, k;
    uint32_t counter;
    
    // Allocate buffer for salt + counter
    buffer_len = salt_len + 4;
    buffer = (uint8_t*)memory_alloc(buffer_len);
    if (!buffer) {
        return -1;
    }
    
    // Copy salt to buffer
    memory_copy(buffer, salt, salt_len);
    
    // Generate derived key
    counter = 1;
    while (key_len > 0) {
        // Set counter in buffer
        buffer[salt_len] = (counter >> 24) & 0xff;
        buffer[salt_len + 1] = (counter >> 16) & 0xff;
        buffer[salt_len + 2] = (counter >> 8) & 0xff;
        buffer[salt_len + 3] = counter & 0xff;
        
        // U1 = HMAC-SHA256(password, salt || counter)
        hmac_sha256((const uint8_t*)passphrase, string_length(passphrase),
                    buffer, buffer_len, u);
        
        // T = U1
        memory_copy(t, u, HMAC_SHA256_DIGEST_SIZE);
        
        // T = U1 ^ U2 ^ ... ^ Uc
        for (i = 1; i < iterations; i++) {
            // U_i = HMAC-SHA256(password, U_{i-1})
            hmac_sha256((const uint8_t*)passphrase, string_length(passphrase),
                        u, HMAC_SHA256_DIGEST_SIZE, u);
            
            // T = T ^ U_i
            for (j = 0; j < HMAC_SHA256_DIGEST_SIZE; j++) {
                t[j] ^= u[j];
            }
        }
        
        // Copy T to derived key
        k = (key_len < HMAC_SHA256_DIGEST_SIZE) ? key_len : HMAC_SHA256_DIGEST_SIZE;
        memory_copy(derived_key, t, k);
        derived_key += k;
        key_len -= k;
        counter++;
    }
    
    memory_free(buffer);
    return 0;
}