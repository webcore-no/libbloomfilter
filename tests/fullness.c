#include "../src/bloomfilter.h"
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>

#define TEST_DURATION 10
#define ELEMENTS 10000000
#define TELEMENTS 1000000

int main(void)
{

	bloomfilter_t *filter = bloomfilter_new(bloomfilter_shm_alloc);

	if (!filter) {
		printf("ERROR: falied to make filter");
		exit(1);
	}

	// create all workers
	for (int i = 1; i <= ELEMENTS; i++) {
		bloomfilter_add(filter, &i, sizeof(i));
		if(i%100000 == 0) {
			int counter = 0;
			for (int i = ELEMENTS; i <= ELEMENTS+TELEMENTS; i++) {
				counter += bloomfilter_test(filter, &i, sizeof(i));
			}
			double error_rate = ((double)counter / (double)TELEMENTS )*(double)100;
			printf("%d, %f\n", i, error_rate);
		}
	}
	bloomfilter_destroy(&filter, bloomfilter_shm_free);
	return 0;
}
