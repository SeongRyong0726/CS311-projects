/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

void FLUSH(void);
void IF_FLUSH(void);
void Sig_Produce(instruction *instr);
void flush_IF_ID(void);
void flush_ID_EX(void);


//instruction NULL_inst = 0;
int PCSrc = 0;
int flush_pc = 0;
int next_time = 0;


	void Forwarding(void){
		if(CURRENT_STATE.EX_MEM_SIG_RegWrite==true && CURRENT_STATE.EX_MEM_DEST!=0 && CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_RS){ //RegWrite && 
			CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.EX_MEM_ALU_OUT;
			//ForwardA = 10; //걍 바로 값을 바꿔
		}
		if(CURRENT_STATE.EX_MEM_SIG_RegWrite==true && CURRENT_STATE.EX_MEM_DEST!=0 && CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_DESTRT){ //RegWrite && 
			CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.EX_MEM_ALU_OUT;
			//ForwardB = 10;
		}
		if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_DEST!=0 && CURRENT_STATE.MEM_WB_DEST==CURRENT_STATE.ID_EX_RS){ //RegWrite && 
			if(!(CURRENT_STATE.EX_MEM_SIG_RegWrite==true && CURRENT_STATE.EX_MEM_DEST!=0 && CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_RS)){
				
				if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==true){ //lw
					CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_MEM_OUT;
				}
				if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==false){ //R type
					CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.MEM_WB_ALU_OUT;
				}				
				//ForwardA = 01;
			}
		}
		if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_DEST!=0 && CURRENT_STATE.MEM_WB_DEST==CURRENT_STATE.ID_EX_DESTRT){ //RegWrite && 
			if(!(CURRENT_STATE.EX_MEM_SIG_RegWrite==true && CURRENT_STATE.EX_MEM_DEST!=0 && CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_DESTRT)){
				if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==true){ //lw
					CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_MEM_OUT;
				}
				if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==false){ //R type
					CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.MEM_WB_ALU_OUT;
				}
				//ForwardB = 01;
				//RegDst =0; ALUSrc =0; Branch =1; MemRead =0; MemWrite =0; RegWrite =0; MemtoReg =0;
			}
		}
		//WB_TO_MEM
		if(CURRENT_STATE.MEM_WB_SIG_MemRead==true && CURRENT_STATE.EX_MEM_SIG_MemWrite==true && (CURRENT_STATE.MEM_WB_DEST==RT(CURRENT_STATE.EX_MEM_INST))){
			CURRENT_STATE.EX_MEM_W_VALUE = CURRENT_STATE.MEM_WB_MEM_OUT;
		}
	}

	void Stall(void){
		if(CURRENT_STATE.EX_MEM_SIG_MemRead==true && CURRENT_STATE.ID_EX_SIG_MemRead==false && ((CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_RS | CURRENT_STATE.EX_MEM_DEST==CURRENT_STATE.ID_EX_DESTRT))){
			CURRENT_STATE.PIPE_STALL[0] = 1; CURRENT_STATE.PIPE_STALL[1] = 1;
			//printf("%dABCDE\n",CYCLE_COUNT);
		}
	}

	void WB_Stage(void){
		if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==true){ //lw
			CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_MEM_OUT;
		}
		if(CURRENT_STATE.MEM_WB_SIG_RegWrite==true && CURRENT_STATE.MEM_WB_SIG_MemtoReg==false){ //R type
			CURRENT_STATE.REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_ALU_OUT;
		}
		//##FOR printing PC value
		if(CURRENT_STATE.MEM_WB_NPC != 0){
			CURRENT_STATE.PIPE[4] = CURRENT_STATE.MEM_WB_NPC-4;	
			INSTRUCTION_COUNT++;
		}
		else	CURRENT_STATE.PIPE[4] = 0;
	}

	void MEM_Stage(void){
		CURRENT_STATE.MEM_WB_INST = CURRENT_STATE.EX_MEM_INST;
		CURRENT_STATE.MEM_WB_NPC = CURRENT_STATE.EX_MEM_NPC;
		CURRENT_STATE.MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
		CURRENT_STATE.MEM_WB_MEM_OUT = 0;
		CURRENT_STATE.MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
		CURRENT_STATE.MEM_WB_SIG_RegWrite = CURRENT_STATE.EX_MEM_SIG_RegWrite;
		CURRENT_STATE.MEM_WB_SIG_MemtoReg = CURRENT_STATE.EX_MEM_SIG_MemtoReg;
		CURRENT_STATE.MEM_WB_SIG_MemRead = CURRENT_STATE.EX_MEM_SIG_MemRead;


		if(CURRENT_STATE.EX_MEM_SIG_Branch==true && CURRENT_STATE.EX_MEM_BR_TAKE==1){  		//beq, bne RegDst =0; ALUSrc =0; Branch =1; MemRead =0; MemWrite =0; RegWrite =0; MemtoReg =0;
			PCSrc = 1;
			CURRENT_STATE.BRANCH_PC = CURRENT_STATE.EX_MEM_BR_TARGET;
			FLUSH();
		}
		else if(CURRENT_STATE.EX_MEM_SIG_MemRead ==true){ 	//lw
			//printf("MEM STAGE lw addr: 0x%08x",CURRENT_STATE.EX_MEM_ALU_OUT );
			CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
		}
		else if(CURRENT_STATE.EX_MEM_SIG_MemWrite ==true){ 	//sw
			mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, CURRENT_STATE.EX_MEM_W_VALUE);
		}
		//##FOR printing PC value
		if(CURRENT_STATE.EX_MEM_NPC != 0){
			CURRENT_STATE.PIPE[3] = CURRENT_STATE.EX_MEM_NPC-4;	
		}
		else	CURRENT_STATE.PIPE[3] = 0;
	}

	void EX_Stage(void){
		int operand1 = CURRENT_STATE.ID_EX_REG1; //RS
		int operand2 = CURRENT_STATE.ID_EX_REG2; //RT
		if(CURRENT_STATE.EX_MEM_SIG_MemRead==TRUE){
			//printf("load in EX_STAGE in %d\n", CYCLE_COUNT+1);
		}
		CURRENT_STATE.EX_MEM_INST = CURRENT_STATE.ID_EX_INST;
		CURRENT_STATE.EX_MEM_NPC = CURRENT_STATE.ID_EX_NPC;
		CURRENT_STATE.EX_MEM_ALU_OUT =0;
		CURRENT_STATE.EX_MEM_W_VALUE =0;
		CURRENT_STATE.EX_MEM_BR_TARGET =0;
		CURRENT_STATE.EX_MEM_BR_TAKE =0;
		CURRENT_STATE.EX_MEM_DEST =0;
		
		CURRENT_STATE.EX_MEM_SIG_MemtoReg = CURRENT_STATE.ID_EX_SIG_MemtoReg;
		CURRENT_STATE.EX_MEM_SIG_RegWrite = CURRENT_STATE.ID_EX_SIG_RegWrite;
		CURRENT_STATE.EX_MEM_SIG_MemWrite = CURRENT_STATE.ID_EX_SIG_MemWrite;
		CURRENT_STATE.EX_MEM_SIG_MemRead = CURRENT_STATE.ID_EX_SIG_MemRead;
		CURRENT_STATE.EX_MEM_SIG_Branch = CURRENT_STATE.ID_EX_SIG_Branch;
		CURRENT_STATE.EX_MEM_SIG_Zero = 0;




		if(CURRENT_STATE.ID_EX_INST != NULL){
			//RegDst
			if(CURRENT_STATE.ID_EX_SIG_RegDst==true){
				CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_DESTRD; //RD(CURRENT_STATE.ID_EX_INST);
			}
			else{
				CURRENT_STATE.EX_MEM_DEST = CURRENT_STATE.ID_EX_DESTRT; //RT(CURRENT_STATE.ID_EX_INST); 
			}
			//Branch cal
			CURRENT_STATE.EX_MEM_BR_TARGET = CURRENT_STATE.ID_EX_NPC + ((uint32_t)(0x00000000 | CURRENT_STATE.ID_EX_IMM)<<2);
			//FOR sw write value
			CURRENT_STATE.EX_MEM_W_VALUE = CURRENT_STATE.ID_EX_REG2;
			//ALUSrc '"NO NEEDED"'			
			
			//ALU
			switch (OPCODE(CURRENT_STATE.ID_EX_INST))
			{
			case 0x9:		//(0x001001)ADDIU RT
				CURRENT_STATE.EX_MEM_ALU_OUT = operand1 + (short) IMM (CURRENT_STATE.ID_EX_INST);
				break;
			case 0xc:		//(0x001100)ANDI RT
				CURRENT_STATE.EX_MEM_ALU_OUT = operand1 & (0xffff & IMM (CURRENT_STATE.ID_EX_INST));
				break;
			case 0xf:		//(0x001111)LUI	RT
				CURRENT_STATE.EX_MEM_ALU_OUT = (IMM (CURRENT_STATE.ID_EX_INST) << 16) & 0xffff0000;
				//printf("LUI: 0x%x",CURRENT_STATE.EX_MEM_ALU_OUT);
				break;
			case 0xd:		//(0x001101)ORI RT
				CURRENT_STATE.EX_MEM_ALU_OUT = operand1 | (0xffff & IMM (CURRENT_STATE.ID_EX_INST));
				break;
			case 0xb:		//(0x001011)SLTIU RT
				{
				int x = (short) IMM (CURRENT_STATE.ID_EX_INST);

				if ((uint32_t) operand1 < (uint32_t) x)
					CURRENT_STATE.EX_MEM_ALU_OUT = 1;
				else
					CURRENT_STATE.EX_MEM_ALU_OUT = 0;
				break;
				}
			case 0x23:		//(0x100011)LW	XX
				CURRENT_STATE.EX_MEM_ALU_OUT = operand1 + IMM (CURRENT_STATE.ID_EX_INST); //RS+imm
				//printf("0x%08x", operand1 ); printf("+ 0x%08x", IMM (CURRENT_STATE.ID_EX_INST) ); printf("= 0x%08x", CURRENT_STATE.EX_MEM_ALU_OUT );
				//printf("LW ADDRESS: 0x%x",CURRENT_STATE.EX_MEM_ALU_OUT);
				break;
			case 0x2b:		//(0x101011)SW  XX
				CURRENT_STATE.EX_MEM_ALU_OUT = operand1 + IMM (CURRENT_STATE.ID_EX_INST);
				break;
			case 0x4:		//(0x000100)BEQ  XX
				if(operand1 == operand2){
					CURRENT_STATE.EX_MEM_BR_TAKE = 1;
				}
				else CURRENT_STATE.EX_MEM_BR_TAKE = 0;
				break;
			case 0x5:		//(0x000101)BNE  XX
				//printf("op1: %d, op2: %d\n", operand1, operand2);
				if(operand1 != operand2){
				CURRENT_STATE.EX_MEM_BR_TAKE = 1;
				}
				else CURRENT_STATE.EX_MEM_BR_TAKE = 0;
				break;
			case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
				{
				switch(FUNC (CURRENT_STATE.ID_EX_INST)){
					case 0x21:	//ADDU  RD
					CURRENT_STATE.EX_MEM_ALU_OUT = operand1 + operand2;
					break;
					case 0x24:	//AND  RD
					CURRENT_STATE.EX_MEM_ALU_OUT = operand1 & operand2;
					break;
					case 0x27:	//NOR  RD
					CURRENT_STATE.EX_MEM_ALU_OUT = ~ (operand1 | operand2);
					break;
					case 0x25:	//OR  RD
					CURRENT_STATE.EX_MEM_ALU_OUT = operand1 | operand2;
					break;
					case 0x2B:	//SLTU  RD
					if ( operand1 <  operand2)
						CURRENT_STATE.EX_MEM_ALU_OUT = 1;
					else
						CURRENT_STATE.EX_MEM_ALU_OUT = 0;
					break;
					case 0x0:	//SLL  RD
					{
						int shamt = SHAMT (CURRENT_STATE.ID_EX_INST);

						if (shamt >= 0 && shamt < 32)
						CURRENT_STATE.EX_MEM_ALU_OUT = operand2 << shamt;
						else
						CURRENT_STATE.EX_MEM_ALU_OUT = operand2;
						break;
					}
					case 0x2:	//SRL  RD
					{
						int shamt = SHAMT (CURRENT_STATE.ID_EX_INST);
						uint32_t val = operand2;

						if (shamt >= 0 && shamt < 32)
						CURRENT_STATE.EX_MEM_ALU_OUT = val >> shamt;
						else
						CURRENT_STATE.EX_MEM_ALU_OUT = val;
						break;
					}
					case 0x23:	//SUBU  RD
					CURRENT_STATE.EX_MEM_ALU_OUT = operand1 - operand2;
					break;

					case 0x8:	//JR  얘는 ID로 가야하나???
					{
						IF_FLUSH();
						break;
					}
					default:
					printf("Unknown function code type: %d\n", FUNC(CURRENT_STATE.ID_EX_INST));
					break;
					}
				}
				break;

			case 0x2:		//(0x000010)J  ???
				IF_FLUSH();
				break;
			case 0x3:		//(0x000011)JAL   ???
				IF_FLUSH();
				break;

			default:
				printf("Unknown instruction type: %d\n", OPCODE(CURRENT_STATE.ID_EX_INST));
				break;
			}
		}
		//##FOR printing PC value
		if(CURRENT_STATE.ID_EX_NPC != 0){
			CURRENT_STATE.PIPE[2] = CURRENT_STATE.ID_EX_NPC-4;	
		}
		else	CURRENT_STATE.PIPE[2] = 0;
		
	}
	void ID_Stage(void){
		
		//latch store
		CURRENT_STATE.ID_EX_INST = CURRENT_STATE.IF_ID_INST;
		CURRENT_STATE.ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;
		if(CURRENT_STATE.IF_ID_INST != NULL){
			CURRENT_STATE.ID_EX_REG1 = CURRENT_STATE.REGS[RS(CURRENT_STATE.IF_ID_INST)];
			CURRENT_STATE.ID_EX_REG2 = CURRENT_STATE.REGS[RT(CURRENT_STATE.IF_ID_INST)];
			CURRENT_STATE.ID_EX_IMM = IMM(CURRENT_STATE.IF_ID_INST);
			CURRENT_STATE.ID_EX_DESTRD = RD(CURRENT_STATE.IF_ID_INST);
			CURRENT_STATE.ID_EX_DESTRT = RT(CURRENT_STATE.IF_ID_INST);
			CURRENT_STATE.ID_EX_RS = RS(CURRENT_STATE.IF_ID_INST);

			Sig_Produce(CURRENT_STATE.IF_ID_INST);

			switch (OPCODE(CURRENT_STATE.IF_ID_INST))
			{
				case 0x2:		//(0x000010)J  ???
					CURRENT_STATE.JUMP_PC = (((CURRENT_STATE.IF_ID_NPC & 0xf0000000) | TARGET (CURRENT_STATE.IF_ID_INST) << 2));
					PCSrc=2;
					next_time=1;
					break;
				case 0x3:		//(0x000011)JAL   ???
					CURRENT_STATE.REGS[31] = CURRENT_STATE.IF_ID_NPC ; // In this project, we dind't consider the delay slot ! (2019-10-02 fixed by sjna)
					CURRENT_STATE.JUMP_PC = (((CURRENT_STATE.IF_ID_NPC & 0xf0000000) | (TARGET (CURRENT_STATE.IF_ID_INST) << 2)));
					PCSrc=2;
					next_time=1;
					break;
				case 0x0:
					switch(FUNC (CURRENT_STATE.IF_ID_INST)){
						case 0x8:	//JR  얘는 ID로 가야하나???
						{
							CURRENT_STATE.JUMP_PC = CURRENT_STATE.REGS[RS(CURRENT_STATE.IF_ID_INST)];
							PCSrc=2;
							next_time=1;
							break;
						}
					}
					break;
			}
		}
		if(CURRENT_STATE.IF_ID_INST == NULL){
			/////////////////////////////////////////////////////////
			CURRENT_STATE.ID_EX_SIG_Jump = false;
			CURRENT_STATE.ID_EX_SIG_RegDst = false;
			CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
			CURRENT_STATE.ID_EX_SIG_Branch = false;
			CURRENT_STATE.ID_EX_SIG_MemRead = false;
			CURRENT_STATE.ID_EX_SIG_MemWrite = false;
			CURRENT_STATE.ID_EX_SIG_RegWrite = false;
			CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
		}
		if(CURRENT_STATE.IF_ID_NPC != 0){
			CURRENT_STATE.PIPE[1] = CURRENT_STATE.IF_ID_NPC-4;	
		}
		else	CURRENT_STATE.PIPE[1] = CURRENT_STATE.IF_ID_NPC;
	}
	void IF_Stage(void){
		instruction *inst;
    	if(FETCH_BIT){
			if(flush_pc==1){
				inst = NULL;
				CURRENT_STATE.PIPE[0] = 0;
				flush_pc=0;
				CURRENT_STATE.PC += BYTES_PER_WORD;
				CURRENT_STATE.IF_ID_NPC = 0;
			}
			else if(PCSrc == 0 | next_time ==1 ){
				next_time = 0;
				inst = get_inst_info(CURRENT_STATE.PC);
				CURRENT_STATE.PIPE[0] = CURRENT_STATE.PC;
				CURRENT_STATE.PC += BYTES_PER_WORD;
				CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
			}
			else if(PCSrc == 1){
				inst = get_inst_info(CURRENT_STATE.BRANCH_PC);
				CURRENT_STATE.PIPE[0] = CURRENT_STATE.BRANCH_PC;
				PCSrc = 0;
				CURRENT_STATE.PC = CURRENT_STATE.BRANCH_PC + BYTES_PER_WORD;
				CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
			}
			else if(PCSrc == 2){
				inst = get_inst_info(CURRENT_STATE.JUMP_PC);
				CURRENT_STATE.PIPE[0] = CURRENT_STATE.JUMP_PC;
				PCSrc = 0;
				CURRENT_STATE.PC = CURRENT_STATE.JUMP_PC + BYTES_PER_WORD;
				CURRENT_STATE.IF_ID_NPC = CURRENT_STATE.PC;
			}
		}
		else{
			inst = NULL;
			CURRENT_STATE.PIPE[0] = 0;
			CURRENT_STATE.IF_ID_NPC = 0;
		}
		CURRENT_STATE.IF_ID_INST = inst;
	}
	void Sig_Produce(instruction *instr){
		if(instr != NULL){
			switch (OPCODE(instr))
			{
			case 0x9:		//(0x001001)ADDIU RT
			case 0xc:		//(0x001100)ANDI RT
			case 0xf:		//(0x001111)LUI	RT
			case 0xd:		//(0x001101)ORI RT
			case 0xb:		//(0x001011)SLTIU RT
			        //printf("I type\n");
					CURRENT_STATE.ID_EX_SIG_Jump = false;
					CURRENT_STATE.ID_EX_SIG_RegDst = false;
					CURRENT_STATE.ID_EX_SIG_ALUSrc = true;
					CURRENT_STATE.ID_EX_SIG_Branch = false;
					CURRENT_STATE.ID_EX_SIG_MemRead = false;
					CURRENT_STATE.ID_EX_SIG_MemWrite = false;
					CURRENT_STATE.ID_EX_SIG_RegWrite = true;
					CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
				break;
			case 0x23:		//(0x100011)LW	XX
					//printf("LW\n");
					CURRENT_STATE.ID_EX_SIG_Jump = false;
					CURRENT_STATE.ID_EX_SIG_RegDst = false;
					CURRENT_STATE.ID_EX_SIG_ALUSrc = true;
					CURRENT_STATE.ID_EX_SIG_Branch = false;
					CURRENT_STATE.ID_EX_SIG_MemRead = true;
					CURRENT_STATE.ID_EX_SIG_MemWrite = false;
					CURRENT_STATE.ID_EX_SIG_RegWrite = true;
					CURRENT_STATE.ID_EX_SIG_MemtoReg = true;
				break;
			case 0x2b:		//(0x101011)SW  XX
					//printf("SW  \n");
					CURRENT_STATE.ID_EX_SIG_Jump = false;
					CURRENT_STATE.ID_EX_SIG_RegDst = false;
					CURRENT_STATE.ID_EX_SIG_ALUSrc = true;
					CURRENT_STATE.ID_EX_SIG_Branch = false;
					CURRENT_STATE.ID_EX_SIG_MemRead = false;
					CURRENT_STATE.ID_EX_SIG_MemWrite = true;
					CURRENT_STATE.ID_EX_SIG_RegWrite = false;
					CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
				break;
			case 0x4:		//(0x000100)BEQ  XX
			case 0x5:		//(0x000101)BNE  XX
					//printf("Branch\n");
					CURRENT_STATE.ID_EX_SIG_Jump = false;
					CURRENT_STATE.ID_EX_SIG_RegDst = false;
					CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
					CURRENT_STATE.ID_EX_SIG_Branch = true;
					CURRENT_STATE.ID_EX_SIG_MemRead = false;
					CURRENT_STATE.ID_EX_SIG_MemWrite = false;
					CURRENT_STATE.ID_EX_SIG_RegWrite = false;
					CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
				break;
			case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
				{
				switch(FUNC (instr)){
					case 0x21:	//ADDU  RD
					case 0x24:	//AND  RD
					case 0x27:	//NOR  RD
					case 0x25:	//OR  RD
					case 0x2B:	//SLTU  RD
					case 0x0:	//SLL  RD
					case 0x2:	//SRL  RD
					case 0x23:	//SUBU  RD
						//printf("R type\n");
						CURRENT_STATE.ID_EX_SIG_Jump = false;
						CURRENT_STATE.ID_EX_SIG_RegDst = true;
						CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
						CURRENT_STATE.ID_EX_SIG_Branch = false;
						CURRENT_STATE.ID_EX_SIG_MemRead = false;
						CURRENT_STATE.ID_EX_SIG_MemWrite = false;
						CURRENT_STATE.ID_EX_SIG_RegWrite = true;
						CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
					break;
					case 0x8:	//JR  얘는 ID로 가야하나???
						//printf("JR\n");
						CURRENT_STATE.ID_EX_SIG_Jump = true;
						CURRENT_STATE.ID_EX_SIG_RegDst = false;
						CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
						CURRENT_STATE.ID_EX_SIG_Branch = false;
						CURRENT_STATE.ID_EX_SIG_MemRead = false;
						CURRENT_STATE.ID_EX_SIG_MemWrite = false;
						CURRENT_STATE.ID_EX_SIG_RegWrite = false;
						CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
					break;
					}
				}
				break;

			case 0x2:		//(0x000010)J  ???
						//printf("J\n");
						CURRENT_STATE.ID_EX_SIG_Jump = true;
						CURRENT_STATE.ID_EX_SIG_RegDst = false;
						CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
						CURRENT_STATE.ID_EX_SIG_Branch = false;
						CURRENT_STATE.ID_EX_SIG_MemRead = false;
						CURRENT_STATE.ID_EX_SIG_MemWrite = false;
						CURRENT_STATE.ID_EX_SIG_RegWrite = false;
						CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
			case 0x3:		//(0x000011)JAL   ??? 혹시모름
						//printf("JAL\n");
						CURRENT_STATE.ID_EX_SIG_Jump = true;
						CURRENT_STATE.ID_EX_SIG_RegDst = false;
						CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
						CURRENT_STATE.ID_EX_SIG_Branch = false;
						CURRENT_STATE.ID_EX_SIG_MemRead = false;
						CURRENT_STATE.ID_EX_SIG_MemWrite = false;
						CURRENT_STATE.ID_EX_SIG_RegWrite = false;
						CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
				break;

			default:
				printf("Unknown instruction type: %d\n", OPCODE(instr));
				break;
			}
		}
	}
	void FLUSH(void){
		flush_pc = 1;
		flush_IF_ID();
		flush_ID_EX();
		//CURRENT_STATE.EX_MEM_INST = NULL; CURRENT_STATE.EX_MEM_SIGNAL = 0; CURRENT_STATE.EX_MEM_NPC =0;
		//CURRENT_STATE.PIPE[0]=0; CURRENT_STATE.PIPE[1]=0; CURRENT_STATE.PIPE[2]=0; 
	}
	void IF_FLUSH(void){
		CURRENT_STATE.IF_ID_INST = NULL; CURRENT_STATE.IF_ID_NPC =0;
	}
	void flush_EX_MEM(void){
		CURRENT_STATE.EX_MEM_INST = NULL;
		CURRENT_STATE.EX_MEM_NPC =0;
		CURRENT_STATE.EX_MEM_ALU_OUT =0;
		CURRENT_STATE.EX_MEM_W_VALUE =0;
		CURRENT_STATE.EX_MEM_BR_TARGET =0;
		CURRENT_STATE.EX_MEM_BR_TAKE =0;
		CURRENT_STATE.EX_MEM_DEST =0;

		CURRENT_STATE.EX_MEM_SIG_Zero =0;
		CURRENT_STATE.EX_MEM_SIG_Branch =0;
		CURRENT_STATE.EX_MEM_SIG_MemRead =0;
		CURRENT_STATE.EX_MEM_SIG_MemWrite =0;
		CURRENT_STATE.EX_MEM_SIG_RegWrite =0;
		CURRENT_STATE.EX_MEM_SIG_MemtoReg =0;

		CURRENT_STATE.PIPE[2] = 0;
	}
	void flush_ID_EX(void){
		CURRENT_STATE.ID_EX_INST = NULL;
		CURRENT_STATE.ID_EX_NPC = 0;
		CURRENT_STATE.ID_EX_REG1 = 0;
		CURRENT_STATE.ID_EX_REG2 = 0;
		CURRENT_STATE.ID_EX_IMM = 0;
		CURRENT_STATE.ID_EX_DESTRD = 0;
		CURRENT_STATE.ID_EX_DESTRT = 0;

		CURRENT_STATE.ID_EX_SIG_Jump = false;
		CURRENT_STATE.ID_EX_SIG_RegDst = false;
		CURRENT_STATE.ID_EX_SIG_ALUSrc = false;
		CURRENT_STATE.ID_EX_SIG_Branch = false;
		CURRENT_STATE.ID_EX_SIG_MemRead = false;
		CURRENT_STATE.ID_EX_SIG_MemWrite = false;
		CURRENT_STATE.ID_EX_SIG_RegWrite = false;
		CURRENT_STATE.ID_EX_SIG_MemtoReg = false;
	}
	void flush_IF_ID(void){
		CURRENT_STATE.IF_ID_INST = NULL; CURRENT_STATE.IF_ID_NPC =0;
	}
