#include "/usr/include/python2.7/Python.h"
#include <stdio.h>

extern int process(char** posts,int postlen,FILE* index,FILE* debug);

PyObject *simple9_encode(PyObject *self, PyObject *args){
	char** postings;
	char* token;
	int posnum=0,listnum=0,filenum=0,i=0,j=0;
	int bytecount=0;
	int thisbyte=0;
	PyObject* lists;
	PyObject* objlist;
	PyObject* objtoken;
	PyObject* objduplist;
	PyObject* objposting;
	
	FILE* debug=fopen("log.txt", "w");
	if(!PyArg_ParseTuple(args, "O!i", &PyList_Type,&lists,&filenum))
	{
		return NULL;
	}
	char* indexfn = (char*)PyMem_Malloc(sizeof(char) * 22);
	strcpy(indexfn, "final/index");
	char* fnum=(char*)PyMem_Malloc(sizeof(char)*8);
	sprintf(fnum, "%d", filenum);
	strcat(indexfn,fnum);
	FILE* index=fopen( indexfn, "wb");
	FILE* lexicon=fopen( "final/lexicon", "a");

	listnum=PyList_Size(lists);
	fprintf(debug,"%i\n",listnum);
	if(listnum<=0)return NULL;
	//fflush(debug);
	for(i=0;i<listnum;i++){
		objlist=PyList_GetItem(lists,i);
		objtoken=PyList_GetItem(objlist,0);
		token=PyString_AsString(objtoken);
		objduplist=PyList_GetItem(objlist,1);
		posnum=PyList_Size(objduplist);
		postings=(char**)PyMem_Malloc(sizeof(char*)*posnum);
		fprintf(debug,"%s ",token);
		fprintf(debug,"%i ",posnum);
		if(posnum<=0)continue;
		for(j=0;j<posnum;j++){
			objposting=PyList_GetItem(objduplist,j);
			postings[j]=PyString_AsString(objposting);
			fprintf(debug,"|%s",postings[j]);
			fflush(debug);
		}
		fprintf(debug,"%i is processing ",i);
		fflush(debug);
		thisbyte=process(postings,posnum,index,debug);
		fprintf(debug,"%i is processed\n",i);
		fflush(debug);
		fprintf(lexicon,"%s %i %i %i %i\n",token,filenum,bytecount,thisbyte,posnum);
		fflush(lexicon);
		bytecount+=thisbyte;

		//for(j=0;j<posnum;j++)PyMem_Free(postings[j]);
		PyMem_Free(postings);
	}
	fclose(index);
	//fprintf(debug,"haha\n");
	//fclose(debug);
	/*call the C function*/
	//process(record,poslen,filenum);
	PyMem_Free(indexfn);
	PyMem_Free(fnum);
	return Py_BuildValue("i",1);
}
static PyMethodDef encodemethods[]={
	{"encode", simple9_encode, METH_VARARGS},
	{NULL, NULL}
};
/* Module initialization function*/
initsimple9(void){
	Py_InitModule("simple9", encodemethods);
}
