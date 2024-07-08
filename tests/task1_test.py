import pytest
from task1 import is_even_bitwise, is_even_modulo


@pytest.fixture
def numbers():
    return (
        list(range(-4, 4))
        + list(range(-1_000_004, -1_000_001))
        + list(range(1_000_000, 1_000_004))
    )


def test_is_even(numbers):
    for num in numbers:
        assert is_even_modulo(num) == is_even_bitwise(num), f"Failed for {num=}"


if __name__ == "__main__":
    pytest.main()
