/*
 * queue.h
 *
 *  Created on: Dec 30, 2015
 *      Author: ashish
 */

#ifndef INCLUDES_WFQUEUE_H_
#define INCLUDES_WFQUEUE_H_

#include <stdint.h>

struct wf_queue_node;

typedef struct wf_queue_node {
	volatile struct wf_queue_node *next;
	volatile int enq_tid;
	volatile int deq_tid; // use CAS to update it
	volatile int sanityData;
} wf_queue_node_t;

typedef struct wf_queue_head {
	volatile wf_queue_node_t* head;
	volatile wf_queue_node_t* tail;
} wf_queue_head_t;

/*
 * One for each thread to sync operations among threads.
 */
typedef struct wf_queues_op_desc {
	volatile long phase;
	volatile uint8_t pending;
	volatile uint8_t enqueue;
	volatile wf_queue_node_t * node;
	volatile wf_queue_node_t * last; // this points to last node of the list that is to be enqueued during bulk enqueue
	wf_queue_head_t* queue;
} wf_queue_op_desc_t;

typedef struct wf_queues_op_head {
	int num_threads;
	volatile wf_queue_op_desc_t** ops;
	volatile wf_queue_op_desc_t** ops_reserve;
} wf_queue_op_head_t;

wf_queue_head_t* create_wf_queue(wf_queue_node_t* sentinel);

wf_queue_node_t* create_wf_queue_node();

void init_wf_queue_node(wf_queue_node_t* node);

wf_queue_op_head_t* create_queue_op_desc(int num_threads);

void wf_enqueue(wf_queue_head_t *q, wf_queue_node_t* node, wf_queue_op_head_t* op_desc, int thread_id);

wf_queue_node_t* wf_dequeue(wf_queue_head_t *q, wf_queue_op_head_t* op_desc, int thread_id);

int wf_queue_count_nodes(wf_queue_head_t* head);

#endif /* INCLUDES_WFQUEUE_H_ */
