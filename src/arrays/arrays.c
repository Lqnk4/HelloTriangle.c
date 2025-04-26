#include "arrays/arrays.h"
#include <stddef.h>
#include <stdint.h>

int compare_uint32_t(const void* a, const void* b) {
    return (*(uint32_t*)a - *(uint32_t*)b);
}

uint32_t removeDup(uint32_t arr[], size_t n) {
    if (n == 0)
        return 0;

    int j = 0;
    for (size_t i = 1; i < n - 1; i++) {

        // If a unique element is found, place
        // it at arr[j + 1]
        if (arr[i] != arr[j])
            arr[++j] = arr[i];
    }

    // Return the new ending of arr that only
    // contains unique elements
    return j + 1;
}
