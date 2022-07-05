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

//^^
#include "parse.h"
int finish = 0;
/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){
    if(finish == 1){
        //printf("finish !!\n");
        return;
    }
    //printf("text_size : %d", text_size/4);
	instruction *inst = INST_INFO[(CURRENT_STATE.PC-0x400000)/4];
    if(inst.opcode == 0){ // R type
        switch((int)inst.func_code){
            case 0: //SLL
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] =  CURRENT_STATE.REGS[inst.r_t.r_i.rt] << inst.r_t.r_i.r_i.r.shamt;
                CURRENT_STATE.PC+=4;
                //printf("1//");
                break;
            case 2: //SRL
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] =  CURRENT_STATE.REGS[inst.r_t.r_i.rt] >> inst.r_t.r_i.r_i.r.shamt; // ^^unsigned 해야함
                CURRENT_STATE.PC+=4;
                //printf("2//");
                break;
            case 8: //JR
                CURRENT_STATE.PC = CURRENT_STATE.REGS[inst.r_t.r_i.rs];
                //printf("3//");
                break;
            case 33: //ADDU
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] + CURRENT_STATE.REGS[inst.r_t.r_i.rt];
                CURRENT_STATE.PC+=4;
                //printf("4//");
                break;
            case 35: //SUBU
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] - CURRENT_STATE.REGS[inst.r_t.r_i.rt];
                CURRENT_STATE.PC+=4;
                //printf("5//");
                break;
            case 36: //AND
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] & CURRENT_STATE.REGS[inst.r_t.r_i.rt];
                CURRENT_STATE.PC+=4;
                //printf("6//");
                break;
            case 37: //OR
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] | CURRENT_STATE.REGS[inst.r_t.r_i.rt];
                CURRENT_STATE.PC+=4;
                //printf("7//");
                break;
            case 39: //NOR 
                CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = ~ (CURRENT_STATE.REGS[inst.r_t.r_i.rs] | CURRENT_STATE.REGS[inst.r_t.r_i.rt]);
                CURRENT_STATE.PC+=4;
                //printf("8//");
                break;
            case 43: //SLTU
                if(CURRENT_STATE.REGS[inst.r_t.r_i.rs] < CURRENT_STATE.REGS[inst.r_t.r_i.rt]){
                    CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = 1;
                }
                else{
                    CURRENT_STATE.REGS[inst.r_t.r_i.r_i.r.rd] = 0;
                }
                CURRENT_STATE.PC+=4;
                //printf("9//");
                break;
        }

    }
    else if(inst.opcode == 0x2  || inst.opcode ==0x3 ){ // J type
        switch((int)inst.opcode){
            case 0x2: //J
                CURRENT_STATE.PC = inst.r_t.target*4; 
                //printf("10//");
                break;  
            case 0x3: //JAL
                CURRENT_STATE.REGS[31] = CURRENT_STATE.PC+4;
                CURRENT_STATE.PC = (inst.r_t.target)*4 ;
                //printf("11//");
                break;
        }
    }
    else{ // I type
        switch((int)inst.opcode){
            case 4: //BEQ
                if(CURRENT_STATE.REGS[inst.r_t.r_i.rs] == CURRENT_STATE.REGS[inst.r_t.r_i.rt]){
                    CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + ((uint32_t)inst.r_t.r_i.r_i.imm * 4) ;
                }
                else{
                    CURRENT_STATE.PC+=4;
                    if((int)inst.value==((text_size/4)-1)){
                        finish = 1;
                        RUN_BIT = FALSE;
                    }
                }
                //printf("12//");
                break;
            case 5: //BNE
                if(CURRENT_STATE.REGS[inst.r_t.r_i.rs] != CURRENT_STATE.REGS[inst.r_t.r_i.rt]){
                    CURRENT_STATE.PC = CURRENT_STATE.PC + 4 + ((uint32_t)inst.r_t.r_i.r_i.imm * 4) ;
                }
                else{
                    CURRENT_STATE.PC+=4;
                    if((int)inst.value==((text_size/4)-1)){
                        finish = 1;
                        RUN_BIT = FALSE;
                    }
                }
                //printf("13//");
                break;
            case 9: //ADDIU
                CURRENT_STATE.REGS[inst.r_t.r_i.rt] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] + (uint32_t)inst.r_t.r_i.r_i.imm;
                CURRENT_STATE.PC+=4;
                //printf("14//");
                break;
            case 11: //SLTIU
                
                if(CURRENT_STATE.REGS[inst.r_t.r_i.rs] < (uint32_t)inst.r_t.r_i.r_i.imm){
                    CURRENT_STATE.REGS[inst.r_t.r_i.rt] = 1;
                }
                else{
                    CURRENT_STATE.REGS[inst.r_t.r_i.rt] = 0;
                }
                CURRENT_STATE.PC+=4;
                //printf("15//");
                break;
            case 12: //ANDI
                CURRENT_STATE.REGS[inst.r_t.r_i.rt] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] & ((uint32_t)inst.r_t.r_i.r_i.imm & 0x0000ffff) ;
                CURRENT_STATE.PC+=4;
                //printf("16//");
                break;
            case 13: //ORI
                CURRENT_STATE.REGS[inst.r_t.r_i.rt] = CURRENT_STATE.REGS[inst.r_t.r_i.rs] | ((uint32_t)inst.r_t.r_i.r_i.imm & 0x0000ffff) ;
                CURRENT_STATE.PC+=4;
                //printf("17//");
                break;
            case 15: //LUI
                CURRENT_STATE.REGS[inst.r_t.r_i.rt] = ((uint32_t)inst.r_t.r_i.r_i.imm)<<16;
                CURRENT_STATE.PC+=4;
                //printf("18//");
                break;
            case 35: //LW
                CURRENT_STATE.REGS[inst.r_t.r_i.rt] = mem_read_32(CURRENT_STATE.REGS[inst.r_t.r_i.rs] + ((uint32_t)inst.r_t.r_i.r_i.imm));
                CURRENT_STATE.PC+=4;
                //printf("19//");
                break;
            case 43: //SW 
                mem_write_32(CURRENT_STATE.REGS[inst.r_t.r_i.rs] + (uint32_t)inst.r_t.r_i.r_i.imm, CURRENT_STATE.REGS[inst.r_t.r_i.rt]);
                CURRENT_STATE.PC+=4;
                //printf("20//");
                break;
        }
    }
    instruction next = INST_INFO[(CURRENT_STATE.PC-0x400000)/4];
    //if(I.opcode == 0 && I.func_code == 0 && I.r_t.target == 0){
    //printf("value: %d", (int)inst.value);
    if( ((int)inst.value==((text_size/4)-1)) && (int)next.opcode!=2 && (int)next.opcode!=3 && (int)next.opcode!=4 && (int)next.opcode!=5 && ((int)next.opcode != 0 || (int)next.func_code != 8)){//if 지금 고른것이 마지막꺼라면//
        finish = 1;
        RUN_BIT = FALSE;
        //printf("finish is set 1 with value %d  ", (int)inst.value);
    }
}
