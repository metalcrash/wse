#include "/usr/include/python2.7/Python.h"
extern int process(int termnum, char* indexls, char* appears);
static PyObject *parser_parser(PyObject *self, PyObject *args){
	char* indexls, *appears, *topK;
	int termnum;
	
	if(!PyArg_ParseTuple(args, "iss", &termnum, &indexls, &appears))
	{
		return NULL;
	}
	/*call the C function*/
	topK=process(termnum, indexls, appears);
	return Py_BuildValue("s",topK);
}

static PyMethodDef processmethods[]={
	{"process", processor_process, METH_VARARGS},
	{NULL, NULL}
};
/* Module initialization function*/
initprocessor(void){
	Py_InitModule("processor", processmethods);
}