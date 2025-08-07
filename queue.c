#include "queue.h"


struct queue *init_queue(size_t qsiz, size_t M, uint8_t nprod) {
    struct queue *ret = (struct queue *)malloc(sizeof(struct queue));
    assert(ret);

    ret->M = M;
    ret->matsiz = M*M;

    ret->qsiz = qsiz;
    ret->nprod = nprod;

    ret->count = ret->in = ret->out = 0;

    int err = pthread_mutex_init(&ret->mut, NULL);
    if (err) {
        exit_with_err("pthread_mutex_init", err);
    }

    err = pthread_cond_init(&ret->not_empty, NULL);
    if (err) {
        exit_with_err("1 pthread_cond_init", err);
    }

    err = pthread_cond_init(&ret->not_full, NULL);
    if (err) {
        exit_with_err("2 pthread_cond_init", err);
    }

    ret->vec = (uint8_t **)malloc(qsiz * sizeof(uint8_t *));
    for (size_t i = 0; i < qsiz; i++) {
        ret->vec[i] = (uint8_t *)malloc(ret->matsiz * sizeof(uint8_t));
        assert(ret->vec[i]);
    }

    return ret;
}

void destroy_queue(struct queue *q) {
    assert(q);

    for (size_t i = 0; i < q->qsiz; i++) {
        free(q->vec[i]);
    }

    int err = pthread_mutex_destroy(&q->mut);
    if (err) {
        exit_with_err("pthread_mutex_destroy", err);
    }

    err = pthread_cond_destroy(&q->not_empty);
    if (err) {
        exit_with_err("1 pthread_cond_destroy", err);
    }

    err = pthread_cond_destroy(&q->not_full);
    if (err) {
        exit_with_err("2 pthread_cond_destroy", err);
    }

    free(q);

}

void enqueue(struct queue *q, uint8_t *new) {

    assert(q && new);

    lockm(&q->mut);
    while (q->count == q->qsiz) {
        waitc(&q->not_full, &q->mut);
    }

    memcpy(q->vec[q->in], new, sizeof(uint8_t) * q->matsiz);
    q->in = (q->in + 1) % q->qsiz;
    if (q->count++ == 0) {
        broadc(&q->not_empty);
    }

    unlockm(&q->mut);
}

bool dequeue(struct queue *q, uint8_t *new) {

    assert(q && new);
    bool ret = true;

    lockm(&q->mut);
    while (q->count == 0 && q->nprod > 0) {
        waitc(&q->not_empty, &q->mut);
    }

    if (q->nprod == 0) {
        ret = false;
    } else {

        memcpy(new, q->vec[q->out], q->matsiz);
        q->out = (q->out + 1) % q->qsiz;
        if (q->count-- == q->qsiz) {
            broadc(&q->not_full);
        }

    }
    unlockm(&q->mut);
    return ret;
}

bool decrement_prod(struct queue *q) {
    bool ret = true;
    lockm(&q->mut);

    if (q->nprod == 0) ret = false; 
    else q->nprod--;
    
    if (q->nprod == 0) broadc(&q->not_empty);
        
    unlockm(&q->mut);
    return ret;
}

bool is_semi_magic(uint8_t *m, size_t M, uint16_t *magic) {

    uint8_t sum = 0;
    for (size_t i = 0; i < M; i++) {
        sum += ind(m, M, 0, i);
    }
    if (magic) *magic = sum;

    for (size_t i = 0; i < M; i++) {

        uint8_t ture = 0;
        if (i != 0) {
            // Rows
            for (size_t j = 0; j < M; j++) {
                ture += ind(m, M, i, j);
            }
            if (ture != sum) return false;
        }

        // Cols
        ture = 0;
        for (size_t j = 0; j < M; j++) {
            ture += ind(m, M, j, i);
        }
        if (ture != sum) return false;
    }

    return true;
}

void print_mat(uint8_t *m, size_t M, bool nl) {

    if (nl) printf("\n");
    for (size_t i = 0; i < M; i++) {
        printf("(");
        for (size_t j = 0; j < M; j++) {
            printf("%d", ind(m, M, i, j));
            if (j != M-1) printf(", ");
        }
        if (nl) {
            printf(")\n");
        } else {
            if (i != M-1) printf(") "); else printf(")");
        }
    }
    if (!nl) printf("\n");

}
