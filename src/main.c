#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 10
#define VALUE_MIN -100
#define VALUE_MAX 100

static int max_in_array(const int *arr, size_t len) {
    size_t i;
    int max_value;

    if (arr == NULL || len == 0) { return INT_MIN; }

    max_value = arr[0];
    for (i = 1; i < len; ++i) {
        if (arr[i] > max_value) { max_value = arr[i]; }
    }

    return max_value;
}

int main(void) {
    int values[ARRAY_SIZE];
    int result;
    size_t i;
    int range;

    srand((unsigned int)time(NULL));
    range = VALUE_MAX - VALUE_MIN + 1;

    printf("Array: ");
    for (i = 0; i < ARRAY_SIZE; ++i) {
        values[i] = VALUE_MIN + (rand() % range);
        printf("%d", values[i]);
        if (i + 1 < ARRAY_SIZE) { printf(" "); }
    }
    printf("\n");

    result = max_in_array(values, ARRAY_SIZE);
    printf("Max: %d\n", result);

    return 0;
}
