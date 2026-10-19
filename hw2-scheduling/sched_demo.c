// sched_demo.c — Assignment II ready (affinity + policy/priority)
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   // ★ 新增：for strncasecmp
#include <pthread.h>
#include <getopt.h>
#include <time.h>
#include <sched.h>
#include <errno.h>
#include <ctype.h>

static int num_threads = 0;
static double busy_wait_sec = 0.0;
static char *policies = NULL;    // "NORMAL,FIFO,..."
static char *priorities = NULL;  // "-1,30,..."
static pthread_barrier_t start_bar;

static double diff_sec(struct timespec a, struct timespec b){
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec)/1e9;
}

static void busy_wait(double t){
    struct timespec s, c;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &s);
    for(;;){
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &c);
        if (diff_sec(s, c) >= t) break;
    }
}

static void pin_to_cpu0(void){
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);
}

struct thread_arg { int id; };

static int parse_csv_policy(const char* s, int *out, int maxn){
    int n = 0; const char *p = s;
    while (p && *p && n < maxn){
        const char *q = strchr(p, ',');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        while (len && isspace((unsigned char)p[0])) { p++; len--; }
        while (len && isspace((unsigned char)p[len-1])) len--;
        if (len == 6 && strncasecmp(p, "NORMAL", 6)==0) out[n++] = SCHED_OTHER;
        else if (len == 4 && strncasecmp(p, "FIFO", 4)==0) out[n++] = SCHED_FIFO;
        else return -1;
        if (!q) break; else p = q + 1;
    }
    return n;
}
static int parse_csv_int(const char* s, int *out, int maxn){
    int n = 0; const char *p = s;
    while (p && *p && n < maxn){
        char *end = NULL;
        long v = strtol(p, &end, 10);
        if (end == p) return -1;
        out[n++] = (int)v;
        if (*end == ',') p = end + 1; else break;
    }
    return n;
}

static void* worker(void *arg){
    int id = ((struct thread_arg*)arg)->id;
    pin_to_cpu0();
    pthread_barrier_wait(&start_bar);
    for (int round=0; round<3; ++round){
        printf("Thread %d is running\n", id);
        fflush(stdout);
        busy_wait(busy_wait_sec);
    }
    return NULL;
}

int main(int argc, char **argv){
    static struct option opts[] = {
        {"num-threads", required_argument, 0, 'n'},
        {"time",        required_argument, 0, 't'},
        {"sched-policy",required_argument, 0, 's'},
        {"priority",    required_argument, 0, 'p'},
        {0,0,0,0}
    };
    int c;
    while ((c = getopt_long(argc, argv, "n:t:s:p:", opts, NULL)) != -1){
        switch(c){
            case 'n': num_threads   = atoi(optarg); break;
            case 't': busy_wait_sec = atof(optarg); break;
            case 's': policies      = optarg;       break;
            case 'p': priorities    = optarg;       break;
            default:
                fprintf(stderr, "Usage: %s -n N -t SEC -s POLS -p PRIOS\n", argv[0]);
                return 1;
        }
    }
    if (num_threads <= 0 || busy_wait_sec <= 0){
        fprintf(stderr, "Require -n > 0 and -t > 0\n");
        return 1;
    }
    if (!policies || !priorities){
        fprintf(stderr, "Require -s <policies> and -p <priorities>\n");
        return 1;
    }

    int *pol  = calloc(num_threads, sizeof(int));
    int *prio = calloc(num_threads, sizeof(int));
    int np = parse_csv_policy(policies, pol, num_threads);
    int nq = parse_csv_int(priorities, prio, num_threads);
    if (np != num_threads || nq != num_threads){
        fprintf(stderr, "-s/-p counts must match -n (got %d,%d vs %d)\n", np, nq, num_threads);
        return 1;
    }
    for (int i=0;i<num_threads;i++){
        if (pol[i]==SCHED_OTHER && prio[i]!=-1){
            fprintf(stderr, "Thread %d: NORMAL must have priority -1\n", i);
            return 1;
        }
        if (pol[i]==SCHED_FIFO && (prio[i]<1 || prio[i]>99)){
            fprintf(stderr, "Thread %d: FIFO priority must be 1..99\n", i);
            return 1;
        }
    }

    pin_to_cpu0(); // main 綁 CPU0

    pthread_t *th = calloc(num_threads, sizeof(pthread_t));
    pthread_attr_t *attr = calloc(num_threads, sizeof(pthread_attr_t));
    struct thread_arg *args = calloc(num_threads, sizeof(*args));
    pthread_barrier_init(&start_bar, NULL, num_threads + 1);

    for (int i=0;i<num_threads;i++){
        args[i].id = i;

        pthread_attr_init(&attr[i]);
        pthread_attr_setinheritsched(&attr[i], PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr[i], pol[i]);

        struct sched_param sp;
        memset(&sp, 0, sizeof(sp));
        sp.sched_priority = (pol[i]==SCHED_FIFO) ? prio[i] : 0;
        pthread_attr_setschedparam(&attr[i], &sp);

        // ★ 修正：用回傳值判斷錯誤，而不是 perror(errno)
        int ret = pthread_create(&th[i], &attr[i], worker, &args[i]);
        if (ret != 0){
            fprintf(stderr,
                "pthread_create[%d] failed: %s (ret=%d, policy=%s, prio=%d)\n",
                i, strerror(ret), ret,
                (pol[i]==SCHED_FIFO ? "FIFO" : "NORMAL"), prio[i]);
            return 2;
        }
    }

    pthread_barrier_wait(&start_bar);
    for (int i=0;i<num_threads;i++) pthread_join(th[i], NULL);

    for (int i=0;i<num_threads;i++) pthread_attr_destroy(&attr[i]);
    pthread_barrier_destroy(&start_bar);
    free(attr); free(th); free(args); free(pol); free(prio);
    return 0;
}
