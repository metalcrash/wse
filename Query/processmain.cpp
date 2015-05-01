#include "process.h"

#define PATH "/home/metalcrash/Indexer/final/"
#define INDEX "index"
#define DOC "doc"
#define LEXICON "lexicon"
#define META "meta"

struct tokendata** lexicon;
struct docdata** doclist;
int termnum = 0;
int* docnum = NULL;
uint32_t lexiconlen = 10000;
uint32_t totaldoc = 0;
uint32_t avg = 0;
uint32_t** termlists = NULL;
uint32_t* listslen = NULL;

char indexpath[] = PATH INDEX;
char docpath[] = PATH DOC;
char lexiconpath[] = PATH LEXICON;
char metapath[] = PATH META;

struct docnscore** daat();
uint32_t* sdecode(uint32_t data, size_t* len);

struct tokendata* addhashitem(char* token){
	uint32_t key = 0;
	char* c = token;
	while (*c != '\0'){
		key = *c + (key << 6) + (key << 16) - key;
		c+=1;
	}
	key = key % lexiconlen;
	struct tokendata* bucketp;
	struct tokendata* newbucket;
	if (*(lexicon + key) == NULL){
		newbucket = (struct tokendata*)malloc(sizeof(struct tokendata));
		*(lexicon + key) = newbucket;
	}
	else{
		bucketp = *(lexicon + key);
		while (bucketp->next != NULL){
			bucketp = bucketp->next;
		}
		newbucket = (struct tokendata*)malloc(sizeof(struct tokendata));
		bucketp->next = newbucket;
	}
	newbucket->token = token;
	return newbucket;
}

struct tokendata* findhashitem(char* token){
	uint32_t key = 0;
	int bit = 1;
	char* c = token;
	while (*c != '\0'){
		key = *c + (key << 6) + (key << 16) - key;
		c += 1;
	}
	key = key % lexiconlen;
	struct tokendata* bucket = *(lexicon + key);
	if (bucket == NULL)return NULL;
	while (bucket != NULL){
		if (strcmp(token, bucket->token) == 0){
			return bucket;
		}
		bucket = bucket->next;
	}
	return NULL;
}

/*struct tokendata** findallhashitem(char* token, int* num){
	int bsize = 10;
	struct tokendata** buckets = (tokendata**)malloc(sizeof(tokendata*)*bsize);
	uint32_t key = 0;
	int bit = 1;
	char* c = token;
	*num = 0;
	while (*c != '\0'){
		key = *c + (key << 6) + (key << 16) - key;
		c += 1;
	}
	key = key % lexiconlen;
	struct tokendata* bucket = *(lexicon + key);
	if (bucket == NULL)return NULL;
	while (bucket != NULL){
		if (strcmp(token, bucket->token) == 0){
			buckets[(*num)++] = bucket;
			if (*num == bsize){
				bsize *= 2;
				buckets=realloc(buckets, bsize);
			}
		}
		bucket = bucket->next;
	}
	return buckets;
}*/

void readmeta(){
	FILE* metaf = fopen(metapath, "r");
	char line[256];
	if ((fgets(line, sizeof(line), metaf)) != NULL) {
		avg=(uint32_t)strtol(line, (char **)NULL, 10);
	}
}

void readlexicon(){
	FILE* lexiconf = fopen(lexiconpath, "r");
	char line[256];
	int i,j;
	lexiconlen = 0;
	while ((fgets(line, sizeof(line), lexiconf)) != NULL) {
		//printf("%s", line);
		lexiconlen += 1;
	}
	lexicon = (struct tokendata**)malloc(sizeof(struct tokendata*) * lexiconlen);
	for (i = 0; i < lexiconlen; i++)lexicon[i] = NULL;
	char tokdata[4][32];
	char* token;
	char* cp;
	fseek(lexiconf, 0, SEEK_SET);
	while ((fgets(line, sizeof(line), lexiconf)) != NULL) {
		i = 0;
		j = 0;
		cp = line;
		token = (char*)malloc(sizeof(char)*30);
		*token = *cp;
		while (*cp != '\0'){
			if (*cp == ' '){
				*(token + j++) = '\0';
				cp += 1;
				break;
			}
			*(token + j++) = *cp;
			cp += 1;
		}
		j = 0;
		while (*cp != '\0'){
			if(*cp == ' '){
				i += 1;
				j = 0;
			}
			else if (*cp == '\n'){
				tokdata[i][j++] = '\0';
				break;
			}else{
				tokdata[i][j++] = *cp;
			}
			cp += 1;
		}
		struct tokendata* bucket = findhashitem(token);
		if (bucket != NULL)continue;
		bucket=addhashitem(token);
		bucket->filenum = (int)strtol(tokdata[0], (char **)NULL, 10);
		bucket->startbyte = (int)strtol(tokdata[1], (char **)NULL, 10);
		bucket->bytelen = (int)strtol(tokdata[2], (char **)NULL, 10);
		bucket->posnum = (int)strtol(tokdata[3], (char **)NULL, 10);
		bucket->next = NULL;
	}
}

