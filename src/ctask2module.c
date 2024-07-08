#include <assert.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

static Py_ssize_t get_len(PyObject *self) {
    return Py_SIZE(self);  // All implementations are PyVarObject
}

static PySequenceMethods sequence_methods = {
    .sq_length = (lenfunc)get_len,
};

/*
 * CircularBuffer
 */
typedef struct CircularBuffer {
    PyObject_VAR_HEAD  // clang-format off
    PyObject **buffer;  // clang-format on
    Py_ssize_t maxlen;
    Py_ssize_t head;
    Py_ssize_t tail;
} CircularBuffer;

static int CircularBuffer_init(CircularBuffer *self, PyObject *args,
                               PyObject *kwargs) {
    Py_ssize_t maxlen;
    static char *keywords[] = {"maxlen", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "n", keywords, &maxlen)) {
        return -1;
    }
    if (maxlen <= 0) {
        PyErr_SetString(PyExc_ValueError, "maxlen must be positive");
        return -1;
    }
    self->maxlen = maxlen;

    self->buffer = (PyObject **)PyMem_Malloc(sizeof(PyObject *) * maxlen);
    if (!self->buffer) {
        PyErr_NoMemory();
        return -1;
    }

    self->head = self->tail = 0;

    return 0;
}

static void CircularBuffer_dealloc(CircularBuffer *self) {
    for (Py_ssize_t i = 0; i < self->maxlen; ++i) {
        Py_XDECREF(self->buffer[i]);
    }
    PyMem_Free(self->buffer);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *CircularBuffer_get_maxlen(CircularBuffer *self,
                                           void *closure) {
    return PyLong_FromSsize_t(self->maxlen);
}

static PyGetSetDef CircularBuffer_getsetters[] = {
    {"maxlen", (getter)CircularBuffer_get_maxlen, NULL, NULL, NULL}, {NULL}};

static PyObject *CircularBuffer_enqueue(CircularBuffer *self, PyObject *args) {
    PyObject *x;
    if (!PyArg_ParseTuple(args, "O", &x)) {
        return NULL;
    }
    Py_INCREF(x);

    if (Py_SIZE(self) < self->maxlen) {
        Py_SET_SIZE(self, Py_SIZE(self) + 1);
    } else {  // trim head
        assert(self->head == self->tail);
        Py_DECREF(self->buffer[self->head]);
        self->head = (self->head + 1) % self->maxlen;
    }
    self->buffer[self->tail] = x;
    self->tail = (self->tail + 1) % self->maxlen;
    Py_RETURN_NONE;
}

static PyObject *CircularBuffer_dequeue(CircularBuffer *self) {
    if (Py_SIZE(self) == 0) {
        PyErr_SetString(PyExc_IndexError, "dequeue from an empty queue");
        return NULL;
    }
    PyObject *x = self->buffer[self->head];
    assert(x != NULL);
    self->head = (self->head + 1) % self->maxlen;
    Py_SET_SIZE(self, Py_SIZE(self) - 1);
    return x;
}

static PyMethodDef CircularBuffer_methods[] = {
    {"enqueue", (PyCFunction)CircularBuffer_enqueue, METH_VARARGS, NULL},
    {"dequeue", (PyCFunction)CircularBuffer_dequeue, METH_NOARGS, NULL},
    {NULL}};

static PyTypeObject CircularBufferType = {
    PyVarObject_HEAD_INIT(NULL, 0)  // clang-format off
    .tp_name = "ctask2.CircularBuffer_c",  // clang-format on
    .tp_basicsize = sizeof(CircularBuffer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)CircularBuffer_init,
    .tp_dealloc = (destructor)CircularBuffer_dealloc,
    .tp_as_sequence = &sequence_methods,
    .tp_getset = CircularBuffer_getsetters,
    .tp_methods = CircularBuffer_methods,
};

/*
 * Node
 */
typedef struct Node {
    PyObject *data;
    struct Node *next;
} Node;

/*
 * CircularLinkedListDynamic
 */
typedef struct CircularLinkedListDynamic {
    PyObject_VAR_HEAD  // clang-format off
    Py_ssize_t maxlen;  // clang-format on
    Node *head;
    Node *tail;
} CircularLinkedListDynamic;

static int CircularLinkedListDynamic_init(CircularLinkedListDynamic *self,
                                          PyObject *args, PyObject *kwargs) {
    Py_ssize_t maxlen;
    static char *keywords[] = {"maxlen", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "n", keywords, &maxlen)) {
        return -1;
    }

    if (maxlen <= 0) {
        PyErr_SetString(PyExc_ValueError, "maxlen must be positive");
        return -1;
    }
    self->maxlen = maxlen;

    self->head = self->tail = NULL;

    return 0;
}

