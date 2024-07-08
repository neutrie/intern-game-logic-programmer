import pytest
from ctask2 import (
    CircularBuffer_c,
    CircularLinkedListDynamic_c,
    CircularLinkedListStatic_c,
)
from task2 import (
    CircularDeque,
    CircularLinkedListDynamic,
    CircularLinkedListStatic,
    CircularList,
)


@pytest.mark.parametrize(
    "buffer_class",
    [
        CircularList,
        CircularLinkedListDynamic,
        CircularLinkedListStatic,
        CircularDeque,
        CircularBuffer_c,
        CircularLinkedListDynamic_c,
        CircularLinkedListStatic_c,
    ],
)
def test_buffer(buffer_class):
    buffer = buffer_class(maxlen=5)
    assert len(buffer) == 0
    # []

    buffer.enqueue(1)
    buffer.enqueue(2)
    assert len(buffer) == 2
    # [1, 2]

    assert 1 == buffer.dequeue()
    assert len(buffer) == 1
    # [2]

    buffer.enqueue(3)
    buffer.enqueue(4)
    buffer.enqueue(5)
    buffer.enqueue(6)
    assert len(buffer) == 5
    # [2, 3, 4, 5, 6]

    buffer.enqueue(7)
    assert len(buffer) == 5
    # [3, 4, 5, 6, 7]

    assert 3 == buffer.dequeue()
    assert len(buffer) == 4
    # [4, 5, 6, 7]

    assert 4 == buffer.dequeue()
    assert len(buffer) == 3
    # [5, 6, 7]

    buffer = buffer_class(maxlen=1)
    buffer.enqueue(1)
    assert len(buffer) == 1
    # [1]

    buffer.enqueue(2)
    assert len(buffer) == 1
    # [2]

    assert buffer.dequeue() == 2
    assert len(buffer) == 0
    # []

    with pytest.raises(IndexError):
        buffer.dequeue()

    with pytest.raises(ValueError):
        buffer = buffer_class(maxlen=-1)


if __name__ == "__main__":
    pytest.main()
