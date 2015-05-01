#include "process.h"
#include <assert.h>

#define GET_TYPE_BY_BITS(nbits) int i;for (i = 0; i < 9; i++){if (selectors[i].nbits == nbits){type = i;break;}}

int boardsize;
int* map;
int* cursor;
uint32_t* doccursor;
uint32_t globdoc;
uint32_t* freq;
struct docnscore** docboard;

static const double k1 = 1.2;
static const double b = 0.5;

uint32_t* sdecode(uint32_t data, size_t* len){
	size_t selector = 0;
	uint32_t items = 0;
	uint32_t bits = 0;
	printf("s0\n");
	uint32_t* nums = (uint32_t*)malloc(sizeof(uint32_t) * 14);
	printf("s1\n");
	selector = (data >> 28) & 0x0000000f;
	items = selectors[selector].nitems;
	bits = selectors[selector].nbits;
	nums[items - 1] = data&getter[selector];
	int i;
	printf("s2\n");
	for (i = 1; i < items; i++){
		data >>= bits;
		nums[items - i - 1] = data&getter[selector];
	}
	*len = items;
	return nums;
}

uint32_t* getposting(uint32_t* list, int* cursor, int* num){
	int datasize = 20;
	printf("0\n");
	uint32_t* data = (uint32_t*)malloc(sizeof(uint32_t)*20);
	uint32_t* newdata;
	uint32_t* curdata;
	size_t curlen;
	int i=*cursor;
	int j,k;
	int posn = 0;
	printf("1\n");
	if (list[i] == 0xfffffff0 || list[i] == 0xffffffff)return NULL;
	printf("1.5\n");
	curdata = sdecode(list[i++], &curlen);
	printf("2\n");	
	if (curlen > 1){
		data[0] = curdata[0];
		data[1] = curdata[1];
		posn = data[1];
		for (j = 2; j < curlen; j++){
			data[j] = curdata[j];
		}
		free(curdata);
	}
	else{
		data[0] = curdata[0];
		if (list[i] == 0xfffffff0 || list[i] == 0xffffffff){
			*cursor = i;
			return NULL;//error
		}
		free(curdata);
		curdata = sdecode(list[i++], &curlen);
		data[1] = curdata[0];
		for (k = 1,j=1; k < curlen; k++, j++){
			data[j] = curdata[k];
		}
		free(curdata);
	}
	printf("3\n");
	while (1){
		if (list[i] == 0xffffffff){
			//assert(2*(posn+1)==j);
			*cursor = i;
			return data;
		}
		curdata = sdecode(list[i++], &curlen);
		if (j + curlen >= datasize){
			printf("4\n");
			datasize += curlen+20;
			newdata = (uint32_t*)realloc(data, sizeof(uint32_t) * datasize);
			if(newdata!=NULL)data=newdata;
		}
		printf("5\n");
		for (k = 0; k < curlen; k++, j++){
			data[j] = curdata[k];
		}
		free(curdata);
	}
}

int moving(uint32_t* list, uint32_t len, int order){
	int i = cursor[map[order]];
	uint32_t* curposting;
	int num;
	uint32_t prevdoc = doccursor[map[order]], curdoc = 0;
	for (; i < len; i++){
		cursor[map[order]] = i;
		curposting = getposting(list, &i, &num);
		if (curposting == NULL){
			continue;
		}
		curdoc = curposting[0] + prevdoc;
		prevdoc = curdoc;
		freq[order] = curposting[1];
		free(curposting);
		if (order == 2){
			order = 2;
		}
		if (curdoc > globdoc)return 0;
		doccursor[map[order]] = prevdoc;
		if (curdoc == globdoc){
			cursor[map[order]] += 1;
			if (order + 1 < termnum)
				return moving(termlists[map[order + 1]], listslen[map[order + 1]], order + 1);
			else
				return 1;
		}
	}
	return -1;//shouldn't happen
}

double bm25(){
	int i;
	double k;
	double score = 0;
	double doclen = (double)doclist[globdoc - 1]->tokennum;
	k = k1*((1 - b) + b*doclen / (double)avg);
	for (i = 0; i < termnum; i++){
		score += log((((double)totaldoc - (double)docnum[i] + 0.5) / ((double)docnum[i] + 0.5))*((k1 + 1)*(double)freq[i] / (k + (double)freq[i])));
	}
	return score;
}

