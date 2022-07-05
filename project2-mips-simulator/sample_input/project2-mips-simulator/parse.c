/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
	int Number_instr = fromBinary((char*)buffer);
	instr.value = index/4;
	//R type
	if((Number_instr & 0xfc000000) == 0x00000000){
		instr.opcode = 0;
		instr.func_code = (short)(Number_instr & 0x0000003f);
		instr.r_t.r_i.rs =  (unsigned char) ((Number_instr >>21) & 0x0000001f);
		instr.r_t.r_i.rt = (unsigned char) ((Number_instr >>16) & 0x0000001f);
		instr.r_t.r_i.r_i.r.rd =(unsigned char) ((Number_instr >>11) & 0x0000001f);
		instr.r_t.r_i.r_i.r.shamt =(unsigned char)((Number_instr >>6) & 0x0000001f);;
	}
	//J type
	else if( ((Number_instr & 0xfc000000) == 0x08000000) || ((Number_instr & 0xfc000000) == 0x0c000000) ){
		instr.opcode = (short)( ((unsigned int)Number_instr>>26) & 0x0000003f);
		instr.r_t.target = (uint32_t)(Number_instr & 0x03ffffff);
	}
	//I type
	else{
		instr.opcode = ((unsigned int)Number_instr & 0xfc000000)>>26;
		instr.r_t.r_i.rs = (unsigned char) ((Number_instr >>21) & 0x0000001f);
		instr.r_t.r_i.rt = (unsigned char) ((Number_instr >>16) & 0x0000001f);
		instr.r_t.r_i.r_i.imm = (short) Number_instr & 0x0000ffff;
	}
	//INST_INFO[index/4] = instr;
	mem_write_32(0x00400000+index,fromBinary((char*)buffer));
	/** Implement this function */
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	int data = fromBinary((char*)buffer);
	mem_write_32(0x10000000+index, data);
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //Type I
	    case 0x9:		//(0x001001)ADDIU
	    case 0xc:		//(0x001100)ANDI
	    case 0xf:		//(0x001111)LUI	
	    case 0xd:		//(0x001101)ORI
	    case 0xb:		//(0x001011)SLTIU
	    case 0x23:		//(0x100011)LW	
	    case 0x2b:		//(0x101011)SW
	    case 0x4:		//(0x000100)BEQ
	    case 0x5:		//(0x000101)BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	    //TYPE R
	    case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU  if JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	    //TYPE J
	    case 0x2:		//(0x000010)J
	    case 0x3:		//(0x000011)JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
