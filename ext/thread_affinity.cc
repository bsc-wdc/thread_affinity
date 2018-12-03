/*         
 *  Copyright 2002-2018 Barcelona Supercomputing Center (www.bsc.es)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/*
  Wrappers that make possible to call thread (set|get)affinity from Python

  @author: srodrig1 (sergio.rodriguez at bsc.es)
*/
#include <Python.h>
#include <structmember.h>
#include <unistd.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <vector>

extern "C" {
#if PY_MAJOR_VERSION >= 3
    PyMODINIT_FUNC
    PyInit_thread_affinity(void);
#else
    void initthread_affinity(void);
#endif
}

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#define PyInt_AsLong PyLong_AsLong
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

static PyObject *
error_out(PyObject *m) {
    struct module_state *st = GETSTATE(m);
    PyErr_SetString(st->error, "Thread affinity extension module: Something bad happened");
    return NULL;
}

//////////////////////////////////////////////////////////////

// Computed in the init function of the module
cpu_set_t default_affinity;

/*
  Wrapper for sched_setaffinity.
  Arguments:
  - mask: a list of integers that denote the CPU identifiers (0-based) that we
          want to allow
  - pid: if zero, this will be transformed to the current pid
  Returns None
*/
static PyObject *pysched_setaffinity(PyObject *self, PyObject *args) {
    long long pid = 0ll;
    PyObject* cpu_list;
    if(!PyArg_ParseTuple(args, "O|l", &cpu_list, &pid)) {
        return NULL;
    }
    cpu_set_t to_assign;
    CPU_ZERO(&to_assign);
    int num_params = PyList_Size(cpu_list);
    for(int i = 0; i < num_params; ++i) {
        int cpu_id = PyInt_AsLong(PyList_GetItem(cpu_list, i));
        CPU_SET(cpu_id, &to_assign);
    }
    if(sched_setaffinity(pid, sizeof(cpu_set_t), &to_assign) < 0) {
        if(sched_setaffinity(pid, sizeof(cpu_set_t), &default_affinity) < 0) {
            PyErr_SetString(PyExc_RuntimeError, "Cannot set default affinity (!)");
        }
    }
    Py_RETURN_NONE;
}

static PyObject *_mask_to_python_list(cpu_set_t& set_cpus) {
    // Convert to Python List
    std::vector< int > ret_val;
    for(int i = 0; i < get_nprocs(); ++i) {
        if(CPU_ISSET(i, &set_cpus)) {
            ret_val.push_back(i);
        }
    }
    PyObject *ret = PyList_New(int(ret_val.size()));
    for(int i = 0; i < int(ret_val.size()); ++i) {
        PyList_SetItem(ret, i, Py_BuildValue("i", ret_val[i]));
    }
    return ret;
}

/*
  Wrapper for sched_getaffinity.
  Arguments:
  - pid (OPTIONAL): if zero or ommited, this will be transformed to the current pid
  Returns the list of allowed CPUs
*/
static PyObject  *pysched_getaffinity(PyObject *self, PyObject *args) {
    long long pid = 0ll;
    if(!PyArg_ParseTuple(args, "|l", &pid)) {
        return NULL;
    }
    if(pid == 0ll) {
        pid = getpid();
    }
    cpu_set_t set_cpus;
    if(sched_getaffinity(pid, sizeof(cpu_set_t), &set_cpus) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "Error during sched_getaffinity call!");
        Py_RETURN_NONE;
    }
    return _mask_to_python_list(set_cpus);
}

/*
  Return the default affinity computed when loading this module
*/
static PyObject *get_default_affinity(PyObject *self, PyObject *args) {
    if(!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    return _mask_to_python_list(default_affinity);
}

/*
  Wrapper for get_nprocs.
  No arguments.
  Returns the number of processors.
*/
static PyObject *get_num_cpus(PyObject *self, PyObject *args) {
    if(!PyArg_ParseTuple(args, "")) {
        return NULL;
    }
    long long ret = get_nprocs();
    return Py_BuildValue("l", ret);
}

//////////////////////////////////////////////////////////////

// setaffinity and getaffinity are here for legacy purposes, do not remove them,
// they are NOT a typo!
static PyMethodDef ThreadAffinityMethods[] = {
    { "error_out", (PyCFunction)error_out, METH_NOARGS, NULL},
    { "setaffinity", pysched_setaffinity, METH_VARARGS, "Args: (mask, [pid=0]) -> set the affinity for the thread with given pid to given mask. If pid equals zero, then the current thread's affinity will be changed." },
    { "getaffinity", pysched_getaffinity, METH_VARARGS, "Args: ([pid=0]) -> returns the affinity for the thread with given pid. If not specified, returns the affinity for the current thread."},
    { "set_affinity", pysched_setaffinity, METH_VARARGS, "Args: (mask, [pid=0]) -> set the affinity for the thread with given pid to given mask. If pid equals zero, then the current thread's affinity will be changed." },
    { "get_affinity", pysched_getaffinity, METH_VARARGS, "Args: ([pid=0]) -> returns the affinity for the thread with given pid. If not specified, returns the affinity for the current thread."},
    { "get_nprocs", get_num_cpus, METH_VARARGS, "No args. Return the number of available CPUs"},
    { "get_default_affinity", get_default_affinity, METH_VARARGS, "No args. Return the default affinity for this process"},
    { NULL, NULL } /* sentinel */
};


#if PY_MAJOR_VERSION >= 3
static int thread_affinity_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}
static int thread_affinity_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}
static struct PyModuleDef cModThAPy = {
    PyModuleDef_HEAD_INIT,
    "thread_affinity",             /* name of module */
    NULL,                          /* module documentation, may be NULL */
    sizeof(struct module_state),   /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    ThreadAffinityMethods,
    NULL,
    thread_affinity_traverse,
    thread_affinity_clear,
    NULL
};
#define INITERROR return NULL
PyMODINIT_FUNC
PyInit_thread_affinity(void)
#else
#define INITERROR return
void initthread_affinity(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *module = PyModule_Create(&cModThAPy);
#else
    PyObject *module = Py_InitModule("thread_affinity", ThreadAffinityMethods);
#endif

    if (module == NULL)
        INITERROR;
    struct module_state *st = GETSTATE(module);

    st->error = PyErr_NewException("thread_affinity.Error", NULL, NULL);
    if (st->error == NULL) {
        Py_DECREF(module);
        INITERROR;
    }
    // Default affinity = affinity when process initd
    sched_getaffinity(0, sizeof(cpu_set_t), &default_affinity);

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
