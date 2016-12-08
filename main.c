#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include "includes/utils.h"
#include "includes/logger.h"
#include "includes/wfqueue.h"
#include "includes/list.h"
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

typedef struct dummy_data_wf_queue {
    int data;
    wf_queue_node_t node;
} dummy_data_wf_queue_t;

typedef struct test_data_wf_queue {
    int thread_id;
    int count_enque_ops;
    int count_deque_ops;
    int number_ops;
    wf_queue_head_t *q;
    wf_queue_op_head_t *op_desc;
    dummy_data_wf_queue_t* dummy_data;
    wf_queue_node_t** queue_data;
} test_data_wf_queue_t;

pthread_barrier_t barr_enq, barr_deq;
void* test_func_wf_queue(void* thread_data) {
    LOG_PROLOG();

    test_data_wf_queue_t* data = (test_data_wf_queue_t*)thread_data;

    pthread_barrier_wait(&barr_enq);
    int i = 0;
    for (i = 0; i < data->count_enque_ops; ++i) {
        wf_enqueue(data->q, &(data->dummy_data[i].node), data->op_desc, data->thread_id);
    }

    LOG_DEBUG("Finished Enqueue - %d", data->thread_id);

    pthread_barrier_wait(&barr_deq);
    LOG_DEBUG("Start Dequeue - %d", data->thread_id);

    wf_queue_node_t *x = NULL;
    dummy_data_wf_queue_t* val = NULL;
    int perThreadSanityFails = 0;
    for (i = 0; i < data->count_deque_ops; ++i) {
        x = wf_dequeue(data->q, data->op_desc, data->thread_id);
        val = (dummy_data_wf_queue_t*) list_entry(x, dummy_data_wf_queue_t, node);

        if (val->data != x->sanityData) {
            perThreadSanityFails++;
        }
    }

    LOG_DEBUG("Finished Dequeue - %d", data->thread_id);
    if (perThreadSanityFails > 0) {
        LOG_DEBUG("Sanity Fails in dequeue - %d", perThreadSanityFails);
    }

    LOG_EPILOG();
    return NULL;
}

void test_wf_queue() {
    LOG_PROLOG();

    const int COUNT_THREADS = 10;
    const int COUNT_ENQUEUE_OPS = 90000;
    const int COUNT_DEQUEUE_OPS = 10000;
    dummy_data_wf_queue_t *dummy_data = (dummy_data_wf_queue_t*) malloc(sizeof(dummy_data_wf_queue_t) * (COUNT_THREADS*COUNT_ENQUEUE_OPS + 1));

    int i = 0;
    for (i = 0; i < (COUNT_THREADS*COUNT_ENQUEUE_OPS + 1); ++i) {
        dummy_data[i].data = i;
        init_wf_queue_node(&(dummy_data[i].node));
        dummy_data[i].node.sanityData = i;
    }

    wf_queue_head_t *q = create_wf_queue(&(dummy_data[0].node));
    wf_queue_op_head_t *op_desc = create_queue_op_desc(COUNT_THREADS);
    pthread_t threads[COUNT_THREADS];
    pthread_barrier_init(&barr_enq, NULL, COUNT_THREADS);
    pthread_barrier_init(&barr_deq, NULL, COUNT_THREADS);

    test_data_wf_queue_t thread_data[COUNT_THREADS];

    for (i = 0; i < COUNT_THREADS; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].count_enque_ops = COUNT_ENQUEUE_OPS;
        thread_data[i].count_deque_ops = COUNT_DEQUEUE_OPS;
        thread_data[i].q = q;
        thread_data[i].op_desc = op_desc;
        thread_data[i].dummy_data = dummy_data + i*COUNT_ENQUEUE_OPS+1;

        LOG_INFO("Creating thread %d", i);
        pthread_create(threads + i, NULL, test_func_wf_queue, thread_data+i);
    }

    for (i = 0; i < COUNT_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    int *verify = (int*)malloc(sizeof(int) * (COUNT_THREADS*(COUNT_ENQUEUE_OPS) + 1));
    memset(verify, 0, sizeof(int) * (COUNT_THREADS*(COUNT_ENQUEUE_OPS) + 1));

    wf_queue_node_t *x = GET_PTR_FROM_TAGGEDPTR(q->head, wf_queue_node_t);
    int total=0;
    int sanityFails = 0;
    while(x!=NULL){
        dummy_data_wf_queue_t* val = (dummy_data_wf_queue_t*)list_entry(x, dummy_data_wf_queue_t, node);
        if (val->data > (COUNT_THREADS*COUNT_ENQUEUE_OPS)) {
            LOG_WARN("Invalid value in node - %d", val->data);
        }
        else {
            if (verify[val->data] == 1) {
                LOG_WARN("Duplicate = %d", val->data);
            } else {
                verify[val->data] = 1;
            }
        }

        if (val->data != val->node.sanityData) {
            sanityFails++;
        }
        total++;
        x=GET_PTR_FROM_TAGGEDPTR(x->next, wf_queue_node_t);
    }

    int count_miss = 0;
    int count_found = 0;
    for (i = 0; i < COUNT_THREADS*(COUNT_ENQUEUE_OPS)+1; ++i) {
        if (verify[i] == 1) {
            count_found++;
        } else {
            count_miss++;
        }
    }

    LOG_INFO("Number of missed items = %d", count_miss);
    LOG_INFO("Number of sanity fails = %d", sanityFails);
    LOG_INFO("Number of items enqueued = %d", count_found);
    LOG_INFO("Total number of items in queue = %d", total);

    LOG_EPILOG();
}

int main() {
    LOG_INIT_CONSOLE();

    test_wf_queue();

    LOG_CLOSE();
    return 0;
}
