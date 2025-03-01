#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_STEPS 200000000

int thread_count = 0;

typedef struct {
	double part_sum;
	int indx;
} thread_data;

void* thread_body(void* param) {
	thread_data* current_thread = (thread_data*) param;
	double sum = 0;

	for (int i = current_thread->indx; i < NUM_STEPS; i += thread_count) {
		sum += 1.0 / (i * 4.0 + 1.0);
		sum -= 1.0 / (i * 4.0 + 3.0);
	}

	current_thread->part_sum = sum;

	return &current_thread->part_sum;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		thread_count = atoi(argv[1]);
	}

	if (thread_count < 1) {
		printf("Error: thread_count is less than 1 or atoi() function returned error.\n");
		exit(-1);
	}

	thread_data* part_sums = (thread_data*)malloc(thread_count * sizeof(thread_data));
	pthread_t* thr_id = (pthread_t*)malloc(thread_count * sizeof(pthread_t));
	
	int pc = 0;

	for (int i = 0; i < thread_count; i++) {
		part_sums[i].indx = i;
		pc = pthread_create(&thr_id[i], NULL, thread_body, &part_sums[i]);

		if (pc != 0) {
			printf("Error in pthread_create().\n");
			exit(-1);
		}
	}

	double* part_sum = 0, ans = 0;

	for (int i = 0; i < thread_count; i++) {
		pthread_join(thr_id[i], (void**)&part_sum);

		ans += *part_sum;
	}

	ans *= 4.0;
	printf("%.15g\n", ans);

	free(part_sums);
	free(thr_id);

	exit(0);
}
