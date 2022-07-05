#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef struct key_value{
	char key[8];
	int	value;
} key_value;

typedef struct inst_type{
	char type; //if 3, no inst
	int inst;
	int addr;
	char label[10];
} inst_type;

void print_inst(int inst);
_Bool Is_tag(char* op);
void make_I_code(inst_type* new,int scan_num, char last);
void make_R_code(inst_type* new,int scan_num, char last);
int sixtoten(char* copy);



int main(int argc, char* argv[]){
	char data[6] =".data"; char word[6]=".word"; char main[6] ="main:"; char text[6] =".text";
	char j[2]="j"; char jal[4]="jal"; char andi[5]="andi"; char beq[4]="beq"; char bne[4]="bne"; char lui[4]="lui";
	char ori[4]="ori"; char lw[3]="lw"; char sw[3]="sw"; char sltiu[6]="sltiu"; char addiu[6]="addiu"; char and[4]="and";
	char jr[3]="jr"; char nor[4]="nor"; char or[3]="or"; char sll[4]="sll"; char srl[4]="srl"; char sltu[5]="sltu";
	char addu[5]="addu"; char subu[5]="subu"; char la[3]="la";

	char data_word[100]; 	// read one-by-one
	char* op =data_word;
	int data_v;				//date value read.
	int d_label_num=0;		//# of label
	int data_table[100];
		

	char Label[100];					//record label
	int t_label_num=0;				//# of label

	int data_length = 0;			//**length of data
	int text_length = 0;			//**length of text

	inst_type *inst_array[100] ;			// array of struct inst_type
    key_value text_label[10] = {}; 			//record label from text.
	key_value data_label[10] = {};			//record data from data.	

	if(argc != 2){
		printf("Usage: ./runfile <assembly file>\n"); //Example) ./runfile /sample_input/example1.s
		printf("Example) ./runfile ./sample_input/example1.s\n");
		exit(0);
	}
	else
	{

		// To help you handle the file IO, the deafult code is provided.
		// If we use freopen, we don't need to use fscanf, fprint,..etc. 
		// You can just use scanf or printf function 
		// ** You don't need to modify this part **
		// If you are not famailiar with freopen,  you can see the following reference
		// http://www.cplusplus.com/reference/cstdio/freopen/

		//For input file read (sample_input/example*.s)

		char *file=(char *)malloc(strlen(argv[1])+3);
		strncpy(file,argv[1],strlen(argv[1]));

		if(freopen(file, "r",stdin)==0){
			printf("File open Error!\n");
			exit(1);
		}

		//From now on, if you want to read string from input file, you can just use scanf function.


		// For output file write 
		// You can see your code's output in the sample_input/example#.o 
		// So you can check what is the difference between your output and the answer directly if you see that file
		// make test command will compare your output with the answer
		file[strlen(file)-1] ='o';
		freopen(file,"w",stdout);

		//If you use printf from now on, the result will be written to the output file.
		

		//1. detect .data
		scanf("%s",data_word);
	
		while(1){
			scanf("%s",data_word);
			if(strcmp(data_word,text)==0) break;

			if(strcmp(data_word,word)){ //word or Label
				//store in table //case of Label
				strcpy(data_label[d_label_num].key , data_word);
				data_label[d_label_num].value = data_length<<2;
				d_label_num++;
				scanf("%s",data_word); //detect word.
			}

			scanf("%d",&data_v); //read value int
			if(data_v==0){
				scanf("%s",data_word);
				char* change = data_word+1;
				data_v = sixtoten(change);
			}

			data_table[data_length]=data_v;
			data_length++;
		}

		//after detect .text, detect main
		scanf("%s",data_word); 
		
		
		while(scanf("%s",data_word)!=-1){ //read operation 1 while, 1 instruction
			char colon=0;
			int cf=0;
			while(data_word[cf]!=0){
				colon = data_word[cf];
				cf+=1;
			}
			//prepare for detect label(above)
			char temp_s[10];
			if(colon != ':'){ //case of instruction
					// J 2
					if(!strcmp(data_word,j)){
						//scanf("%s",temp_s);
						inst_type* j = malloc(sizeof(inst_type));
						scanf("%s",Label);
						j->type = 2;
						strcpy(j->label,Label);
						j->inst = 2<<26;
						inst_array[text_length]=j;
						text_length++;
						
					}
					
					else if(!strcmp(data_word,jal)){
						//scanf("%s",temp_s);
						inst_type* jal = malloc(sizeof(inst_type));
						scanf("%s",Label);
						jal->type = 2;
						strcpy(jal->label,Label);
						jal->inst = 3<<26;
						inst_array[text_length]=jal;
						text_length++;
						
					}

					// I 4
					else if(!strcmp(data_word,andi)){
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						inst_type* andi = malloc(sizeof(inst_type));
						andi->inst = 12<<26;
						make_I_code(andi,3,'N');
						inst_array[text_length]=andi;
						text_length++;
						
					}
					else if(!strcmp(data_word,beq)){
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* beq = malloc(sizeof(inst_type));
						beq->inst = 4<<26;
						make_I_code(beq,3,'L');
						inst_array[text_length]=beq;
						text_length++;
						
					}
					else if(!strcmp(data_word,bne)){
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* bne = malloc(sizeof(inst_type));
						bne->inst = 5<<26;
						make_I_code(bne,3,'L');
						inst_array[text_length]=bne;
						text_length++;
						
					}
					else if(!strcmp(data_word,lui)){
						//scanf("%s",temp_s);
						//scanf("%s",temp_s);
						
						inst_type* lui = malloc(sizeof(inst_type));
						lui->inst = 15<<26;
						make_I_code(lui,2,'N');
						inst_array[text_length]=lui;
						text_length++;
						
					}
					else if(!strcmp(data_word,ori)){
						//scanf("%s",temp_s);
						//scanf("%s",temp_s);
						//scanf("%s",temp_s);
						
						inst_type* ori = malloc(sizeof(inst_type));
						ori->inst = 13<<26;
						make_I_code(ori,3,'N');
						inst_array[text_length]=ori;
						text_length++;
						
					}
					else if(!strcmp(data_word,lw)){
						//scanf("%s",temp_s);
						//0($3)
						//scanf("%s",temp_s);
						
						inst_type* lw = malloc(sizeof(inst_type));
						lw->inst = 35<<26;
						make_I_code(lw,2,'S');
						inst_array[text_length]=lw;
						text_length++;
						
					}
					else if(!strcmp(data_word,sw)){
						//scanf("%s",temp_s);
						//0($3)
						//scanf("%s",temp_s);
						
						inst_type* sw = malloc(sizeof(inst_type));
						sw->inst = 43<<26;
						make_I_code(sw,2,'S');
						inst_array[text_length]=sw;
						text_length++;
						
					}
				

					else if(!strcmp(data_word,sltiu)){
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* sltiu = malloc(sizeof(inst_type));
						sltiu->inst = 11<<26;
						make_I_code(sltiu,3,'N');
						inst_array[text_length] = sltiu;
						text_length++;
						
					}
					else if(!strcmp(data_word,addiu)){
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* addiu = malloc(sizeof(inst_type));
						addiu->inst = 9<<26;
						make_I_code(addiu,3,'N');
						inst_array[text_length] = addiu;
						text_length++;
						
					}
					
					// R 6 //op = 000000 func=??????
					else if(!strcmp(data_word,and)){ //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* and = malloc(sizeof(inst_type));
						and->inst = 36;
						make_R_code(and,3,'R');
						inst_array[text_length] = and;
						text_length++;
						
					}
					else if(!strcmp(data_word,jr)){  //r
						//scanf("%s",temp_s);
						
						inst_type* jr = malloc(sizeof(inst_type));
						jr->inst = 8;
						make_R_code(jr,1,'R');
						inst_array[text_length] = jr;
						text_length++;
						
					}
					else if(!strcmp(data_word,nor)){ //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* nor = malloc(sizeof(inst_type));
						nor->inst = 39;
						make_R_code(nor,3,'R');
						inst_array[text_length] = nor;
						text_length++;
						
					}
					else if(!strcmp(data_word,or)){  //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* or_ = malloc(sizeof(inst_type));
						or_->inst = 37;
						make_R_code(or_,3,'R');
						inst_array[text_length] = or_;
						text_length++;
						
					}
					
					else if(!strcmp(data_word,sll)){  //rr n
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* sll = malloc(sizeof(inst_type));
						sll->inst = 0;
						make_R_code(sll,3,'N');
						inst_array[text_length] = sll;
						text_length++;
						
					}
					else if(!strcmp(data_word,srl)){  //rr n
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* srl = malloc(sizeof(inst_type));
						srl->inst = 2;
						make_R_code(srl,3,'N');
						inst_array[text_length] = srl;
						text_length++;
						
					}
					else if(!strcmp(data_word,sltu)){ //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* sltu = malloc(sizeof(inst_type));
						sltu->inst = 43;
						make_R_code(sltu,3,'R');
						inst_array[text_length] = sltu;
						text_length++;
						
					}
					else if(!strcmp(data_word,addu)){ //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* addu = malloc(sizeof(inst_type));
						addu->inst = 33;
						make_R_code(addu,3,'R');
						inst_array[text_length] = addu;
						text_length++;
						
					}
					else if(!strcmp(data_word,subu)){ //rrr
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						inst_type* subu = malloc(sizeof(inst_type));
						subu->inst = 35;
						make_R_code(subu,3,'R');
						inst_array[text_length] = subu;
						text_length++;
						
					}
					
					//pseudo
					else if(!strcmp(data_word,la)){  
						// scanf("%s",temp_s);
						// scanf("%s",temp_s);
						
						char string[10];
						int  number;
						char temp[5];
						int k=0;
						scanf("%s",string);
						for(k=0; string[k+1]!=',';k++){
							temp[k]=string[k+1];
						}
						temp[k]=0;
						number = atoi(temp);						
						
						//lui
						inst_type* la_lui = malloc(sizeof(inst_type));
						la_lui->inst = 15<<26 | number<<16 | 1<<12;
						la_lui->type = 1;
						inst_array[text_length]=la_lui;
						text_length++;
						
						//ori
						inst_type* la_ori = malloc(sizeof(inst_type));
						scanf("%s",string);
						int data_order=0;
						int data_addr;
						la_ori->type = 1;
						if(strncmp(data_label[data_order].key,string,strlen(string))!=0){ //if LSB 16bit != 0x0000
							while(strncmp((data_label[data_order]).key,string,strlen(string))!=0){
								data_order++;
							}
							data_addr = data_label[data_order].value;
							la_ori->inst = 13<<26 | number<<21 | number<<16 | data_addr;
							inst_array[text_length]=la_ori;
							text_length++;
						}
						
					}
			}
			else{ //Label이면
				strcpy(text_label[t_label_num].key, data_word);
				text_label[t_label_num].value = text_length;
				t_label_num++;
			}
		}	
	}
//fill addr of jump instruction
	inst_type **detector = inst_array;
	int L_finder;
	int op_code;
	char *Labell;
	int table_num;
	int offset;
	for(L_finder=0; L_finder < text_length; L_finder++){
			op_code = (**detector).inst & 0xfc000000 ;
			if( op_code == 0x08000000 | op_code == 0x0c000000){
				Labell = (**detector).label;
				for(table_num = 0; table_num < t_label_num; table_num++){
					if(strncmp((text_label[table_num]).key, Labell, strlen(Labell))==0){
						(**detector).inst += 0x100000 + text_label[table_num].value;
					}
				}
			}
			else if(op_code == 0x10000000 | op_code == 0x14000000){
				Labell = (**detector).label;
				for(table_num = 0; table_num < t_label_num; table_num++){
					if(strncmp((text_label[table_num]).key, Labell, strlen(Labell))==0){
						//printf("%d    ",(text_label[table_num]).value); printf("%d\n",L_finder);
						if((text_label[table_num]).value < L_finder){
							offset = (text_label[table_num]).value - L_finder -1;
							//printf("offset: %d     \n",offset);
							(**detector).inst += offset & 0x0000ffff;
						}
						else if((text_label[table_num]).value > L_finder){
							offset = (text_label[table_num]).value - L_finder -1;
							(**detector).inst += offset & 0x0000ffff;
							//printf("offset: %d     \n",offset);
						}
						else{
							offset = (text_label[table_num]).value - L_finder;
							(**detector).inst += offset & 0x0000ffff;
							//printf("offset: %d     \n",offset);
						}
					}
				}
			}
			detector++;
	}
	//print
	print_inst(text_length*4);
	print_inst(data_length*4);

	inst_type **find = inst_array;
	int kk;
	for(kk=0;kk<text_length;kk++){
		print_inst((**find).inst);
		find++;
	}

	int p_data;
	for(p_data=0;p_data<data_length;p_data++){
		print_inst(data_table[p_data]);
	}
	

	return 0;
}


