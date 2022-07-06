#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BYTES_PER_WORD 4

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void RLRU(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB);
void WLRU(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB);
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                       */   
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */ 
/* 							       */
/* Cache Design						       */
/*  							       */
/* 	    cache[set][assoc][word per block]		       */
/*      						       */
/*      						       */
/*       ----------------------------------------	       */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*      						       */
/*                                                             */
/***************************************************************/
void xdump(int set, int way, uint32_t** cache)
{
	int i,j,k = 0;

	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			printf("0x%08x  ", cache[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

//return 1 --> True , 0 --> False
int Read_hit(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** LRU_order){
	int mask = ~((1<<block_offset)-1);
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int hit = 0;
	for(int i = 0; i<way; i++){
		if(cache[set][i]==(data_addr&mask)){
			LRU_order[set][i] = 0;  	//touch LRU =0
			hit = 1;
		}
		else if(cache[set][i]!=0x0){			//LRU
			LRU_order[set][i]+=1;		//not touch LRU++
		}
	}
	if(hit==1){
		return 1;
	}
	else{
		return 0;
	}
}


int Write_hit(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, int** dirty_addr, uint32_t** LRU_order, uint32_t** dirty_cache){
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int mask = ~((1<<block_offset)-1);
	int hit = 0;
	for(int i = 0; i<way; i++){
		if(cache[set][i]==(data_addr & mask)){
			dirty_cache[set][i] = 1;
			LRU_order[set][i] = 0;  	//touch LRU =0
			hit = 1;
		}
		else if(cache[set][i]!=0x0){			//LRU
			LRU_order[set][i]+=1;		//not touch LRU++
		}
	}
	if(hit==1){
		return 1;
	}
	else{
		return 0;
	}
}

int Read_fill(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB){
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int mask = ~((1<<block_offset)-1);
	for(int i = 0; i<way; i++){
		if(cache[set][i]==0){
			dirty_cache[set][i]=0;
			LRU_order[set][i] = 0;  	//touch LRU = 0
			cache[set][i]=(data_addr & mask);
			return 1;
		}
	}
	RLRU(cache, data_addr, way, index_bit, block_offset, dirty_cache, LRU_order, WB);
	return 0;
}

int Write_fill(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB){
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int mask = ~((1<<block_offset)-1);
	for(int i = 0; i<way; i++){
		if(cache[set][i]==0){
			dirty_cache[set][i] = 1;
			LRU_order[set][i] = 0;  	//touch LRU = 0
			cache[set][i]=(data_addr & mask);
			return 1;
		}
	}
	WLRU(cache, data_addr, way, index_bit, block_offset, dirty_cache, LRU_order, WB);
	return 0;
}
void RLRU(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB){
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int mask = ~((1<<block_offset)-1);
	int max_LRU=0;
	int replace_index = 0;
	for(int i = 0; i<way; i++){
		if((cache[set][i]!=0) && (LRU_order[set][i] > max_LRU)){
			max_LRU = LRU_order[set][i];
			replace_index = i;
		}
	}
	//write back
	if(dirty_cache[set][replace_index]!=0){
		*WB += 1;
	}
	cache[set][replace_index] = (data_addr & mask);
	dirty_cache[set][replace_index] = 0;
	LRU_order[set][replace_index] =0;
}

void WLRU(uint32_t** cache, uint32_t data_addr, int way, int index_bit, int block_offset, uint32_t** dirty_cache, uint32_t** LRU_order, int* WB){
	int set = (int)((data_addr>>block_offset)%(1<<index_bit));
	int mask = ~((1<<block_offset)-1);
	int max_LRU=0;
	int replace_index = 0;
	for(int i = 0; i<way; i++){
		if((cache[set][i]!=0) && (LRU_order[set][i] > max_LRU)){
			max_LRU = LRU_order[set][i];
			replace_index = i;
		}
	}
	//write back
	if(dirty_cache[set][replace_index]!=0){
		*WB += 1;
	}
	cache[set][replace_index] = (data_addr & mask);
	dirty_cache[set][replace_index] = 1;
	LRU_order[set][replace_index] =0;
}

int main(int argc, char *argv[]) {                              

	uint32_t** cache;
	uint32_t** dirty_cache;
	uint32_t** LRU_order;
	int i, j, k;	
	int capacity = 256;
	int way = 4;
	int blocksize = 8;
	int set = capacity/way/blocksize;
	int words = blocksize / BYTES_PER_WORD;	

// Cache State
	int total_reads=0;
	int total_writes=0;
	int write_backs=0;
	int reads_hits=0;
	int write_hits=0;
	int reads_misses=0;
	int write_misses=0;
//INDEX_bit Block_offset (USE to find cache position)
	int index_bit=0;
	int block_offset =0;

//1번 FOR getting information about Capacity, way, blocksize. from argv[2].
	char* CABinfo = argv[2];
	char colon[2] = ":";
	char *result;
	char information[3][6];

//2번 FOR manipulate to get address to access.
	char ReadorWrite[2];	// get R or W
	char space[2];
	int hex0; char removeX; //remove 0 and x from addr format
	char address[10];		//store ADDR to access.

	char* readop ="R";
	char* writeop = "W";

//READ

//WRITE
	int* dirty_addr[2];
	int dirty_set=0;
	int dirty_way=0;
	dirty_addr[0] = &dirty_set;
	dirty_addr[1] = &dirty_way;


	if(argc != 5){
		printf("Wrong\n"); //Example) ./runfile /sample_input/example1.s
		exit(0);
	}
	else
	{
//For input file read (sample_input/example)
		char *file=(char *)malloc(strlen(argv[4])+3);
		strncpy(file,argv[4],strlen(argv[4]));
//scanf --> read from file.
		FILE *fp = freopen(file, "r",stdin);
		if(fp==0){
			printf("File open Error!\n");
			exit(1);
		}
// 1번 argv[2] 를 가지고 CAPACITY, WAY, BLOCKSIZE 파라미터 세팅 
		int i = 0;
		result = strtok(CABinfo, colon);
		while (result != NULL){
			strcpy(information[i++], result);
			result = strtok(NULL, colon);
		}
			capacity = atoi(information[0]);
			way = atoi(information[1]);
			blocksize = atoi(information[2]);
			set = capacity/way/blocksize;
			//printf("capacity = %d\n", capacity); printf("way = %d\n", way); printf("blocksize = %d\n", blocksize);
//INDEX bit, BLOCK OFFSET bit
			int temp =0;
			temp =set;
			while(temp != 1){
				temp = temp/2;
				index_bit ++;
			}
			temp = blocksize;
			while(temp != 1){
				temp = temp/2;
				block_offset ++;
			}
			//printf("index= %dbit, block =%dbit\n",index_bit, block_offset);
//project4
		// cache memory allocate
		cache = (uint32_t**) malloc (sizeof(uint32_t*) * set);
		for(i = 0; i < set; i++) {
			cache[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
		}
		dirty_cache = (uint32_t**) malloc (sizeof(uint32_t*) * set);
		for(i = 0; i < set; i++) {
			dirty_cache[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
		}
		LRU_order = (uint32_t**) malloc (sizeof(uint32_t*) * set);
		for(i = 0; i < set; i++) {
			LRU_order[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
		}

		for(i = 0; i < set; i++) {
			for(j = 0; j < way; j ++) 
				cache[i][j] = 0x0;
				dirty_cache[i][j] = 0x0;
				LRU_order[i][j] = 0x0;
		}

//2번 	1. 주소를 읽고 
//		2. 그 주소에 해당하는 block이 있는지 확인
//		3. 있으면 hit 없으면 채워 넣는다(LRU, write back).

//		R 0x4cb26ec
		scanf("%c",ReadorWrite);
		while(!feof(fp)){
			scanf("%d",&hex0); scanf("%c",&removeX); 	//REMOVE "0x"
			scanf("%s",address);						//address 주소 string으로 나타냄			
			uint32_t data_addr = (uint32_t)strtol(address, NULL, 16);	//data_addr 주소 숫자로 나타냄
			//printf("%s 0x%08x\n", ReadorWrite, data_addr); 		
			
			if(strcmp(ReadorWrite,readop)==0){
				total_reads+=1;					//cache state
				if(Read_hit(cache, data_addr , way, index_bit, block_offset, LRU_order)==1){ //hit
				reads_hits += 1;				//cache state
				}
				else{ //miss
				reads_misses += 1;				//cache state
				Read_fill(cache, data_addr , way, index_bit, block_offset, dirty_cache, LRU_order, &write_backs);
				}
			}
			else if(strcmp(ReadorWrite,writeop)==0){
				total_writes+=1;				//cache state
				if(Write_hit(cache, data_addr, way, index_bit, block_offset, dirty_addr, LRU_order, dirty_cache)==1){ //hit
				write_hits += 1;				//cache state
				}
				else{ //miss
				write_misses += 1;				//cache state
				Write_fill(cache, data_addr , way, index_bit, block_offset, dirty_cache, LRU_order, &write_backs);
				}
			}
			scanf("%c",space);
			scanf("%c",ReadorWrite);
		}






		// test example
    	cdump(capacity, way, blocksize);
    	sdump(total_reads, total_writes, write_backs, reads_hits, write_hits, reads_misses, write_misses); 
    	xdump(set, way, cache);

    	return 0;
	}
}
