#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sorting.h"
void print_array(const char* prefix, int arr[], int n) {
    printf("%s: [", prefix);
    for (int i = 0; i < n; i++) { printf("%d%s", arr[i], (i < n - 1) ? ", " : ""); }
    printf("]\n");
}
int run_test(void (*sort_func)(int*, size_t), const char* func_name) {
    printf("--- Testing %s ---\n", func_name);
    int test_arr[] = {64, 34, 25, 12, 22, 11, 90};
    int expected_arr[] = {11, 12, 22, 25, 34, 64, 90};
    int n = sizeof(test_arr) / sizeof(test_arr[0]);
    print_array("Original", test_arr, n);
    sort_func(test_arr, n);
    print_array("Sorted", test_arr, n);
    if (memcmp(test_arr, expected_arr, sizeof(test_arr)) == 0) {
        printf("Result: PASS\n\n"); return 0;
    } else {
        printf("Result: FAIL\n\n"); return 1;
    }
}
int main() {
    int status = 0;
    status |= run_test(bubble_sort, "Bubble Sort");
    status |= run_test(insertion_sort, "Insertion Sort");
    if (status == 0) { printf("All tests passed!\n"); return 0; }
    else { printf("One or more tests failed.\n"); return 1; }
}
