#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "includes/blqueue.h"
#include "includes/logger.h" 
#include "includes/utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>

bl_queue_head_t* create_bl_queue() {
    bl_queue_head_t* q = (bl_queue_head_t*)malloc(sizeof(bl_queue_head_t));

    pthread_mutex_init(&(q->head_lock), NULL);
    pthread_mutex_init(&(q->tail_lock), NULL);
    
    init_bl_queue_node(&q->sentinel);
    
    q->head = &q->sentinel;
    q->tail = &q->sentinel;

    return q;
}

bl_queue_node_t* create_bl_queue_node() {
    bl_queue_node_t *node = (bl_queue_node_t*)malloc(sizeof(bl_queue_node_t));
    init_bl_queue_node(node);

    return node;
}

void init_bl_queue_node(bl_queue_node_t *node) {
    node-> next = NULL;
}

void bl_enqueue(bl_queue_head_t* q, bl_queue_node_t* node) {
    node->next = NULL;

    pthread_mutex_lock(&q->tail_lock);

    q->tail->next = node;
    q->tail = node;

    pthread_mutex_unlock(&q->tail_lock);
}

bl_queue_node_t* bl_dequeue(bl_queue_head_t* q) {
    bl_queue_node_t *head, *next;

    while (1) {
        pthread_mutex_lock(&q->head_lock);
        head = q->head;
        next = head->next;

        if (next == NULL) {
            pthread_mutex_unlock(&q->head_lock);
            return NULL;
        }

        q->head = next;
        pthread_mutex_unlock(&q->head_lock);

        if (head == &q->sentinel) {
            bl_enqueue(q, head);
            continue;
        }

        head->next = NULL;
        return head;
    }
}
