#include "sha256.h"
#include "../memory.h"

// SHA-256 constants
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Right rotate macro
#define ROTR(n,x) (((x) >> (n)) | ((x) << (32 - (n))))

// SHA-256 functions
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(2,x) ^ ROTR(13,x) ^ ROTR(22,x))
#define EP1(x) (ROTR(6,x) ^ ROTR(11,x) ^ ROTR(25,x))
#define SIG0(x) (ROTR(7,x) ^ ROTR(18,x) ^ ((x) >> 3))
#define SIG1(x) (ROTR(17,x) ^ ROTR(19,x) ^ ((x) >> 10))

// Initialize SHA-256 context
void sha256_init(sha256_context_t* context) {
    context->state[0] = 0x6a09e667;
    context->state[1] = 0xbb67ae85;
    context->state[2] = 0x3c6ef372;
    context->state[3] = 0xa54ff53a;
    context->state[4] = 0x510e527f;
    context->state[5] = 0x9b05688c;
    context->state[6] = 0x1f83d9ab;
    context->state[7] = 0x5be0cd19;
    context->count = 0;
    memory_set(context->buffer, 0, sizeof(context->buffer));
}

// Process a single 512-bit block
static void sha256_process_block(sha256_context_t* context, const uint8_t block[64]) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    int i;

    // Break chunk into sixteen 32-bit big-endian words
    for (i = 0; i < 16; i++) {
        w[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) |
               (block[i * 4 + 2] << 8) | block[i * 4 + 3];
    }

    // Extend the sixteen 32-bit words into sixty-four 32-bit words
    for (i = 16; i < 64; i++) {
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    // Initialize hash value for this chunk
    a = context->state[0];
    b = context->state[1];
    c = context->state[2];
    d = context->state[3];
    e = context->state[4];
    f = context->state[5];
    g = context->state[6];
    h = context->state[7];

    // Main loop
    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + k[i] + w[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    // Add this chunk's hash to result so far
    context->state[0] += a;
    context->state[1] += b;
    context->state[2] += c;
    context->state[3] += d;
    context->state[4] += e;
    context->state[5] += f;
    context->state[6] += g;
    context->state[7] += h;
}

// Update SHA-256 context with data
void sha256_update(sha256_context_t* context, const void* data, size_t len) {
    const uint8_t* input = (const uint8_t*)data;
    size_t buffer_index = (size_t)(context->count & 63);
    size_t remaining;

    context->count += len;

    // Process complete 64-byte blocks
    if (buffer_index > 0) {
        remaining = 64 - buffer_index;
        if (len >= remaining) {
            memory_copy(context->buffer + buffer_index, input, remaining);
            sha256_process_block(context, context->buffer);
            input += remaining;
            len -= remaining;
            buffer_index = 0;
        }
    }

    // Process complete 64-byte blocks
    while (len >= 64) {
        sha256_process_block(context, input);
        input += 64;
        len -= 64;
    }

    // Buffer remaining data
    if (len > 0) {
        memory_copy(context->buffer + buffer_index, input, len);
    }
}

// Finalize SHA-256 hash
void sha256_final(sha256_context_t* context, uint8_t digest[SHA256_DIGEST_SIZE]) {
    uint8_t padding[64];
    uint64_t bit_count;
    int i;

    // Calculate padding
    memory_set(padding, 0, sizeof(padding));
    padding[0] = 0x80;

    // Calculate bit count
    bit_count = context->count * 8;

    // Pad to 448 bits mod 512
    size_t buffer_index = (size_t)(context->count & 63);
    if (buffer_index < 56) {
        sha256_update(context, padding, 56 - buffer_index);
    } else {
        sha256_update(context, padding, 64 - buffer_index + 56);
    }

    // Append bit count as 64-bit big-endian integer
    for (i = 0; i < 8; i++) {
        padding[56 + i] = (bit_count >> (56 - 8 * i)) & 0xff;
    }
    sha256_update(context, padding, 8);

    // Store digest
    for (i = 0; i < 8; i++) {
        digest[i * 4] = (context->state[i] >> 24) & 0xff;
        digest[i * 4 + 1] = (context->state[i] >> 16) & 0xff;
        digest[i * 4 + 2] = (context->state[i] >> 8) & 0xff;
        digest[i * 4 + 3] = context->state[i] & 0xff;
    }

    // Clear context
    memory_set(context, 0, sizeof(sha256_context_t));
}

// Convenience function to hash data in one call
int sha256_hash(const void* data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]) {
    sha256_context_t context;
    
    sha256_init(&context);
    sha256_update(&context, data, len);
    sha256_final(&context, digest);
    
    return 0;
}