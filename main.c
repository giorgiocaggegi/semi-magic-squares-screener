#include "queue.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct reader_tdata {
    struct queue *q;
    pthread_mutex_t *somut;
    const char *path;
    uint8_t no;
    size_t M;
};
void *reader_tfunc(void *arg) {

    struct reader_tdata *t = (struct reader_tdata *)arg;
    size_t matsiz = t->M*t->M;
    
    int fd = open(t->path, O_RDONLY);
    if (fd == -1) {
        exit_with_sys_err("open");
    }

    lockm(t->somut); 
    printf("[READER-%d]: opened \'%s\'\n", t->no, t->path); 
    unlockm(t->somut);

    struct stat st;
    if (fstat(fd, &st) == -1) {
        exit_with_sys_err("fstat");
    }

    uint8_t *mat = (uint8_t *)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mat == MAP_FAILED) {
        exit_with_sys_err("mmap");
    }
    close(fd);

    size_t mat_count = st.st_size / matsiz;

    for (size_t k = 0; k < mat_count; k++) {
        lockm(t->somut); 
        printf("[READER-%d]: candidate n.%zu: ", t->no, k); 
        print_mat(mat + (k * matsiz), t->M, false);
        unlockm(t->somut);
        enqueue(t->q, mat + (k * matsiz));
    }

    decrement_prod(t->q);

    free(t);
    return NULL;
}

struct verifier_tdata {
    struct queue *inter;
    struct queue *final;
    pthread_mutex_t *somut;
    size_t M;
};
void *verifier_tfunc(void *arg) {
    struct verifier_tdata *t = (struct verifier_tdata *)arg;

    bool work = true;
    do {
        uint8_t ture[t->M*t->M];
        work = dequeue(t->inter, ture);
        if (work && is_semi_magic(ture, t->M, NULL)) {
            lockm(t->somut);
            printf("[VERIFIER]: found a semi-magic!\n");
            enqueue(t->final, ture);
            unlockm(t->somut);
        }

    } while (work);

    decrement_prod(t->final);

    free(t);
    return NULL;
}

size_t check_args(int argc, char *argv[]) {

    char *ture = "Usage: <%s> <(3<=M<=16)> <path1> ...\n";

    if (argc < 3) {
        exit_with_err_msg(ture, argv[0]);
    }

    int ret = atoi(argv[1]);
    if (ret < 3 || ret > 16) {
        exit_with_err_msg(ture, argv[0]);
    }

    return (size_t)ret;
}

int main(int argc, char *argv[]) {

    size_t M = check_args(argc, argv);

    pthread_mutex_t *somut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    int err = pthread_mutex_init(somut, NULL);
    if (err) {
        exit_with_err("pthread_mutex_init", err);
    }

    struct queue *inter = init_queue(5, M, argc-2);
    struct queue *final = init_queue(1, M, 1);

    printf("Creating 1 verif and %d reader threads\n", argc-2);

    for (size_t i = 0; i < argc-2; i++) {
        struct reader_tdata *t = (struct reader_tdata *)malloc(sizeof(struct reader_tdata));
        assert(t);
        t->q = inter; t->somut = somut; t->path = argv[2+i]; t->no = i; t->M = M;
        pthread_t tid;
        err = pthread_create(&tid, NULL, reader_tfunc, t);
        if (err != 0) {
            exit_with_err("pthread_create", err);
        } else {
            err = pthread_detach(tid);
            if (err != 0) {
                exit_with_err("pthread_detach", err);
            }
        }
    }

    pthread_t tid;
    struct verifier_tdata *t = (struct verifier_tdata *)malloc(sizeof(struct verifier_tdata));
    assert(t);
    t->inter = inter; t->final = final; t->somut = somut; t->M = M;
    err = pthread_create(&tid, NULL, verifier_tfunc, t);
    if (err != 0) {
        exit_with_err("pthread_create", err);
    } else {
        err = pthread_detach(tid);
        if (err != 0) {
            exit_with_err("pthread_detach", err);
        }
    }

    uint8_t ture[M*M];
    bool work = true;
    size_t found = 0;
    while (work) {
        work = dequeue(final, ture);
        if (work) {

            lockm(t->somut);
            printf("---\n[MAIN]: found a semi-magic!");
            print_mat(ture, t->M, true);
            printf("---\n");
            unlockm(t->somut);

            found++;
        }
    }
    printf("[MAIN]: terminating with %zu semi-magic square found\n", found);

}