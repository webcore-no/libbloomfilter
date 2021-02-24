#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include "../src/bloomfilter.h"
#define XXH_STATIC_LINKING_ONLY /* access advanced declarations */
#define XXH_IMPLEMENTATION /* access definitions */
#define XXH_INLINE_ALL
#include "xxhash.h"

// 2^27
#define BLOOMFILTER_SIZE 134217728
#define BLOOMFILTER_SIZE_BYTE BLOOMFILTER_SIZE / sizeof(volatile char)

#define hash(key, keylen) XXH3_64bits(key, keylen)

struct bloomfilter_s {
	volatile char data[BLOOMFILTER_SIZE_BYTE];
};

static inline int get(bloomfilter_t *filter, size_t key)
{
	uint64_t index = key / sizeof(char);
	uint8_t bit = 1u << (key % sizeof(char));
	return (filter->data[index] & bit) != 0;
}

static inline int set(bloomfilter_t *filter, size_t key)
{
	uint64_t index = key / sizeof(char);
	uint64_t bit = 1u << (key % sizeof(char));
	return (atomic_fetch_or(&filter->data[index], bit) & bit) == 0;
}

static inline int reset(bloomfilter_t *filter, size_t key)
{
	uint64_t index = key / sizeof(char);
	uint64_t bit = 1u << (key % sizeof(char));
	return (atomic_fetch_and(&filter->data[index], bit) & bit) != 0;
}

bloomfilter_t *bloomfilter_new(bloomfilter_allocator allocator)
{
	bloomfilter_t *filter = allocator(sizeof(bloomfilter_t));
	memset(filter, 0, sizeof(bloomfilter_t));
	return filter;
}
void bloomfilter_destroy(bloomfilter_t **filter,
			 bloomfilter_deallocator deallocators)
{
	deallocators(*filter, sizeof(filter));
	*filter = NULL;
}

void bloomfilter_clear(bloomfilter_t *filter)
{
	memset(filter, 0, sizeof(bloomfilter_t));
}

void bloomfilter_add(bloomfilter_t *filter, const void *key, size_t keylen)
{
	uint64_t hbase = hash(key, keylen);
	uint32_t h1 = (hbase >> 32) % BLOOMFILTER_SIZE;
	uint32_t h2 = hbase % BLOOMFILTER_SIZE;
	set(filter, h1);
	set(filter, h2);
}

int bloomfilter_test(bloomfilter_t *filter, const void *key, size_t keylen)
{
	uint64_t hbase = hash(key, keylen);
	uint32_t h1 = (hbase >> 32) % BLOOMFILTER_SIZE;
	uint32_t h2 = hbase % BLOOMFILTER_SIZE;
	return (get(filter, h1) && get(filter, h2));
}

// ------ Allocators --------
// Normal
void *bloomfilter_alloc(size_t size)
{
	return malloc(size);
}

void bloomfilter_free(void *ptr, size_t size)
{
	(void)size;
	free(ptr);
}
// SHM
void *bloomfilter_shm_alloc(size_t size)
{
	void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
			 MAP_ANON | MAP_SHARED, -1, 0);

	if (ptr == MAP_FAILED) {
		return NULL;
	}

	return ptr;
}
void bloomfilter_shm_free(void *ptr, size_t size)
{
	munmap(ptr, size);
}

// SWAP
struct bloomfilter_swap_s {
	bloomfilter_t *active;
	bloomfilter_t front;
	bloomfilter_t back;
};

bloomfilter_swap_t *bloomfilterswap_new(bloomfilter_allocator allocator)
{
	bloomfilter_swap_t *filter = allocator(sizeof(bloomfilter_swap_t));
	memset(&filter->front, 0xff, sizeof(bloomfilter_t));
	memset(&filter->back, 0x00, sizeof(bloomfilter_t));
	filter->active = &filter->front;
	return filter;
}
void bloomfilterswap_destroy(bloomfilter_swap_t **swap,
			     bloomfilter_deallocator deallocator)
{
	deallocator(*swap, sizeof(bloomfilter_swap_t));
	*swap = NULL;
}

void bloomfilterswap_swap(bloomfilter_swap_t *filter)
{
	if (filter->active == &filter->front) {
		filter->active = &filter->back;
		bloomfilter_clear(&filter->front);
	} else {
		filter->active = &filter->front;
		bloomfilter_clear(&filter->back);
	}
}

void bloomfilterswap_add(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen)
{
	uint64_t hbase = hash(key, keylen);
	uint32_t h1 = (hbase >> 32) % BLOOMFILTER_SIZE;
	uint32_t h2 = hbase % BLOOMFILTER_SIZE;

	set(&filter->front, h1);
	set(&filter->front, h2);

	set(&filter->back, h1);
	set(&filter->back, h2);
}
int bloomfilterswap_test(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen)
{
	uint64_t hbase = hash(key, keylen);
	uint32_t h1 = (hbase >> 32) % BLOOMFILTER_SIZE;
	uint32_t h2 = hbase % BLOOMFILTER_SIZE;
	return (get(filter->active, h1) && get(filter->active, h2));
}

