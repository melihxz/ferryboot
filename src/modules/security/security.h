#ifndef FERRYBOOT_SECURITY_MODULE_H
#define FERRYBOOT_SECURITY_MODULE_H

#include "../../common/module.h"
#include "../../common/crypto/sha256.h"

// Security module context
typedef struct {
    bool secure_boot_enabled;
    bool signature_verification_enabled;
    uint8_t password_hash[SHA256_DIGEST_SIZE];
    bool password_set;
} security_context_t;

// Security module interface
typedef struct {
    module_interface_t base;
    security_context_t context;
} security_module_t;

#endif // FERRYBOOT_SECURITY_MODULE_H