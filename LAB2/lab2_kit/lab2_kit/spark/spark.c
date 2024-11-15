#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CACHE_MIN (4 * 1024)        // 4 KB
#define CACHE_MAX (4 * 1024 * 1024) // 4 MB
#define N_REPETITIONS (100)
#define TARGET_STRIDE (2048)        // Stride alvo

// retorna o tempo decorrido em segundos
double get_elapsed(struct timespec const *start) {
    struct timespec end;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    double nanoseconds = ((end.tv_sec - start->tv_sec) * 1000000000) +
                         (end.tv_nsec - start->tv_nsec);
    return nanoseconds / 1000000000.0;
}

int main() {
    uint8_t *array = calloc(CACHE_MAX, sizeof(uint8_t));

    fputs("size\t   stride\t  elapsed(s)\t  cycles\t  accesses\t mean_access_time(s)\n", stdout);

    for (size_t cache_size = CACHE_MIN; cache_size <= CACHE_MAX; cache_size = cache_size * 2) {
        fprintf(stderr, "[LOG]: running with array of size %zu KiB\n", cache_size >> 10);
        fflush(stderr);
        
        for (size_t stride = 1; stride <= cache_size / 2; stride = 2 * stride) {
            size_t limit = cache_size - stride + 1;

            // Condição para medir apenas quando o stride for 2048
            if (stride != TARGET_STRIDE) {
                continue;  // Pule as outras iterações
            }

            // Aquece o cache
            for (size_t index = 0; index < limit; index += stride) {
                array[index] = array[index] + 1;
            }

            clock_t const start_cycles = clock();
            struct timespec start_time;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);

            size_t n_iterations = 0;
            size_t total_accesses = 0;  // Número total de acessos

            // **************************************************************
            for (size_t repeat = 0; repeat < N_REPETITIONS * stride; repeat++) {
                for (size_t index = 0; index < limit; index += stride, n_iterations++) {
                    array[index] = array[index] + 1;
                    total_accesses++;  // Conta um acesso
                }
            }
            // **************************************************************

            clock_t const cycle_count = clock() - start_cycles;
            double const time_diff = get_elapsed(&start_time);

            // Calcula o mean access time
            double mean_access_time = time_diff / total_accesses;

            // Saída para stdout
            fprintf(stdout, "%-10zu %-10zu %-15.6lf %-10zu %-10zu %-15.9lf\n",
                    cache_size, stride, time_diff, cycle_count, total_accesses, mean_access_time);
        }
    }

    free(array);
    return 0;
}