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
#include <assert.h>
#include <time.h>
#include <limits.h>

long usecs();

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
    long start_enq;
    long end_enq;
    long start_deq;
    long end_deq;
} test_data_wf_queue_t;

pthread_barrier_t barr_enq, barr_deq;
void* test_func_wf_queue(void* thread_data) {
    LOG_PROLOG();

    test_data_wf_queue_t* data = (test_data_wf_queue_t*)thread_data;


    pthread_barrier_wait(&barr_enq);
    data->start_enq = usecs();
    int i = 0;
    for (i = 0; i < data->count_enque_ops; ++i) {
        wf_enqueue(data->q, &(data->dummy_data[i].node), data->op_desc, data->thread_id);
    }

    data->end_enq = usecs();
    LOG_DEBUG("Finished Enqueue - %d", data->thread_id);

    pthread_barrier_wait(&barr_deq);
    LOG_DEBUG("Start Dequeue - %d", data->thread_id);

    //wf_queue_node_t *x = NULL;
    //dummy_data_wf_queue_t* val = NULL;
    int perThreadSanityFails = 0;
    
    data->start_deq = usecs();
    for (i = 0; i < data->count_deque_ops; ++i) {
        wf_dequeue(data->q, data->op_desc, data->thread_id);
        /*x = wf_dequeue(data->q, data->op_desc, data->thread_id);
        val = (dummy_data_wf_queue_t*) list_entry(x, dummy_data_wf_queue_t, node);

        if (x == NULL || val->data != x->sanityData) {
            perThreadSanityFails++;
        }*/
    }

    data->end_deq = usecs();

    LOG_DEBUG("Finished Dequeue - %d", data->thread_id);
    if (perThreadSanityFails > 0) {
        LOG_DEBUG("Sanity Fails in dequeue - %d", perThreadSanityFails);
    }

    LOG_EPILOG();
    return NULL;
}

void test_wf_queue(int count_threads, int enq_ops, int deq_ops) {
    LOG_PROLOG();

    dummy_data_wf_queue_t *dummy_data = (dummy_data_wf_queue_t*) malloc(sizeof(dummy_data_wf_queue_t) * (count_threads*enq_ops + 1));

    int i = 0;
    for (i = 0; i < (count_threads*enq_ops + 1); ++i) {
        dummy_data[i].data = i;
        init_wf_queue_node(&(dummy_data[i].node));
        dummy_data[i].node.sanityData = i;
    }

    wf_queue_head_t *q = create_wf_queue(&(dummy_data[0].node));
    wf_queue_op_head_t *op_desc = create_queue_op_desc(count_threads);
    pthread_t *threads = (pthread_t*)malloc(sizeof(pthread_t) * count_threads);;
    pthread_barrier_init(&barr_enq, NULL, count_threads);
    pthread_barrier_init(&barr_deq, NULL, count_threads);

    test_data_wf_queue_t *thread_data = (test_data_wf_queue_t*)malloc(sizeof(test_data_wf_queue_t) * count_threads);

    for (i = 0; i < count_threads; ++i) {
        thread_data[i].thread_id = i;
        thread_data[i].count_enque_ops = enq_ops;
        thread_data[i].count_deque_ops = deq_ops;
        thread_data[i].q = q;
        thread_data[i].op_desc = op_desc;
        thread_data[i].dummy_data = dummy_data + i*enq_ops+1;
        thread_data[i].start_enq = 0;
        thread_data[i].end_enq = 0;
        thread_data[i].start_deq = 0;
        thread_data[i].end_deq = 0;

        pthread_create(threads + i, NULL, test_func_wf_queue, thread_data+i);
    }

    for (i = 0; i < count_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    long start_enq = LONG_MAX;
    long end_enq = 0;
    long start_deq = LONG_MAX;
    long end_deq = 0;
    for (i = 0; i < count_threads; ++i) {
       start_enq = MIN(start_enq, thread_data[i].start_enq); 
       end_enq = MAX(end_enq, thread_data[i].end_enq); 
       start_deq = MIN(start_deq, thread_data[i].start_deq); 
       end_deq = MAX(end_deq, thread_data[i].end_deq);
       printf("%d, %lu, %lu\n", i, (thread_data[i].end_enq - thread_data[i].start_enq), (thread_data[i].end_deq - thread_data[i].start_deq));
    }
    
    printf("%d threads, %d enq_ops, %d deq_ops - Enq = %lu, Deq = %lu", count_threads, enq_ops, deq_ops, (end_enq-start_enq), (end_deq-start_deq));

    int *verify = (int*)malloc(sizeof(int) * (count_threads*enq_ops + 1));
    memset(verify, 0, sizeof(int) * (count_threads*enq_ops + 1));

    wf_queue_node_t *x = GET_PTR_FROM_TAGGEDPTR(q->head, wf_queue_node_t);
    int total=0;
    int sanityFails = 0;
    while(x!=NULL){
        dummy_data_wf_queue_t* val = (dummy_data_wf_queue_t*)list_entry(x, dummy_data_wf_queue_t, node);
        if (val->data > (count_threads*enq_ops)) {
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
    for (i = 0; i < count_threads*enq_ops+1; ++i) {
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

int main(int argc, char** argv) {
    LOG_INIT_CONSOLE();

    int count_threads = atoi(argv[1]);
    int enq_ops = atoi(argv[2]);
    int deq_ops = atoi(argv[3]);

    test_wf_queue(count_threads, enq_ops, deq_ops);

    LOG_CLOSE();
    return 0;
}

long usecs() {
    struct timespec t = {0,0};
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (long)(t.tv_sec*1000000 + 1.0e-3*t.tv_nsec);
}
