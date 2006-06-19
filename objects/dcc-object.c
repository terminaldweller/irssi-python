#include <Python.h>
#include "pyirssi_irc.h"
#include "pymodule.h"
#include "dcc-object.h"
#include "factory.h"
#include "pycore.h"

/* monitor "dcc destroyed signal" */
static void dcc_cleanup(DCC_REC *dcc)
{
    PyDcc *pydcc = signal_get_user_data();

    if (dcc == pydcc->data)
    {
        pydcc->data = NULL;
        pydcc->cleanup_installed = 0;
        signal_remove_data("dcc destroyed", dcc_cleanup, pydcc);
    }
}

static void PyDcc_dealloc(PyDcc *self)
{
    if (self->cleanup_installed)
        signal_remove_data("dcc destroyed", dcc_cleanup, self);

    Py_XDECREF(self->server);
    Py_XDECREF(self->chat);

    self->ob_type->tp_free((PyObject*)self);
}

/* Getters */
PyDoc_STRVAR(PyDcc_orig_type_doc,
    "Original DCC type that was sent to us - same as type except GET and SEND are swapped"
);
static PyObject *PyDcc_orig_type_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;
    const char *type;
    
    RET_NULL_IF_INVALID(data);

    type = module_find_id_str("DCC", data->orig_type);
    RET_AS_STRING_OR_NONE(type);
}

PyDoc_STRVAR(PyDcc_created_doc,
    "Time stamp when the DCC record was created"
);
static PyObject *PyDcc_created_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    return PyLong_FromUnsignedLong(data->created);
}

PyDoc_STRVAR(PyDcc_server_doc,
    "Server record where the DCC was initiated."
);
static PyObject *PyDcc_server_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_OBJ_OR_NONE(self->server);
}

PyDoc_STRVAR(PyDcc_servertag_doc,
    "Tag of the server where the DCC was initiated."
);
static PyObject *PyDcc_servertag_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->servertag);
}

PyDoc_STRVAR(PyDcc_mynick_doc,
    "Our nick to use in DCC chat."
);
static PyObject *PyDcc_mynick_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->mynick);
}

PyDoc_STRVAR(PyDcc_nick_doc,
    "Other side's nick name."
);
static PyObject *PyDcc_nick_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->nick);
}

PyDoc_STRVAR(PyDcc_chat_doc,
    "Dcc chat record if the request came through DCC chat"
);
static PyObject *PyDcc_chat_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_OBJ_OR_NONE(self->chat);
}

PyDoc_STRVAR(PyDcc_target_doc,
    "Who the request was sent to - your nick, channel or empty if you sent the request"
);
static PyObject *PyDcc_target_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->target);
}

PyDoc_STRVAR(PyDcc_arg_doc,
    "Given argument .. file name usually"
);
static PyObject *PyDcc_arg_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->arg);
}

PyDoc_STRVAR(PyDcc_addr_doc,
    "Other side's IP address."
);
static PyObject *PyDcc_addr_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    RET_AS_STRING_OR_NONE(data->addrstr);
}

PyDoc_STRVAR(PyDcc_port_doc,
    "Port we're connecting in."
);
static PyObject *PyDcc_port_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    return PyInt_FromLong(data->port);
}

PyDoc_STRVAR(PyDcc_starttime_doc,
    "Unix time stamp when the DCC transfer was started"
);
static PyObject *PyDcc_starttime_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    return PyLong_FromUnsignedLong(data->starttime);
}

PyDoc_STRVAR(PyDcc_transfd_doc,
    "Bytes transferred"
);
static PyObject *PyDcc_transfd_get(PyDcc *self, void *closure)
{
    DCC_REC *data = self->data;

    RET_NULL_IF_INVALID(data);
    return PyLong_FromUnsignedLong(data->transfd);
}

/* specialized getters/setters */
static PyGetSetDef PyDcc_getseters[] = {
    {"orig_type", (getter)PyDcc_orig_type_get, NULL,
        PyDcc_orig_type_doc, NULL},
    {"created", (getter)PyDcc_created_get, NULL,
        PyDcc_created_doc, NULL},
    {"server", (getter)PyDcc_server_get, NULL,
        PyDcc_server_doc, NULL},
    {"servertag", (getter)PyDcc_servertag_get, NULL,
        PyDcc_servertag_doc, NULL},
    {"mynick", (getter)PyDcc_mynick_get, NULL,
        PyDcc_mynick_doc, NULL},
    {"nick", (getter)PyDcc_nick_get, NULL,
        PyDcc_nick_doc, NULL},
    {"chat", (getter)PyDcc_chat_get, NULL,
        PyDcc_chat_doc, NULL},
    {"target", (getter)PyDcc_target_get, NULL,
        PyDcc_target_doc, NULL},
    {"arg", (getter)PyDcc_arg_get, NULL,
        PyDcc_arg_doc, NULL},
    {"addr", (getter)PyDcc_addr_get, NULL,
        PyDcc_addr_doc, NULL},
    {"port", (getter)PyDcc_port_get, NULL,
        PyDcc_port_doc, NULL},
    {"starttime", (getter)PyDcc_starttime_get, NULL,
        PyDcc_starttime_doc, NULL},
    {"transfd", (getter)PyDcc_transfd_get, NULL,
        PyDcc_transfd_doc, NULL},
    {NULL}
};

/* Methods */
PyDoc_STRVAR(PyDcc_destroy_doc,
    "Destroy DCC connection"
);
static PyObject *PyDcc_destroy(PyDcc *self, PyObject *args)
{
    RET_NULL_IF_INVALID(self->data);

    dcc_destroy(DCC(self->data));
    
    Py_RETURN_NONE;
}

/* Methods for object */
static PyMethodDef PyDcc_methods[] = {
    {"destroy", (PyCFunction)PyDcc_destroy, METH_NOARGS,
        PyDcc_destroy_doc},
    {NULL}  /* Sentinel */
};

PyTypeObject PyDccType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "Dcc",            /*tp_name*/
    sizeof(PyDcc),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyDcc_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyDcc objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyDcc_methods,             /* tp_methods */
    0,                      /* tp_members */
    PyDcc_getseters,        /* tp_getset */
    &PyIrssiBaseType,          /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};


/* Dcc factory function */
PyObject *pydcc_sub_new(void *dcc, const char *name, PyTypeObject *subclass)
{
    PyObject *chat = NULL, *server = NULL;
    PyDcc *pydcc;
    DCC_REC *rec = dcc;

    server = py_irssi_chat_new(rec->server, 1);
    if (!server)
        return NULL;

    //XXX: do dcc chat
    
    pydcc = py_instp(PyDcc, subclass);
    if (!pydcc)
    {
        Py_XDECREF(server);
        Py_XDECREF(chat);
        return NULL;
    }

    pydcc->data = dcc;
    pydcc->server = server;
    pydcc->base_name = name;

    pydcc->cleanup_installed = 1;
    signal_add_last_data("dcc destroyed", dcc_cleanup, pydcc);
    
    return (PyObject *)pydcc;
}

PyObject *pydcc_new(void *dcc)
{
    static const char *name = "DCC";
    return pydcc_sub_new(dcc, name, &PyDccType);
}

int dcc_object_init(void) 
{
    g_return_val_if_fail(py_module != NULL, 0);

    if (PyType_Ready(&PyDccType) < 0)
        return 0;
    
    Py_INCREF(&PyDccType);
    PyModule_AddObject(py_module, "Dcc", (PyObject *)&PyDccType);

    return 1;
}
