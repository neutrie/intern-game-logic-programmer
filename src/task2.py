from collections import deque
from timeit import timeit
from typing import Any

try:
    import ctask2
except ImportError:
    ctask2 = None


class CircularList:
    def __init__(self, maxlen: int) -> None:
        if maxlen <= 0:
            raise ValueError("maxlen must be positive")
        self._buffer: list[Any] = [None] * maxlen
        self._maxlen = maxlen
        self._len = 0
        self._head = 0
        self._tail = 0

    @property
    def maxlen(self) -> int:
        return self._maxlen

    def enqueue(self, x: Any, /) -> None:
        if self._len < self._maxlen:
            self._len += 1
        else:  # trim head
            self._buffer[self._head] = None
            self._head = (self._head + 1) % self._maxlen
        self._buffer[self._tail] = x
        self._tail = (self._tail + 1) % self._maxlen

    def dequeue(self) -> Any:
        if self._len == 0:
            raise IndexError("dequeue from an empty queue")
        x = self._buffer[self._head]
        self._buffer[self._head] = None
        self._head = (self._head + 1) % self._maxlen
        self._len -= 1
        return x

    def __len__(self) -> int:
        return self._len


class Node:
    def __init__(self, data: Any, /) -> None:
        self.data = data
        self.next: Node | None = None


class CircularLinkedListDynamic:
    def __init__(self, maxlen: int) -> None:
        if maxlen <= 0:
            raise ValueError("maxlen must be positive")
        self._maxlen = maxlen
        self._len = 0
        self._head: Node | None = None
        self._tail: Node | None = None

    @property
    def maxlen(self) -> int:
        return self._maxlen

    def enqueue(self, x: Any, /) -> None:
        new_node = Node(x)
        if self._len == 0:
            self._head = self._tail = new_node
        new_node.next = self._head
        self._tail.next = new_node
        self._tail = new_node

        if self._len < self._maxlen:
            self._len += 1
        else:  # trim head
            self._head.data = None
            self._head = self._head.next
            self._tail.next = self._head

    def dequeue(self) -> Any:
        if self._len == 0:
            raise IndexError("dequeue from an empty queue")
        x = self._head.data
        if self._len == 1:
            self._head = self._tail = None
        else:
            self._head = self._head.next
            self._tail.next = self._head
        self._len -= 1
        return x

    def __len__(self) -> int:
        return self._len


class CircularLinkedListStatic:
    def __init__(self, maxlen: int) -> None:
        if maxlen <= 0:
            raise ValueError("maxlen must be positive")
        self._maxlen = maxlen
        self._len = 0
        self._head = self._tail = Node(None)
        self._head.next = self._head
        for _ in range(maxlen - 1):
            new_node = Node(None)
            new_node.next = self._head
            self._tail.next = new_node
            self._tail = new_node
        self._tail = self._head  # queue is empty

    @property
    def maxlen(self) -> int:
        return self._maxlen

    def enqueue(self, x: Any, /) -> None:
        if self._len < self._maxlen:
            self._len += 1
        else:  # trim head
            self._head = self._head.next
        self._tail.data = x
        self._tail = self._tail.next

    def dequeue(self) -> Any:
        if self._len == 0:
            raise IndexError("dequeue from an empty queue")
        x = self._head.data
        self._head.data = None
        self._head = self._head.next
        self._len -= 1
        return x

    def __len__(self) -> int:
        return self._len


class CircularDeque(deque[Any]):
    def __init__(self, *args: Any, maxlen: int, **kwargs: Any):
        if maxlen <= 0:
            raise ValueError("maxlen must be positive")
        super().__init__(*args, maxlen=maxlen, **kwargs)
        self.enqueue = self.append
        self.dequeue = self.popleft


def use_buffer(buffer):
    for i in range(buffer.maxlen):
        buffer.enqueue(i)
    for i in range(buffer.maxlen):
        _ = buffer.dequeue()


def main():
    maxlen = 2**16
    iterations = 1_000
    buffers = [
        CircularList(maxlen=maxlen),
        CircularLinkedListDynamic(maxlen=maxlen),
        CircularLinkedListStatic(maxlen=maxlen),
        CircularDeque(maxlen=maxlen),
    ]
    try:
        buffers.append(ctask2.CircularBuffer_c(maxlen=maxlen))
        buffers.append(ctask2.CircularLinkedListDynamic_c(maxlen=maxlen))
        buffers.append(ctask2.CircularLinkedListStatic_c(maxlen=maxlen))
    except AttributeError:
        pass
    for buffer in buffers:
        elapsed_time = timeit(lambda: use_buffer(buffer), number=iterations)
        print(type(buffer).__name__, elapsed_time, sep="\n\t")


if __name__ == "__main__":
    main()
