#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define GET_TYPE_BY_BITS(nbits) int i;for (i = 0; i < 9; i++){if (selectors[i].nbits == nbits){type = i;break;}}
FILE* fp;
static const uint32_t postdelim = 0xffffffff;
static const uint32_t termdelim = 0xfffffff0;
static const uint32_t getter[9] = { 
	0x00000001, 
	0x00000003, 
	0x00000007, 
	0x0000000f, 
	0x0000001f, 
	0x0000007f, 
	0x000001ff, 
	0x00003fff,
	0x0fffffff
};
static const struct {
	uint32_t nitems;
	uint32_t nbits;
	uint32_t nwaste;
} selectors[9] = {
		{ 28, 1, 0 },
		{ 14, 2, 0 },
		{ 9, 3, 1 },
		{ 7, 4, 0 },
		{ 5, 5, 3 },
		{ 4, 7, 0 },
		{ 3, 9, 1 },
		{ 2, 14, 0 },
		{ 1, 28, 0 },
};

size_t encode(uint32_t* num, uint32_t** data, size_t len){
	size_t idx = 0;
	size_t nbits = 0;
	size_t numc = 0;
	size_t curbits = 0;
	size_t type = 0;
	uint32_t curnum = 0;
	size_t datac = 0;
	//fprintf(fp,"encode\n");
	//fflush(fp);
	while (idx < len){
		curbits = 0;
		curnum = *(num + idx);
		while (curnum != 0){
			curnum >>= 1;
			curbits += 1;
		}
		int i;
		for (i = 0; i < 9; i++){
			if (selectors[i].nbits >= curbits){
				curbits = selectors[i].nbits;
				break;
			}
		}
		if (curbits > nbits){
			if (curbits*(numc + 1) <= 28){
				nbits = curbits;
				numc += 1;
				idx += 1;
			}
			else{
				nbits = 28 / numc;
				GET_TYPE_BY_BITS(nbits);
				*data[datac] = type << 28;
				for (; numc>0; numc--){
					curnum = *(num + idx - numc);
					*data[datac] |= curnum << (numc - 1)*nbits;
				}
				datac += 1;
				nbits = 0;
			}
		}
		else if (nbits*(numc + 1) > 28){
			GET_TYPE_BY_BITS(nbits);
			*data[datac] = type << 28;
			for (; numc>0; numc--){
				curnum = *(num + idx - numc);
				*data[datac] |= curnum << (numc - 1)*nbits;
			}
			datac += 1;
			nbits = 0;
		}
		else{
			numc += 1;
			idx += 1;
		}
	}
	if (numc == 0) return datac;
	nbits = 28 / numc;
	GET_TYPE_BY_BITS(nbits);
	*data[datac] = type << 28;
	for (; numc>0; numc--){
		curnum = *(num + idx - numc);
		*data[datac] |= curnum << (numc - 1)*nbits;
	}
	return datac+1;
}

size_t decode(uint32_t* data, uint32_t** num, size_t len){
	size_t selector = 0;
	size_t idx = 0;
	size_t numc = 0;
	uint32_t curdata = 0;
	uint32_t items = 0;
	uint32_t bits = 0;
	for (idx = 0; idx < len; idx++){
		curdata = *(data + idx);
		selector = (curdata >> 28)&0x0000000f;
		items = selectors[selector].nitems;
		bits = selectors[selector].nbits;
		*num[numc+items-1] = curdata&getter[selector];
		int i;
		for (i = 1; i < items; i++){
			curdata >>= bits;
			*num[numc+items-i-1] = curdata&getter[selector];
		}
		numc += items;
	}
	return numc;
}

uint32_t* split(char* str, int* len){
	int strl = strlen(str);
	char* s = str;
	char* t = (char*)PyMem_Malloc(sizeof(char)*strl);
	uint32_t* r = (uint32_t*)PyMem_Malloc(sizeof(uint32_t)*strl);
	int i = 0, j = 0;
	int c=0;
	//fprintf(fp,"split\n");
	//fflush(fp);
	while (*s != '\0'){
		if (*s == ' '){
			*(t+i) = '\0';
			//fprintf(fp,"len %d\n",i);
			//fflush(fp);
			*(r + j) = (uint32_t)strtoul(t,NULL,10);
			if(*(r + j)>>29!=0)return NULL;
			j+=1;
			i = 0;
			c=0;
		}
		else if (*s > 64 && *s < 91){
			*(t + i++) = *s/ 10 + 48;
			*(t + i++) = *s % 10 + 48;
			c+=1;
			if(c>3){
				*(t + i+1)='\0';
				return NULL;
			}
		}
		else if (*s > 47 && *s < 58){
			*(t + i++) = *s;
			c=0;
		}
		else{
			*(t + i+1)='\0';
			return NULL;
		}
		s += 1;
	}
	*(t + i) = '\0';
	*(r + j) = (uint32_t)strtoul(t, NULL, 10);
	if(*(r + j)>>29!=0)return NULL;
	j+=1;
	*len = j;
	PyMem_Free(t);
	return r;
}