void readdoc(){
	FILE* doc = fopen(docpath, "r");
	char line[512];
	size_t read;
	int i = 0;
	int docfilelen = 0;
	while (fgets(line, sizeof(line), doc) != NULL) {
		//printf("%s", line);
		docfilelen += 1;
	}
	totaldoc = docfilelen;
	doclist = (struct docdata**)malloc(sizeof(struct docdata*) * docfilelen);
	i = 0;
	char tokennum[32];
	char* url;
	char* cp;
	int space,urli,tokni;
	fseek(doc, 0, SEEK_SET);
	while (fgets(line, sizeof(line), doc) != NULL) {
		space = 0;
		urli=0;
		tokni=0;
		cp = line;
		url=(char*)malloc(sizeof(char)*256);
		while (*cp != '\0'){
			if (*cp == ' '){
				if(space==1) *(url + urli++)='\0';
				if(space==2) tokennum[tokni++]='\0';
				space += 1;
			}
			else if (*cp == '\n'){
				tokennum[tokni++]='\0';
			}
			else{
				if (space == 1)	*(url + urli++) = *cp;
				if (space == 2) tokennum[tokni++] = *cp;
			}
			cp += 1;
		}
		doclist[i] = (struct docdata*)malloc(sizeof(struct docdata));
		doclist[i]->url = url;
		doclist[i]->tokennum = (uint32_t)strtol(tokennum, (char **)NULL, 10);
		i++;
	}
	fclose(doc);
}
void loadindexlists(char** term, int termnum){
	termlists = (uint32_t**)malloc(sizeof(uint32_t*)*termnum);
	listslen = (uint32_t*)malloc(sizeof(uint32_t)*termnum);
	docnum = (int*)malloc(sizeof(int)*termnum);
	int match = 0;
	int i,j;
	int l;
	char fn[80];
	char numstr[8];
	FILE* curfile;
	for (i = 0; i < termnum; i++){
		struct tokendata* bucket = findhashitem(term[i]);
		if (bucket == NULL){
			termlists = NULL;
			return;
		}
		sprintf(numstr, "%d", bucket->filenum);
		strcpy(fn, indexpath);
		strcat(fn, numstr);
		printf("%s\n",fn);
		curfile = fopen(fn, "rb");
		fseek(curfile, bucket->startbyte, SEEK_SET);
		termlists[i] = (uint32_t*)malloc(sizeof(uint32_t)*(bucket->bytelen / 4));
		fread(termlists[i], sizeof(uint32_t), bucket->bytelen / 4, curfile);
		printf("%x\n",(uint32_t)termlists[i][]);
		listslen[i] = bucket->bytelen / 4;
		docnum[i] = bucket->posnum;
		fclose(curfile);
	}
}

int main(){
	clock_t begin, end, verybegin,veryend;
	verybegin = clock();
	int i, j;
	readdoc();
	readlexicon();
	readmeta();
	end = clock();
	printf("data initialized in %f", (double)(end - verybegin));
	int k = 0;
	for (i = 0; i < lexiconlen; i++){
		if (*(lexicon + i) != NULL)k += 1;
	}
	printf("%d\n", k);
	char** term = (char**)malloc(sizeof(char*) * 10);
	char curc;
	while (1){
		printf("input query terms, end with return\n");
		term[0] = (char*)malloc(sizeof(char) * 20);
		i = 0; j = 0;
		begin = clock();
		while (1){
			scanf("%c", &curc);
			if (curc == '\n'){
				*(term[i] + j) = '\0';
				i += 1;
				break;
			}
			if (curc == ' '){
				*(term[i] + j) = '\0';
				i += 1;
				term[i] = (char*)malloc(sizeof(char) * 20);
				j = 0;
			}
			else{
				if (curc >= 65 && curc <= 90)curc += 32;
				*(term[i] + j++) = curc;
			}
		}
		begin = clock();
		termnum = i;
		loadindexlists(term, termnum);
		struct docnscore** topk = daat();
		for (i = 0; i < dek; i++){
			printf("%u %f\n", topk[i]->id, topk[i]->score);
			printf("%s\n", doclist[topk[i]->id]->url);
		}
		printf("\n");
		veryend = clock();
		printf("%f\n", (double)(veryend - begin));
		for (i = 0; i < termnum; i++){
			free(term[i]);
			free(termlists[i]);
		}
		//free(termlists);
		//free(listslen);
		//free(docnum);
		printf("another query?(Y/N)\n");
		scanf("%c", &curc);
		if (curc == 'n' || curc == 'N')break;
		while ((curc = getchar()) != '\n' && curc != EOF);
	}
}
