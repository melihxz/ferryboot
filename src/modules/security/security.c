#include "security.h"
#include "../../common/memory.h"
#include "../../common/string.h"
#include "../../common/crypto/sha256.h"
#include "../../common/crypto/pbkdf2.h"

// Global security module instance
static security_module_t security_module_instance;

// Initialize security module
static int security_init(void) {
    // Initialize module instance
    memory_set(&security_module_instance, 0, sizeof(security_module_instance));
    
    // Set default security settings
    security_module_instance.context.secure_boot_enabled = true;
    security_module_instance.context.signature_verification_enabled = true;
    security_module_instance.context.password_set = false;
    
    return 0;
}

// Cleanup security module
static void security_cleanup(void) {
    // Clear sensitive data
    memory_set(&security_module_instance.context.password_hash, 0, SHA256_DIGEST_SIZE);
    memory_set(&security_module_instance, 0, sizeof(security_module_instance));
}

// Set password
int security_set_password(const char* password) {
    if (!password) {
        return -1;
    }
    
    // Use a fixed salt for simplicity (in production, this should be random)
    uint8_t salt[32] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    
    // Hash password with PBKDF2
    if (pbkdf2_sha256(password, salt, sizeof(salt), 1000, 
                     security_module_instance.context.password_hash, 
                     SHA256_DIGEST_SIZE) != 0) {
        return -1;
    }
    
    security_module_instance.context.password_set = true;
    return 0;
}

// Verify password
int security_verify_password(const char* password) {
    if (!password || !security_module_instance.context.password_set) {
        return -1;
    }
    
    // Use the same salt as in set_password
    uint8_t salt[32] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
                        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    
    // Hash the provided password
    uint8_t password_hash[SHA256_DIGEST_SIZE];
    if (pbkdf2_sha256(password, salt, sizeof(salt), 1000, 
                     password_hash, SHA256_DIGEST_SIZE) != 0) {
        return -1;
    }
    
    // Compare hashes
    if (memory_compare(password_hash, security_module_instance.context.password_hash, 
                      SHA256_DIGEST_SIZE) == 0) {
        return 0; // Password correct
    }
    
    return -1; // Password incorrect
}

// Hash data
int security_hash_data(const void* data, size_t len, uint8_t hash[SHA256_DIGEST_SIZE]) {
    return sha256_hash(data, len, hash);
}

// Module metadata
static const module_header_t security_module_header = {
    .magic = MODULE_MAGIC,
    .version = 1,
    .type = MODULE_TYPE_SECURITY,
    .flags = 0,
    .name = "security",
    .description = "Security module with password protection and hashing"
};

// Module interface
static module_interface_t security_module_interface = {
    .init = security_init,
    .cleanup = security_cleanup
};

// Module entry point (renamed to avoid conflict)
int security_module_main(module_header_t* header) {
    if (header->magic != MODULE_MAGIC) {
        return -1;
    }
    
    *header = security_module_header;
    return (int)&security_module_interface;
}

// Public functions
int security_module_set_password(const char* password) {
    return security_set_password(password);
}

int security_module_verify_password(const char* password) {
    return security_verify_password(password);
}

int security_module_hash_data(const void* data, size_t len, uint8_t hash[SHA256_DIGEST_SIZE]) {
    return security_hash_data(data, len, hash);
}