#include <stddef.h>
#include <stdint.h>

// comparison function for qsort()
int compare_uint32_t(const void* a, const void* b);

// remove duplicate elements of sorted array and return the new ending of the array
// containing only unique elements
uint32_t removeDup(uint32_t arr[], size_t n);
