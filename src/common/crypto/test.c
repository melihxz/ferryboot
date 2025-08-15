#include "crypto/sha256.h"
#include "crypto/hmac_sha256.h"
#include "crypto/pbkdf2.h"
#include "common/memory.h"
#include "common/string.h"

// Simple test function
void test_crypto_functions(void) {
    // Test SHA-256
    uint8_t digest[SHA256_DIGEST_SIZE];
    const char* test_data = "Hello, World!";
    
    if (sha256_hash(test_data, string_length(test_data), digest) == 0) {
        // Hash calculated successfully
    }
    
    // Test HMAC-SHA256
    const char* key = "secret_key";
    uint8_t hmac_digest[HMAC_SHA256_DIGEST_SIZE];
    
    if (hmac_sha256((const uint8_t*)key, string_length(key),
                    (const uint8_t*)test_data, string_length(test_data),
                    hmac_digest) == 0) {
        // HMAC calculated successfully
    }
    
    // Test PBKDF2
    uint8_t derived_key[32];
    uint8_t salt[32] = {0};
    
    if (pbkdf2_sha256("password", salt, sizeof(salt), 1000, 
                     derived_key, sizeof(derived_key)) == 0) {
        // PBKDF2 completed successfully
    }
}