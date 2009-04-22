#include <pygobject.h>

void pygnash_register_classes (PyObject *d);

extern PyMethodDef pygnash_functions[];

DL_EXPORT(void)
initgnash(void)
{
    PyObject *m, *d;

    init_pygobject ();

    m = Py_InitModule ("gnash", pygnash_functions);
    d = PyModule_GetDict (m);

    pygnash_register_classes (d);

    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module gnash");
    }
}