/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/

instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/* TODO: Implement 5-stage pipeplined MIPS simulator           */
/*                                                             */
/***************************************************************/
//int RegDst; int ALUSrc; int Branch; int MemRead; int MemWrite; int RegWrite; int MemtoReg;
int already_flush = 0;
int buffer = 5;
void process_instruction(){
	// 현재 상황에서 forwarding필요한거, stall필요한거 다 찾아서 준비시킴.
	if(CURRENT_STATE.ID_EX_SIG_Jump==TRUE && CURRENT_STATE.ID_EX_NPC !=0){
		FETCH_BIT=TRUE; 
		//printf("%d    rstart\n", CYCLE_COUNT);
	}
	Forwarding();
	Stall();
	////////////////singal
	WB_Stage();//RegWrite, MemtoReg
	MEM_Stage();		  //Branch, MemReag, MemWrite
	EX_Stage();  //RegDst,  ALUSrc

	if(CURRENT_STATE.PIPE_STALL[1]==0)   {	ID_Stage(); }
	else{
		//printf("%dSTAAA EX_MEM_flush\n",CYCLE_COUNT);
		CURRENT_STATE.PIPE_STALL[1]-=1;
		flush_EX_MEM();
		already_flush = 1;
	}
	if(CURRENT_STATE.PIPE_STALL[0]==0){
		IF_Stage();
	}
	else{
		CURRENT_STATE.PIPE_STALL[0]-=1;
		//printf("%dSTAAA not flush ID_EX\n",CYCLE_COUNT);
		if(already_flush==0)  {flush_ID_EX();}
		already_flush =0;
		}


    if (CURRENT_STATE.PC < MEM_REGIONS[0].start || CURRENT_STATE.PC >= (MEM_REGIONS[0].start + (NUM_INST * 4))){
		FETCH_BIT=FALSE;
	}
	if(CURRENT_STATE.PIPE[0]==0 && CURRENT_STATE.PIPE[1]==0 && CURRENT_STATE.PIPE[2]==0 && CURRENT_STATE.PIPE[3]==0){
		RUN_BIT=FALSE;
	}
}
////////////////////////////////////////////////////////
