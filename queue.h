#include "lib-misc.h"
#include <pthread.h>
#include <stdbool.h>
#include <inttypes.h>

#define lockm(m) \
    do { \
        int err = pthread_mutex_lock((m)); \
        if (err) { \
            exit_with_err("pthread_mutex_lock", err); \
        } \
    } while (0) 

#define unlockm(m) \
    do { \
        int err = pthread_mutex_unlock((m)); \
        if (err) { \
            exit_with_err("pthread_mutex_unlock", err); \
        } \
    } while (0) 

#define waitc(c, m) \
    do { \
        int err = pthread_cond_wait((c), (m)); \
        if (err) { \
            exit_with_err("pthread_cond_wait", err); \
        } \
    } while (0) 

#define broadc(c) \
    do { \
        int err = pthread_cond_broadcast((c)); \
        if (err) { \
            exit_with_err("pthread_cond_broadcast", err); \
        } \
    } while (0) 
#define ind(mat, M, i, j) *((mat)+(i)*(M)+(j))
#define dind(mat, M, i, j, k) *((mat)+((k)*(M)*(M))+((i)*(M)+(j)))

struct queue {
    uint8_t **vec;
    size_t matsiz;
    size_t M;

    size_t qsiz;
    uint8_t nprod;
    size_t count;
    size_t in;
    size_t out;

    pthread_mutex_t mut;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
};


struct queue *init_queue(size_t qsiz, size_t M, uint8_t nprod);
void enqueue(struct queue *q, uint8_t *new);
bool dequeue(struct queue *q, uint8_t *new);
bool decrement_prod(struct queue *q);
bool is_semi_magic(uint8_t *m, size_t M, uint16_t *magic);
void print_mat(uint8_t *m, size_t M, bool ture);