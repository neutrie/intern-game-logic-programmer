from timeit import timeit


def is_even_modulo(value: int) -> bool:
    return value % 2 == 0


def is_even_bitwise(value: int) -> bool:
    return value & 1 == 0


def main():
    small_int = 100
    big_int = 1_000_000
    iterations = 10_000_000
    for num in (small_int, big_int):
        for func in (is_even_modulo, is_even_bitwise):
            elapsed = timeit(
                f"{func.__name__}({num})", globals=globals(), number=iterations
            )
            print(f"{func.__name__}({num})\n\t{elapsed}")


if __name__ == "__main__":
    main()
