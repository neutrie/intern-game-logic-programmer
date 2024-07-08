#include <math.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

/*
 * counting_sort_c
 */
static PyObject *counting_sort(PyObject *m, PyObject *args, PyObject *kwargs) {
    PyObject *list;
    long long upper_bound;
    static char *keywords[] = {"array", "upper_bound", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!L", keywords,
                                     &PyList_Type, &list, &upper_bound)) {
        return NULL;
    }
    if (upper_bound <= 0) {
        PyErr_SetString(PyExc_ValueError, "upper_bound must be positive");
        return NULL;
    }
    long long *counters =
        (long long *)PyMem_Calloc(upper_bound, sizeof(long long));
    if (!counters) return PyErr_NoMemory();

    for (Py_ssize_t i = 0; i < PyList_GET_SIZE(list); ++i) {
        counters[PyLong_AsLongLong(PyList_GET_ITEM(list, i))] += 1;
    }

    long long current_idx = 0;
    long long end_idx;
    for (long long i = 0; i < upper_bound; ++i) {
        end_idx = current_idx + counters[i];
        while (current_idx < end_idx) {
            PyList_SetItem(list, current_idx, PyLong_FromLongLong(i));
            ++current_idx;
        }
    }
    PyMem_Free(counters);
    Py_RETURN_NONE;
}

/*
 *  sort_c
 */
#define DOUBLE_AT(list, i) PyFloat_AsDouble(PyList_GET_ITEM((list), (i)))

static inline void swap(PyObject *list, Py_ssize_t i, Py_ssize_t j) {
    PyObject *tmp = Py_NewRef(PyList_GET_ITEM(list, i));
    PyList_SetItem(list, i, Py_NewRef(PyList_GET_ITEM(list, j)));
    PyList_SetItem(list, j, tmp);
}

static inline void insertion_sort(PyObject *list, Py_ssize_t left,
                                  Py_ssize_t right) {
    PyObject *item_i, *item_j;
    double key;
    Py_ssize_t j;
    for (Py_ssize_t i = left + 1; i <= right; ++i) {
        item_i = Py_NewRef(PyList_GET_ITEM(list, i));
        key = PyFloat_AsDouble(item_i);
        for (j = i - 1; j >= left; --j) {
            item_j = PyList_GET_ITEM(list, j);
            if (key >= PyFloat_AsDouble(item_j)) break;
            PyList_SetItem(list, j + 1, Py_NewRef(item_j));
        }
        PyList_SetItem(list, j + 1, item_i);
    }
}

static inline void heapify(PyObject *list, Py_ssize_t left, Py_ssize_t right,
                           Py_ssize_t i) {
    Py_ssize_t largest = i;
    Py_ssize_t left_child, right_child;
    while (1) {
        left_child = 2 * i - left + 1;
        right_child = 2 * i - left + 2;
        if (left_child <= right &&
            DOUBLE_AT(list, largest) < DOUBLE_AT(list, left_child)) {
            largest = left_child;
        }
        if (right_child <= right &&
            DOUBLE_AT(list, largest) < DOUBLE_AT(list, right_child)) {
            largest = right_child;
        }
        if (largest == i) {
            break;
        } else {
            swap(list, i, largest);
            i = largest;
        }
    }
}

static inline void heapsort(PyObject *list, Py_ssize_t left, Py_ssize_t right) {
    for (Py_ssize_t i = left + (right - left + 1) / 2 - 1; i >= left; --i) {
        heapify(list, left, right, i);
    }
    for (Py_ssize_t i = right; i > left; --i) {
        swap(list, i, left);
        heapify(list, left, i - 1, left);
    }
}

static inline double median_of_three(double a, double b, double c) {
    if (a <= b) {
        if (b <= c) return b;  // a <= b <= c
        if (a <= c) return c;  // a <= c < b
        return a;              // c < a < b
    } else {
        if (a <= c) return a;  // b < a <= c
        if (b <= c) return c;  // b <= c < a
        return b;              // c < b < a
    }
}

static Py_ssize_t partition(PyObject *list, Py_ssize_t left, Py_ssize_t right) {
    double pivot_val = median_of_three(DOUBLE_AT(list, left),
                                       DOUBLE_AT(list, (left + right) / 2),
                                       DOUBLE_AT(list, right));
    while (1) {
        while (DOUBLE_AT(list, left) < pivot_val) ++left;
        while (DOUBLE_AT(list, right) > pivot_val) --right;
        if (left >= right) return right;
        swap(list, left, right);
        ++left;
        --right;
    }
}

static void introsort(PyObject *list, Py_ssize_t left, Py_ssize_t right,
                      Py_ssize_t depth_limit) {
    if (left < 0 || right < 0 || left >= right) return;

    if ((right - left) < 20) {
        insertion_sort(list, left, right);
    } else if (depth_limit == 0) {
        heapsort(list, left, right);
    } else {
        Py_ssize_t pivot = partition(list, left, right);
        introsort(list, left, pivot, depth_limit - 1);
        introsort(list, pivot + 1, right, depth_limit - 1);
    }
}

static PyObject *sort(PyObject *m, PyObject *args, PyObject *kwargs) {
    PyObject *list;
    Py_ssize_t left;
    Py_ssize_t right;
    static char *keywords[] = {"array", "left", "right", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!nn", keywords,
                                     &PyList_Type, &list, &left, &right)) {
        return NULL;
    }
    Py_ssize_t depth_limit = (Py_ssize_t)(2 * log2(right - left + 1));
    introsort(list, left, right, depth_limit);
    Py_RETURN_NONE;
}

static PyMethodDef ctask3module_methods[] = {
    {"sort_c", (PyCFunctionWithKeywords)sort, METH_VARARGS | METH_KEYWORDS,
     NULL},
    {"counting_sort_c", (PyCFunctionWithKeywords)counting_sort,
     METH_VARARGS | METH_KEYWORDS, NULL},
    {NULL}};

static struct PyModuleDef ctask3module = {PyModuleDef_HEAD_INIT, "ctask3", NULL,
                                          -1, ctask3module_methods};

PyMODINIT_FUNC PyInit_ctask3(void) { return PyModule_Create(&ctask3module); }
