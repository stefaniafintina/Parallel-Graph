// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_sum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_vis = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_process = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_max = PTHREAD_MUTEX_INITIALIZER;
int new_task = 1;
int maxim = -1;
int processed[1000] = {0};
os_graph_t *new_graph;
/* TODO: Define graph synchronization mechanisms. */
/* TODO: Define graph task argument. */

static void process_node(void *arg)
{
	/* TODO: Implement thread-pool based processing of graph. */
	//adaug la sum nodul idx
	//ii verific vecinii lui idx si ii adaug daca nu sunt vizitati;
	//creez un nou task (contine process node ~ action , idx(args))
	// printf("vreau sa verifiic\n");
	//printf("%d\n", *(int *)arg);
	int idx = *(int *)arg;

	pthread_mutex_lock(&mutex_sum);
	sum += graph->nodes[idx]->info;
	pthread_mutex_unlock(&mutex_sum);

	pthread_mutex_lock(&mutex_vis);
	graph->visited[idx] = 1;
	pthread_mutex_unlock(&mutex_vis);

	for (int i = 0; i < (int)graph->nodes[idx]->num_neighbours; i++) {
		pthread_mutex_lock(&mutex_vis);
		if (graph->visited[graph->nodes[idx]->neighbours[i]] == 0) {
			graph->visited[graph->nodes[idx]->neighbours[i]] = 1;
			pthread_mutex_unlock(&mutex_vis);
			os_task_t *t = create_task(process_node, &(graph->nodes[graph->nodes[idx]->neighbours[i]]->id), free);

			enqueue_task(tp, t);
		} else {
			pthread_mutex_unlock(&mutex_vis);
		}
	}

	pthread_mutex_lock(&mutex_vis);
	graph->visited[idx] = 2;
	pthread_mutex_unlock(&mutex_vis);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);
	new_graph = graph;
	/* TODO: Initialize graph synchronization mechanisms. */
	//adaug nodul 0 in coada;

	tp = create_threadpool(NUM_THREADS);
	enqueue_task(tp, create_task(process_node, &graph->nodes[0]->id, free));
	wait_for_completion(tp);
	destroy_threadpool(tp);
	printf("%d", sum);

	return 0;
}
