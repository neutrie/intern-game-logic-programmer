import random

import pytest
from ctask3 import counting_sort_c, sort_c
from task3 import counting_sort, sort

random.seed(128)


@pytest.fixture
def hexadecimal():
    return [
        [9, 8, 6, 2, 12],
        [15, 0, 15, 0, 7],
        [3, 3, 1, 1, 5],
        [7] * 10,
        [7],
    ]


@pytest.fixture
def floats():
    return [
        [7.79, 3.05, 9.73, 2.87, 1.42],
        [9.04, 9.20, 7.53, 2.29, 8.03],
        [5.5, 4.4, 3.3, 2.2, 1.1],
        [random.random() for _ in range(80)],
        [7.7] * 10,
        [7.7],
    ]


@pytest.mark.parametrize("counting_sort", (counting_sort, counting_sort_c))
def test_counting_sort(counting_sort, hexadecimal):
    for array in hexadecimal:
        array_copy = array.copy()
        counting_sort(array_copy, upper_bound=0x10)
        assert array_copy == sorted(array)


@pytest.mark.parametrize("sort", (sort, sort_c))
def test_sort(sort, hexadecimal, floats):
    for array in hexadecimal + floats:
        array_copy = array.copy()
        sort(array_copy, 0, len(array_copy) - 1)
        assert array_copy == sorted(array)


if __name__ == "__main__":
    pytest.main()
