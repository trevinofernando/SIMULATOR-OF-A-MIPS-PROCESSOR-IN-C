#include <stdio.h>
#include <stdlib.h>
#include "spimcore.h"

/* ALU */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero){
    // Switch to control which ALU control gets executed
	switch ((int) ALUControl)
	{
        case 0: //000://ADD
            *ALUresult = A + B;
            break;

        case 1: //001://Subtract
            *ALUresult = A - B;
            break;

        case 2: //010://if A < B, Z = 1; otherwise, Z = 0
            if ((signed)A < (signed)B)
            {
                *ALUresult = 1;
            }
            else
            {
                *ALUresult = 0;
            }
            break;

        case 3: //011://if A < B, Z = 1; otherwise, Z = 0 (A and B are unsigned integers)
            if (A < B)
            {
                *ALUresult = 1;
            }
            else
            {
                *ALUresult = 0;
            }
            break;

        case 4: //100://bitwise AND
            *ALUresult = A & B;
            break;

        case 5: //101://bitwise OR

            *ALUresult = A | B;
            break;


        case 6: //110://Shift left B by 16 bits
            *ALUresult = B << 16;
            break;

        case 7: //111://bitwise NOT
            *ALUresult = ~A;
            break;
	}
	if(*ALUresult == 0){
		*Zero = 1;//True Zero
	}
	else{
		*Zero = 0;//Not Zero
	}
}

/* instruction fetch */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction){
    //Check the proper part for the memory index
	unsigned MemoryIndex = PC >> 2; //right shift 2 to divide by 4
	//Check if it in proper word-aligned format (multiple of 4)
    if (PC % 4 == 0) {
        *instruction = Mem[MemoryIndex];
        return 0;
    }
    else{//If not Halt
	return 1;
    }
}

/* instruction partition */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec){
	unsigned rPartition		    = 0x1f; //5 bits total
	unsigned functopPartition	= 0x0000003f; //6 bits total
	unsigned offsetPartition	= 0x0000ffff; //16 bits total
	unsigned jsecPartition	    = 0x03ffffff; //26 bits total

	// shift to right align and then bit-masking the right amount
	*op		= (instruction >> 26)   & functopPartition;	// instruction [31-26] #need 6 bits
	*r1		= (instruction >> 21)   & rPartition;       // instruction [25-21] #need 5 bits
	*r2		= (instruction >> 16)   & rPartition;       // instruction [20-16] #need 5 bits
	*r3		= (instruction >> 11)   & rPartition;       // instruction [15-11] #need 5 bits
	*funct	= instruction           & functopPartition; // instruction [5-0]   #need 6 bits
	*offset	= instruction           & offsetPartition;  // instruction [15-0]  #need 16 bits
	*jsec	= instruction           & jsecPartition;    // instruction [25-0]  #need 26 bits
}

/* instruction decode */
int instruction_decode(unsigned op,struct_controls *controls){
        switch(op){
		//R Type
		case 0: //000 000
		    controls->RegDst = 1;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 7;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 1;
		    break;
		//Jump
		case 2:  //j 000 010
		    controls->RegDst = 0;
		    controls->Jump = 1;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 0;
		    break;
		//Branch if equal
		case 4:  //beq 000 100
		    controls->RegDst = 2;
		    controls->Jump = 0;
		    controls->Branch = 1;
		    controls->MemRead = 0;
		    controls->MemtoReg = 2;
		    controls->ALUOp = 1;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 0;
		    controls->RegWrite = 0;
		    break;
		//Add immediate
		case 8:   // addi 001 000
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    break;
		//Set less than immediate
		case 10:  //slti 010 010
		    controls->RegDst = 0;//1
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 2;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    break;
		 //Set less than immediate unsigned
		case 11:  //sltiu 001 011
		    controls->RegDst = 0;//1
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 3;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;//0
		    controls->RegWrite = 1;
		    break;
		//load upper imediate
		case 15: //lui 001 111
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 0;
		    controls->ALUOp = 6;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    break;
		 //Load word
		case 35:  //lw 100 011
		    controls->RegDst = 0;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 1;
		    controls->MemtoReg = 1;
		    controls->ALUOp = 0;
		    controls->MemWrite = 0;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 1;
		    break;
		//Store word
		case 43:  //sw 101 011
		    controls->RegDst = 2;
		    controls->Jump = 0;
		    controls->Branch = 0;
		    controls->MemRead = 0;
		    controls->MemtoReg = 2;
		    controls->ALUOp = 0;
		    controls->MemWrite = 1;
		    controls->ALUSrc = 1;
		    controls->RegWrite = 0;
		    break;

		default://Return 1 if Halt
		    return 1;

	     	}
       		return 0;
}

