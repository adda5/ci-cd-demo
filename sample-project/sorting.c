#include "sorting.h"
void swap(int *a, int *b) { int temp = *a; *a = *b; *b = temp; }
void bubble_sort(int *arr, size_t n) {
    if (n < 2) return;
    for (size_t i = 0; i < n - 1; i++)
        for (size_t j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1]) swap(&arr[j], &arr[j + 1]);
}
void insertion_sort(int *arr, size_t n) {
    if (n < 2) return;
    for (size_t i = 1; i < n; i++) {
        int key = arr[i]; int j = i - 1;
        while (j >= 0 && arr[j] > key) { arr[j + 1] = arr[j]; j--; }
        arr[j + 1] = key;
    }
}