struct docnscore** topk(int boardsize){
	int i,j;
	struct docnscore** theks = (struct docnscore**)malloc(sizeof(struct docnscore*) * dek);
	struct docnscore* tmp;
	int len = 1;
	for (i = 0; i < dek; i++)theks[i] = (struct docnscore*)malloc(sizeof(struct docnscore*));
	theks[0]->id = docboard[0]->id;
	theks[0]->score = docboard[0]->score;
	for (i = 1; i < boardsize; i++){
		if (docboard[i]->score > theks[len - 1]->score){
			if (len!=dek)len += 1;
			theks[len - 1]->id = docboard[i]->id;
			theks[len - 1]->score = docboard[i]->score;
			for (j = len-1; j > 0; j--){
				if (theks[j]->score > theks[j - 1]->score){
					tmp = theks[j - 1];
					theks[j - 1] = theks[j];
					theks[j] = tmp;
				}
			}
		}
	}
	return theks;
}

struct docnscore** daat(){
	freq = (uint32_t*)malloc(sizeof(uint32_t)*termnum);
	map = (int*)malloc(termnum * sizeof(int));
	cursor = (int*)malloc(sizeof(int)*termnum);
	doccursor = (uint32_t*)malloc(sizeof(uint32_t)*termnum);
	int i, j;
	int boardsize = 50;
	int boardi = 0;
	docboard = (struct docnscore**)malloc(sizeof(struct docnscore*) * boardsize);
	struct docnscore** newdocboard;
	//sort by document number contained in ascending order
	int mindn = 0xffff;
	int idx=0;
	int** tmp = (int**)malloc(termnum * sizeof(int*));
	int* tmpswap;
	for (i = 0; i < termnum; i++){
		tmp[i] = (int*)malloc(2 * sizeof(int));
		tmp[i][0] = docnum[i];
		tmp[i][1] = i;
	}
	for (i = 0; i < termnum-1; i++){
		mindn = tmp[i][0];
		idx = i;
		for (j = i+1; j < termnum; j++){
			if (tmp[j][0] < mindn){
				mindn = tmp[j][0];
				idx = j;
			}
		}
		tmpswap = tmp[idx];
		tmp[idx] = tmp[i];
		tmp[i] = tmpswap;
	}
	for (i = 0; i < termnum; i++){
		map[i] = tmp[i][1];
		free(tmp[i]);
	}
	free(tmp);
	globdoc = 0xffffffff;
	uint32_t* curposting;
	int num;
	int state;
	uint32_t len = listslen[map[0]];
	uint32_t* list = termlists[map[0]];
	uint32_t prevdoc = 0, curdoc = 0;
	for (i = 0; i < termnum; i++){ cursor[i] = 0; doccursor[i] = 0; }
	for (; cursor[map[0]] < len; cursor[map[0]]++){
		printf("%x,%d \n",(uint32_t)list[0],cursor[map[0]]);
		printf("%d,%d\n",*(list+cursor[map[0]]),cursor[map[0]]);
		curposting = getposting(list, &cursor[map[0]], &num);
		printf("%d,%d\n",*(list+cursor[map[0]]),cursor[map[0]]);
		if (curposting == NULL){
			printf("ah\n");
			break;
		}
		curdoc = curposting[0] + prevdoc;
		prevdoc = curdoc;
		freq[0] = curposting[1];
		globdoc = curdoc;
		if (termnum == 1)state = 1;
		else state = moving(termlists[map[1]], listslen[map[1]], 1);
		if (state == 1){
			struct docnscore* newdoc = (struct docnscore*)malloc(sizeof(struct docnscore));
			newdoc->id = globdoc;
			newdoc->score = bm25();
			//if current docboard is full, extend it
			if (boardi >= boardsize){
				boardsize += 50;
				newdocboard = (struct docnscore**)malloc(sizeof(struct docnscore*) * boardsize);
				memcpy(newdocboard, docboard, sizeof(struct docnscore*) * (boardsize - 50));
				docboard = newdocboard;
			}
			docboard[boardi++] = newdoc;
		}
		else if (state == -1){
			break;
		}
	}
	struct docnscore** theks = topk(boardi);
	for (i = 0; i < boardi; i++)free(docboard[i]);
	free(docboard);
	free(freq);
	free(map);
	free(cursor);
	return theks;
}
