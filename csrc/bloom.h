#ifndef BLOOM_H
#define BOOM_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t *bit_array; /* the actual bits, packed into bytes */
    uint64_t bit_count; /* total bits (m) */
    uint32_t hash_count; // number of hash functions (k)
    uint64_t item_count; // items inserted so far (n)
} BloomFilter;

/* Create a new filter. Return NULL on failure. */
BloomFilter *bloom_create(uint64_t capacity, double error_rate);

// Free all the memory
void bloom_free(BloomFilter *bf);

// Add a key (string + length). 
void bloom_add(BloomFilter *bf, const char *key, size_t len);

// Check a key. Returns 1 if probably present, 0 if definitely absent.
int bloom_contains(const BloomFilter *bf, const char *key, size_t len);

#endif // BLOOM_H