char* ulcat(uint32_t** outdata, size_t len){
	char* outstr = (char*)PyMem_Malloc(sizeof(char) * 4 * len );
	char* cur4byte = 0;
	int i;
	for (i = 0; i < len; i++){
		memcpy(outstr+4*i, (char*)*(outdata + i), 4);
		/*cur4byte = (char*)*(outdata + i);
		*(outstr + 4*i) = *(cur4byte + 3); 
		*(outstr + 4*i + 1) = *(cur4byte + 2);
		*(outstr + 4*i + 2) = *(cur4byte + 1);
		*(outstr + 4*i + 3) = *(cur4byte);*/
	}
	return outstr;
}

uint32_t* outencode(char* str, int* codenum){
	int* len = (int*)PyMem_Malloc(sizeof(int));
	int i;
	//fprintf(fp,"outencode\n");
	//fflush(fp);
	uint32_t* innum = split(str, len);
	//fprintf(fp,"split return\n");
	//fflush(fp);
	if(innum==NULL){
		//fprintf(fp,"NULL!\n");
		PyMem_Free(len);
		return NULL;
	}
	//for (i = 0; i<*len; i++)	fprintf(log, "%u ", innum[i]);
	//fprintf(log, "\n");
	uint32_t** outdata = (uint32_t**)PyMem_Malloc((*len) * sizeof(uint32_t*));
	for (i = 0; i < *len; i++)
		*(outdata + i) = (uint32_t*)PyMem_Malloc(sizeof(uint32_t));
	size_t n32 = encode(innum, outdata, *len);
	//fprintf(fp,"finish encode\n");
	//fflush(fp);
	//for (i = 0; i<n32; i++)	fprintf(log, "%u ", *(outdata[i]));
	for (i = 0; i<n32; i++){
		innum[i] = *(outdata[i]);
		PyMem_Free(outdata[i]);
	}
	for(;i<*len;i++)PyMem_Free(outdata[i]);
	PyMem_Free(outdata);
	*codenum = n32;
	PyMem_Free(len);
	return innum;
}

int process(char** posts,int postlen,FILE* index,FILE* debug){
	int i=0;
	int bytecount=0;
	int* codenum=(int*)PyMem_Malloc(sizeof(int));
	uint32_t* codes;
	//fp=fopen("log1.txt", "w");
	for(i=0;i<postlen;i++){
		//fprintf(fp,"process %d\n",i);
		//fflush(fp);
		codes=outencode(posts[i],codenum);
		//fprintf(fp,"finish outencode\n");
		//fflush(fp);
		if(codes==NULL)continue;
		fwrite(codes,sizeof(uint32_t),*codenum,index);
		fwrite(&postdelim,sizeof(uint32_t),1,index);
		bytecount+=*codenum+1;
		PyMem_Free(codes);
	}
	fwrite(&termdelim,sizeof(uint32_t),1,index);
	bytecount+=1;
	PyMem_Free(codenum);
	//fprintf(fp,"finish process\n");
	//fflush(fp);
	//fclose(fp);
	return 4*bytecount;
}

/*int main(){
	char** a = (char**)malloc(sizeof(char*) * 3);
	int i = 0;
	a[0] = "33975 1 7009 P";
	a[1] = "244796 1 16957 P";
	a[2] = "244943 1 9397 P";
	int* nchar = (int*)malloc(sizeof(int));
	process(a, 3, 0);
	FILE* readf = fopen("index0", "rb");
	uint32_t curcode = 0;
	while (fread(&curcode, sizeof(uint32_t), 1, readf)){
		if (curcode == 0xffffffff)
			printf("\n");
		else
			printf("%x ", curcode);
	}
	fclose(readf);
	return 0;
}
*/
