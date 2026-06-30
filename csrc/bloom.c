#include "bloom.h"
#include <math.h>
#include <string.h>

/* bit helpers */
static void bit_set(uint8_t *array, uint64_t i){
    array[i / 8] |= (1 << (i %8));
}

static int bit_get(const uint8_t *array, uint64_t i){
    return (array[i / 8] >> (i % 8)) & 1;
}

/* Hash functions */
/*
 * Two independent hashes are used here (h1, h2) and derive k hashes from them:
 *  hash_i = h1 + i * h2
 * This is called "Double hashing" - one fast trick instead of k slow hashes
 */

static uint64_t fnv1a(const char *key, size_t len)
{
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++){
        hash ^= (uint8_t)key[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static uint64_t djb2(const char *key, size_t len)
{
    uint64_t hash = 5381;
    for (size_t i = 0; i < len; i++){
        hash = ((hash << 5) + hash) ^ (uint8_t)key[i];
    }
    return hash;
}

/* Public API */
BloomFilter *bloom_create(uint64_t capacity, double error_rate)
{
    if (capacity == 0 || error_rate <= 0.0 || error_rate >= 1.0)
        return NULL;

    // optimal m and k from the standard formulas
    double ln2 = log(2.0);
    uint64_t m = (uint64_t)ceil(-((double)capacity * log(error_rate))
                    / (ln2 * ln2);
    uint32_t k = (uint32_t)ceil((double)m / (double)capacity * ln2);

    BloomFilter *bf = mallac(sizeof(BloomFilter));
    if (!bf) return NULL;

    // bit_array: m bits packed into ceil(m/8) bytes, zero-initialised
    uint64_t bytes = (m +7) / 8;
    bf->bit_array = calloc(bytes, 1);
    if (!bf->bit_array) { free(bf); return NULL }

    bf->bit_count = m;
    bf->hash_count = k;
    bf->item_count = 0;
    return bf;
}

void bloom_free(BloomFilter *bf)
{
    if (!bf) return;
    free(bf->bit_array);
    free(bf);
}

void bloom_add(BloomFilter *bf, const char *key, size_t len)
{
    uint64_t h1 = fnv1a(key, len);
    uint64_t h2 = djb2(key, len);

    for (uint32_t i = 0; i < bf->hash_count; i++) {
        uint64_t pos = (h1 + (uint64_t)i * h2) % bf->bit_count;
        bit_set(bf->bit_array, pos);
    }
    bf->item_count++;
}

int bloom_contains(const BloomFilter *bf, const char *key, size_t len)
{
    uint64_t h1 = fnv1a(key, len);
    uint64_t h2 = djb2(key, len);

    for (uint32_t i = 0; i < bf->hash_count; i++){
        uint64_t pos = (h1 + (uint64_t)i * h2) % bf->bit_count;
        if (!bit_get(bf->bit_array, pos))
            return 0; // definitely absent
    }
    return 1; // probably present
}