void print_inst(int inst) {
    int k = 0x80000000; // 1000 0000 0000 0000 0000 0000 0000 0000(2)
    if ((k & inst) == k)
 		printf("1");
	else
        printf("0");
	k = 0x40000000; // 0100 0000 0000 0000 0000 0000 0000 0000(2)
	for (int i = 0; i < 31; i++) {
        if ((k & inst) == k)
            printf("1");
        else
            printf("0");
        k >>= 1;
    }
}


_Bool Is_tag(char* op){
	char colon;
	while(*op==0){
		colon = *op;
		op++;
	}
	return colon == ':';
}

void make_I_code(inst_type* new,int scan_num, char last){ //N,L,S
	char string[10];
	char *copy =string+1;
	int  number;
	char temp[5];
	new->type=1;
	int k=0;
	if(scan_num==2 && last=='S'){
		scanf("%s",string);
		for(k=0; string[k+1]!=',';k++){
			temp[k]=string[k+1];
		}
		temp[k]=0;
		number = atoi(temp);
		new->inst|=number<<16;
		///////
		scanf("%s",string);
		int i=0;
		for(i=0;string[i]!='(';i++){

		}
		number = atoi(string); // first number (consider negative)
		number = number & 0x0000ffff; 
		new->inst|=number;
		//
		char* reg = string + i + 2;
		number = atoi(reg);
		new->inst|=number<<21;
	}
	else if(scan_num==2 && last=='N'){
		scanf("%s",string);
		number = atoi(copy);
		new->inst|=number<<16;
		//////
		scanf("%d",&number);
		if(number==0){
			scanf("%s",string);
			number = sixtoten(copy);  //16 -> 10
		}
		new->inst|=number;
	}
	else if(scan_num==3 && last=='N'){
		scanf("%s",string);
		number = atoi(copy);
		new->inst|=number<<16;
		//
		scanf("%s",string);
		number = atoi(copy);
		new->inst|=number<<21;
		//////////
		scanf("%d",&number);
		if(number==0){
			scanf("%s",string);
			number = sixtoten(copy); //16 -> 10
		}
		new->inst|=number;
	}
	else if(scan_num==3 && last=='L'){
		scanf("%s",string);
		number = atoi(copy);
		new->inst|=number<<21;
		//
		scanf("%s",string);
		number = atoi(copy);
		new->inst|=number<<16;
		///// Label
		scanf("%s",string);
		strcpy(new->label, string);
	}
}


void make_R_code(inst_type* new,int scan_num, char last){
	char string[10];
	char* copy = string+1;
	int  number;
	new->type=2;
	int k=0;
	if(scan_num==1){
		scanf("%s",string);
		number = atoi(copy);
		new->inst+=number<<21;
	}
	else if(scan_num==3 && last=='R'){
		int shift[3]={11,21,16};
		for(int i=0; i<3; i++){
			scanf("%s",string);
			number = atoi(copy);
			new->inst+=number<<shift[i];
		}
		


	}
	else if(scan_num==3 && last=='N'){
		int shift[2]={11,16};
		for(int i=0; i<2; i++){
			scanf("%s",string);
			number = atoi(copy);
			new->inst+=number<<shift[i];
		}
		scanf("%d",&number);
		if(number==0){
			scanf("%s",string);
			number = sixtoten(copy); //16 -> 10
		}
		new->inst+=number<<6;
	}



}

int sixtoten(char* copy){
	int length = strlen(copy) -1;
	int i = length;
	int result = 0;
	int six=1;
	for(;i >= 0; i--){
		if(*(copy+i)<58){
			result += (*(copy+i)-48) * six;
		}
		else{
			result += (*(copy+i)-87) * six;
		}
		six *=16;	
	}
	return result;
}