/* Read Register */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2){
    //  Read the registers addressed by r1 and r2 from Reg, and write the read values to data1 and data2 respectively
	*data1 = Reg[r1];
	*data2 = Reg[r2];
}

/* Sign Extend */
void sign_extend(unsigned offset,unsigned *extended_value){
    //If negative sign extend for negative
	unsigned extend1s = 0xFFFF0000;
	unsigned Negative = offset >> 15;

    if (Negative == 1)
         *extended_value = offset | extend1s;


    //Otherwise sign extend normally
    else
         *extended_value = offset & 0x0000ffff;
    return;
}

/* ALU operations */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero){
   	//Check which data we are opperating on by ALU src
	if(ALUSrc == 1){
		data2 = extended_value;
	}

	//All ALUOps just send instructions to ALU which updates ALU result
	//ALUOP 7 is R type insturction which requires use of funct
	if(ALUOp == 7){
		//Find the proper ALUOp for each R type instruction
		switch(funct) {

			//Add
			case 32:
					ALUOp = 0;
					break;
			//Sub
			case 34:
					ALUOp = 1;
					break;
			//Set Less Signed
			case 42:
					ALUOp = 2;
					break;
			//Set Less Unsigned
			case 43:
					ALUOp = 3;
					break;
			//And
			case 36:
					ALUOp = 4;
					break;
			//Or
			case 37:
					ALUOp = 5;
					break;
			//Shift Left extended value 16
			case 6:
					ALUOp = 6;
					break;
			//Nor
			case 39:
					ALUOp = 7;
					break;
			//Halt not proper funct
			default:
					return 1;

		}
		//Send to ALU for funct
		ALU(data1,data2,ALUOp,ALUresult,Zero);

	}


	else{
	//Send to ALU for non funct
	ALU(data1,data2,ALUOp,ALUresult,Zero);
	}

	//Return
	return 0;
}

/* Read / Write Memory */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem){
    //if reading from memory
	if (MemRead == 1) {
		if((ALUresult % 4) == 0){
			*memdata = Mem[ALUresult >> 2];
		}

		//Improper Address Halt
		else{
			return 1;
		}

	}

	//If writting to memory
	if (MemWrite == 1) {
		if((ALUresult % 4) == 0){
			Mem[ALUresult >> 2] = data2;
		}
		//Improper Address Halt
		else{
			return 1;
		}
	}

             return 0;
}

/* Write Register */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg){
    //Check if writing
	if(RegWrite == 1){
		 // If Mem to Register
		 if (MemtoReg == 1 && RegDst == 0) {
			Reg[r2] = memdata;
		 }

		 //If Mem to Register but r3

		 else if(MemtoReg == 1 && RegDst == 1){
			 Reg[r3] = memdata;
		 }

		 // If Result to Register
		 else if (MemtoReg == 0 && RegDst == 0) {
			Reg[r2] = ALUresult;
		 }

		 // If Result to Register but next value
		 else if (MemtoReg == 0 && RegDst == 1){
			Reg[r3] = ALUresult;
		 }
	}
}

/* PC update */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC){
    // increment the PC by 4 always
	*PC += 4;

	//If branching and we got a zero properly add extended value
	if(Branch == 1 && Zero == 1){
		*PC += extended_value << 2;
	}

	// If Jumping shift Instruction and combine with PC
	if(Jump == 1){
		*PC = (jsec << 2) | (*PC & 0xf0000000);
	}
}