static void CircularLinkedListDynamic_dealloc(CircularLinkedListDynamic *self) {
    Node *current_node = self->head;
    Node *next_node;
    for (Py_ssize_t i = 0; i < Py_SIZE(self); ++i) {
        next_node = current_node->next;
        Py_XDECREF(current_node->data);
        PyMem_Free(current_node);
        current_node = next_node;
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject *CircularLinkedListDynamic_get_maxlen(
    CircularLinkedListDynamic *self, void *closure) {
    return PyLong_FromSsize_t(self->maxlen);
}

static PyGetSetDef CircularLinkedListDynamic_getsetters[] = {
    {"maxlen", (getter)CircularLinkedListDynamic_get_maxlen, NULL, NULL, NULL},
    {NULL}};

static PyObject *CircularLinkedListDynamic_enqueue(
    CircularLinkedListDynamic *self, PyObject *args) {
    PyObject *x;
    if (!PyArg_ParseTuple(args, "O", &x)) {
        return NULL;
    }
    Py_INCREF(x);

    Node *new_node = (Node *)PyMem_Malloc(sizeof(Node));
    if (!new_node) {
        Py_DECREF(x);
        return PyErr_NoMemory();
    }

    if (Py_SIZE(self) == 0) {
        self->head = self->tail = new_node;
    }
    new_node->next = self->head;
    new_node->data = x;
    self->tail->next = new_node;
    self->tail = new_node;

    if (Py_SIZE(self) < self->maxlen) {
        Py_SET_SIZE(self, Py_SIZE(self) + 1);
    } else {  // trim head
        Node *old_head = self->head;
        self->head = old_head->next;
        self->tail->next = self->head;
        Py_DECREF(old_head->data);
        PyMem_Free(old_head);
    }
    Py_RETURN_NONE;
}

static PyObject *CircularLinkedListDynamic_dequeue(
    CircularLinkedListDynamic *self) {
    if (Py_SIZE(self) == 0) {
        PyErr_SetString(PyExc_IndexError, "dequeue from an empty queue");
        return NULL;
    }
    PyObject *x = self->head->data;
    assert(x != NULL);
    if (Py_SIZE(self) == 1) {
        assert(self->head == self->tail);
        PyMem_Free(self->head);
    } else {
        self->head = self->head->next;
        self->tail->next = self->head;
    }
    Py_SET_SIZE(self, Py_SIZE(self) - 1);
    return x;
}

static PyMethodDef CircularLinkedListDynamic_methods[] = {
    {"enqueue", (PyCFunction)CircularLinkedListDynamic_enqueue, METH_VARARGS,
     NULL},
    {"dequeue", (PyCFunction)CircularLinkedListDynamic_dequeue, METH_NOARGS,
     NULL},
    {NULL}};

static PyTypeObject CircularLinkedListDynamicType = {
    PyVarObject_HEAD_INIT(NULL, 0)  // clang-format off
    .tp_name = "ctask2.CircularLinkedListDynamic_c",  // clang-format on
    .tp_basicsize = sizeof(CircularLinkedListDynamic),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)CircularLinkedListDynamic_init,
    .tp_dealloc = (destructor)CircularLinkedListDynamic_dealloc,
    .tp_as_sequence = &sequence_methods,
    .tp_getset = CircularLinkedListDynamic_getsetters,
    .tp_methods = CircularLinkedListDynamic_methods,
};

/*
 * CircularLinkedListStatic
 */
typedef struct CircularLinkedListStatic {
    PyObject_VAR_HEAD  // clang-format off
    Py_ssize_t maxlen;  // clang-format on
    Node *head;
    Node *tail;
} CircularLinkedListStatic;

static int CircularLinkedListStatic_init(CircularLinkedListStatic *self,
                                         PyObject *args, PyObject *kwargs) {
    Py_ssize_t maxlen;
    static char *keywords[] = {"maxlen", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "n", keywords, &maxlen)) {
        return -1;
    }

    if (maxlen <= 0) {
        PyErr_SetString(PyExc_ValueError, "maxlen must be positive");
        return -1;
    }
    self->maxlen = maxlen;

    Node *head = (Node *)PyMem_Malloc(sizeof(Node));
    if (!head) {
        PyErr_NoMemory();
        return -1;
    }

    head->data = NULL;
    head->next = head;
    self->head = self->tail = head;

    Node *new_node;
    for (Py_ssize_t i = 1; i < maxlen; ++i) {
        new_node = (Node *)PyMem_Malloc(sizeof(Node));
        if (!new_node) {  // free previously allocated nodes
            Node *current_node = head;
            Node *next_node;
            for (Py_ssize_t j = 0; j < i; ++j) {
                next_node = current_node->next;
                PyMem_Free(current_node);
                current_node = next_node;
            }
            PyErr_NoMemory();
            return -1;
        }
        new_node->data = NULL;
        new_node->next = head;
        self->tail->next = new_node;
        self->tail = new_node;
    }
    self->tail = self->head;  // queue is empty
    return 0;
}

