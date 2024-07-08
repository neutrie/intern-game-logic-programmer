import math
import random
from timeit import timeit
from typing import Any

try:
    import ctask3
except ImportError:
    ctask3 = None


def counting_sort(array: list[int], upper_bound: int) -> None:
    counters = [0] * upper_bound
    for value in array:
        counters[value] += 1
    current_idx = 0
    for i in range(upper_bound):
        end_idx = current_idx + counters[i]
        while current_idx < end_idx:
            array[current_idx] = i
            current_idx += 1


def insertion_sort(array: list[Any], left: int, right: int) -> None:
    for i in range(left + 1, right + 1):
        key = array[i]
        j = i - 1
        while j >= left and key < array[j]:
            array[j + 1] = array[j]
            j -= 1
        array[j + 1] = key


def heapify(array: list[Any], left: int, right: int, i: int) -> None:
    largest = i
    while True:
        left_child = 2 * i - left + 1
        right_child = 2 * i - left + 2
        if left_child <= right and array[largest] < array[left_child]:
            largest = left_child
        if right_child <= right and array[largest] < array[right_child]:
            largest = right_child
        if largest == i:
            break
        else:
            array[i], array[largest] = array[largest], array[i]
            i = largest


def heapsort(array: list[Any], left: int, right: int) -> None:
    for i in range(left + (right - left + 1) // 2 - 1, left - 1, -1):
        heapify(array, left, right, i)
    for i in range(right, left, -1):
        array[i], array[left] = array[left], array[i]
        heapify(array, left, i - 1, left)


def median_of_three(a: int, b: int, c: int) -> int:
    if a <= b:
        if b <= c:  # a <= b <= c
            return b
        if a <= c:  # a <= c < b
            return c
        return a  # c < a < b
    else:
        if a <= c:  # b < a <= c
            return a
        if b <= c:  # b <= c < a
            return c
        return b  # c < b < a


def partition(array: list[Any], left: int, right: int) -> int:
    pivot_val = median_of_three(array[left], array[(left + right) // 2], array[right])
    while True:
        while array[left] < pivot_val:
            left += 1
        while array[right] > pivot_val:
            right -= 1
        if left >= right:
            return right
        array[left], array[right] = array[right], array[left]
        left += 1
        right -= 1


def introsort(array: list[Any], left: int, right: int, depth_limit: int) -> None:
    if left < 0 or right < 0 or left >= right:
        return

    if right - left < 20:
        insertion_sort(array, left, right)
    elif depth_limit == 0:
        heapsort(array, left, right)
    else:
        pivot = partition(array, left, right)
        introsort(array, left, pivot, depth_limit - 1)
        introsort(array, pivot + 1, right, depth_limit - 1)


def sort(array: list[Any], left: int, right: int) -> None:
    depth_limit = int(2 * math.log2(right - left + 1))
    introsort(array, left, right, depth_limit)


def builtin(array: list[Any], *args: Any, **kwargs: Any) -> None:
    array.sort()


def main():
    array_size = 2**16
    iterations = 1_000

    random.seed(128)

    # [7, 12, 11...]
    hexadecimal_digits = [random.randrange(0x0, 0x10) for _ in range(array_size)]

    # [0.08834126925343144, 0.9763209982919767, 0.18086510167640257...]
    unsorted_array = [random.random() for _ in range(array_size)]

    # [0.00012058646377621773, 0.00012629879167436187, 0.00013528730329082084...]
    sorted_array = sorted(unsorted_array)

    def print_elapsed(func, array_description, elapsed):
        template_string = "{}({})\n\t{}"
        print(template_string.format(func.__name__, array_description, elapsed))

    counting_sorts = [counting_sort]
    try:
        counting_sorts.append(ctask3.counting_sort_c)
    except AttributeError:
        pass
    for f in counting_sorts:
        elapsed = timeit(
            lambda: f(hexadecimal_digits, upper_bound=0x10), number=iterations
        )
        print_elapsed(f, "Hexadecimal digits", elapsed)

    sorts = [builtin, sort]
    try:
        sorts.append(ctask3.sort_c)
    except AttributeError:
        pass
    for array, array_description in (
        (hexadecimal_digits, "Hexadecimal digits"),
        (unsorted_array, "Unsorted array"),
        (sorted_array[::-1], "Reverse sorted array"),
        (sorted_array, "Sorted array"),
    ):
        for g in sorts:
            array_copy = array.copy()
            elapsed = timeit(
                lambda: g(array_copy, 0, len(array) - 1), number=iterations
            )
            print_elapsed(g, array_description, elapsed)
        print()


if __name__ == "__main__":
    main()
