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
#define CONTAINS_P 85
#define WORKER_COUNT 8

typedef struct {
	int parent_fd;
	bloomfilter_t *filter;
	uint32_t op_counter;
} _globals;
_globals globals;

void worker_loop()
{
	const u_int8_t key[] = "www.foobar.co.uk";
	u_int64_t key_len= strlen((const char *)key);
	int *k = (int *)key;
	srand(getpid());
	*k = rand();
	while(1) {
		int n = (*k)%100;
		if(n < CONTAINS_P) {
			bloomfilter_test(globals.filter, key, key_len);
		} else {
			bloomfilter_add(globals.filter, key, key_len);
		}
		(*k)++;
		globals.op_counter++;
	}
}

void handle_sighup(int __attribute__((unused)) signal) {
	// Write total ammount off operations and exit
	ssize_t wr = write(globals.parent_fd, &globals.op_counter, sizeof(uint32_t));
	if(wr == -1) {
		printf("%d:%s\n", errno, strerror(errno));
	}
	fflush(stdout);
	close(globals.parent_fd);
	exit(0);
}


typedef struct {
	int fd;
	pid_t pid;
} worker;

int create_worker(worker *wrk)
{
	int fd[2];
	int ret;
	pid_t pid;

	ret = pipe(fd);
	if(ret == -1) {
		return -1;
	}
	pid = fork();
	if(!pid) {
		// Worker
		close(fd[0]);
		globals.parent_fd = fd[1];
		worker_loop();
		exit(0);
	}
	if(pid < 0) {
		printf("ERROR[%d]:%s", errno, strerror(errno));
		close(fd[0]);
		close(fd[1]);
		return -1;
	}
	close(fd[1]);
	wrk->pid = pid;
	wrk->fd = fd[0];

	return 0;
}

int main(void)
{
	if(signal(SIGHUP, &handle_sighup) == SIG_ERR) {
		printf("ERROR[%d]:%s\n", errno, strerror(errno));
		exit(1);
	}
	worker workers[WORKER_COUNT];
	globals.filter = bloomfilter_new(bloomfilter_shm_alloc);
	if(!globals.filter) {
		printf("ERROR: falied to make filter");
		exit(1);
	}
	// create all workers
	for(int i = 0; i < WORKER_COUNT; i++) {
		if(create_worker(&workers[i])) {
			bloomfilter_destroy(&globals.filter, bloomfilter_shm_free);
			printf("ERROR[%d]:%s", errno, strerror(errno));
			exit(1);
		}
	}
	// sleep some time
	sleep(TEST_DURATION);
	// Kill all workers
	for(int i = 0; i < WORKER_COUNT; i++) {
		uint32_t worker_out = 0;
		if(kill(workers[i].pid, SIGHUP)) {
			bloomfilter_destroy(&globals.filter, bloomfilter_shm_free);
			printf("ERROR[%d]:%s", errno, strerror(errno));
			exit(1);
		}
		read(workers[i].fd, &worker_out, sizeof(uint32_t));
		globals.op_counter += worker_out;
	}
	bloomfilter_destroy(&globals.filter, bloomfilter_shm_free);
	printf("%d ops/s\n", globals.op_counter/ TEST_DURATION);
	return 0;
}