static void CircularLinkedListStatic_dealloc(CircularLinkedListStatic *self) {
    Node *current_node = self->head;
    Node *next_node;
    for (Py_ssize_t i = 0; i < self->maxlen; ++i) {
        next_node = current_node->next;
        Py_XDECREF(current_node->data);
        PyMem_Free(current_node);
        current_node = next_node;
    }
    Py_TYPE(self)->tp_free(self);
}

static PyObject *CircularLinkedListStatic_get_maxlen(
    CircularLinkedListStatic *self, void *closure) {
    return PyLong_FromSsize_t(self->maxlen);
}

static PyGetSetDef CircularLinkedListStatic_getsetters[] = {
    {"maxlen", (getter)CircularLinkedListStatic_get_maxlen, NULL, NULL, NULL},
    {NULL}};

static PyObject *CircularLinkedListStatic_enqueue(
    CircularLinkedListStatic *self, PyObject *args) {
    PyObject *x;
    if (!PyArg_ParseTuple(args, "O", &x)) {
        return NULL;
    }
    Py_INCREF(x);

    if (Py_SIZE(self) < self->maxlen) {
        Py_SET_SIZE(self, Py_SIZE(self) + 1);
    } else {  // trim head
        assert(self->head == self->tail);
        Py_DECREF(self->head->data);
        self->head = self->head->next;
    }
    self->tail->data = x;
    self->tail = self->tail->next;
    Py_RETURN_NONE;
}

static PyObject *CircularLinkedListStatic_dequeue(
    CircularLinkedListStatic *self) {
    if (Py_SIZE(self) == 0) {
        PyErr_SetString(PyExc_IndexError, "dequeue from an empty queue");
        return NULL;
    }
    PyObject *x = self->head->data;
    assert(x != NULL);
    self->head = self->head->next;
    Py_SET_SIZE(self, Py_SIZE(self) - 1);
    return x;
}

static PyMethodDef CircularLinkedListStatic_methods[] = {
    {"enqueue", (PyCFunction)CircularLinkedListStatic_enqueue, METH_VARARGS,
     NULL},
    {"dequeue", (PyCFunction)CircularLinkedListStatic_dequeue, METH_NOARGS,
     NULL},
    {NULL}};

static PyTypeObject CircularLinkedListStaticType = {
    PyVarObject_HEAD_INIT(NULL, 0)  // clang-format off
    .tp_name = "ctask2.CircularLinkedListStatic_c",  // clang-format on
    .tp_basicsize = sizeof(CircularLinkedListStatic),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)CircularLinkedListStatic_init,
    .tp_dealloc = (destructor)CircularLinkedListStatic_dealloc,
    .tp_as_sequence = &sequence_methods,
    .tp_getset = CircularLinkedListStatic_getsetters,
    .tp_methods = CircularLinkedListStatic_methods,
};

static PyModuleDef ctask2module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "ctask2",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_ctask2(void) {
    PyObject *m;
    m = PyModule_Create(&ctask2module);
    if (m == NULL) return NULL;

    if ((PyModule_AddType(m, &CircularBufferType) < 0) ||
        (PyModule_AddType(m, &CircularLinkedListDynamicType) < 0) ||
        (PyModule_AddType(m, &CircularLinkedListStaticType) < 0)) {
        Py_DECREF(m);
        return NULL;
    }
    return m;
}
