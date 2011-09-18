#include <Python.h>
#include <iostream>
#include <wendy/Project.hpp>

extern "C" {

static PyObject *pywendy_plop(PyObject *self, PyObject *args)
{
	std::cout << "Plooop !!" << std::endl;
	return Py_BuildValue("i", 0);
}

PyMethodDef methods[] = {
	{"plop", pywendy_plop, METH_VARARGS, "Plop the world!"},
	
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initpywendy()
{
	Py_InitModule("pywendy", methods);
}

} // extern "C"

