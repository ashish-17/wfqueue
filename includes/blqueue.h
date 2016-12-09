
#ifndef INCLUDES_BLQUEUE_H_
#define INCLUDES_BLQUEUE_H_

#include <stdint.h>
#include <pthread.h>

struct bl_queue_node;

typedef struct bl_queue_node {
    int sanityData;
    struct bl_queue_node *next;    
} bl_queue_node_t;

typedef struct bl_queue_head {
    bl_queue_node_t *head;
    bl_queue_node_t *tail;
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;
    bl_queue_node_t sentinel;
} bl_queue_head_t;

bl_queue_head_t* create_bl_queue();

void init_bl_queue_node(bl_queue_node_t *node);

bl_queue_node_t* create_bl_queue_node();

void bl_enqueue(bl_queue_head_t* q, bl_queue_node_t* node);

bl_queue_node_t* bl_dequeue(bl_queue_head_t* q);

#endif
