#include <stdio.h>
#include <stdlib.h>


typedef unsigned char BIT;
typedef unsigned char WORD;
typedef WORD REGISTER;

typedef WORD OPCODE;
struct Operation{
    OPCODE op;
    WORD operand;
};

typedef struct Operation Operation;


WORD RAM[4096];

REGISTER r0;
REGISTER r1;
REGISTER r2;
REGISTER r3;

REGISTER acc;
REGISTER tmp;
REGISTER pc;
REGISTER ir;


REGISTER mar[2];

BIT ALU_zerobit;

unsigned char running = 0;

// STATS
unsigned int TICKS = 0;



void boot() {
    int i;

    running = 1;

    // == MIMIC computer's initial state ==
    // flush RAM
    for(i = 0; i < 4096; i++) {
        RAM[i] = 0;
    }

    // flush registers
    r0 = 0;
    r1 = 0;
    r2 = 0;
    r3 = 0;
    
    acc = 0;
    tmp = 0;
    pc = 0;
    ir = 0;
    
    ALU_zerobit = 0;

    mar[0] = 0;
    mar[1] = 0;


    // == test PROGRAM - this should be removed ==
    /* if (23 > 10) {
         a = 100;
    } */

    // - PROGRAMME -
    RAM[0] = 0x00; // load
    RAM[1] = 0x0D; // from addr 13 number 10

    RAM[2] = 0x20; // move 10 to r0
    
    RAM[3] = 0x00; // load
    RAM[4] = 0x0E; // from addr 14 number 23;

    RAM[5] = 0x90; // [acc] > [r0]; which is 23 > 10, so zerobit = 1(set)
    
    RAM[6] = 0xC1; // if zerobit is set -> pc+=1, this should jump halt
    
    RAM[7] = 0xD0; // HALT

    RAM[8] = 0x00; // load
    RAM[9] = 0x0F; // from addr 15 number 100
    
    RAM[10] = 0x10; // store
    RAM[11] = 0x10; // to addr 16 number 100

    RAM[12] = 0xD0; // HALT

    // - DATA -
    RAM[13] = 10;
    RAM[14] = 23;
    RAM[15] = 100;

}

void clock_tick() {
    TICKS++;
}

void fetch() {
    // Instruction register loaded with op
    ir = RAM[pc++];
}

const Operation readOp() {
    // read in control unit
    const Operation operation = {
        (ir >> 4) & 0x0F, // mask opcode
        ir & 0x0F         // mask operand
    };
    
    return operation;
}

void execute(const Operation operation) {
    char test[2];
    // TODO
    switch (operation.op)
    {
        // --- MEMORY ---
        case 0x0:
            // load

            // little endian system, we need to store the 2 bytes in reverse order. So, mar[1] first, then mar[2]
            mar[1] = operation.operand; // first 4 bits of the memory address

            clock_tick(); // second read needs clock tick again
            mar[0] = RAM[pc++]; // next memory location has the rest 8 bits of memory addr


            acc = RAM[0x0000FFFF & *(int*)mar]; // load from memory | bitwise-AND clears the garbage first two bytes of typecasted integer
            break;
        case 0x1:
            // store
            
            mar[1] = operation.operand;

            clock_tick();
            mar[0] = RAM[pc++];


            RAM[0x0000FFFF & *(int*)mar] = acc; // store
            break;

        // --- REGISTER ---
        case 0x2:
            // MOVE
            switch (operation.operand)
            {
                case 0x0:
                    r0 = acc;
                    break;
                case 0x1:
                    r1 = acc;
                    break;
                case 0x2:
                    r2 = acc;
                    break;
                case 0x3:
                    r3 = acc;
                    break;
                case 0x4:
                    tmp = acc;
                    break;
            }
            break;

        case 0x3:
            // GET FROM REGISTER
            switch (operation.operand)
            {
                case 0x0:
                    acc = r0;
                    break;
                case 0x1:
                    acc = r1;
                    break;
                case 0x2:
                    acc = r2;
                    break;
                case 0x3:
                    acc = r3;
                    break;
                case 0x4:
                    acc = tmp;
                    break;
            }
            break;
        
        case 0x4:
            // FLUSH
            switch (operation.operand)
            {
                case 0x0:
                    r0 = 0;
                    break;
                case 0x1:
                    r1 = 0;
                    break;
                case 0x2:
                    r2 = 0;
                    break;
                case 0x3:
                    r3 = 0;
                    break;
                case 0x4:
                    tmp = 0;
                    break;
                case 0x5:
                    acc = 0;
                    break;
            }
            break;

        // --- ARITHMETIC ---
        case 0x5:
            // ADD:
            switch (operation.operand)
            {
                case 0x0:
                    acc = acc + r0;
                    break;
                case 0x1:
                    acc = acc + r1;
                    break;
                case 0x2:
                    acc = acc + r2;
                    break;
                case 0x3:
                    acc = acc + r3;
                    break;
            }
            break;
        
        case 0x6:
            // NEGATIVE
            acc = -acc;
            break;
        
        case 0x7:
            // MULTIPLY
            switch (operation.operand)
            {
                case 0x0:
                    acc = acc * r0;
                    break;
                case 0x1:
                    acc = acc * r1;
                    break;
                case 0x2:
                    acc = acc * r2;
                    break;
                case 0x3:
                    acc = acc * r3;
                    break;
                case 0x4:
                    acc = acc * tmp;
                    break; 
            }
            break;

        case 0x8:
            // DIVIDE
            switch (operation.operand)
            {
                case 0x0:
                    acc = acc / r0;
                    break;
                case 0x1:
                    acc = acc / r1;
                    break;
                case 0x2:
                    acc = acc / r2;
                    break;
                case 0x3:
                    acc = acc / r3;
                    break;
                case 0x4:
                    acc = acc / tmp;
                    break; 
            }
            break;

        // --- COMPARISON ---
        case 0x9:
            // Greater than
            switch (operation.operand)
            {
                case 0x0:
                    ALU_zerobit = acc > r0;
                    break;
                case 0x1:
                    ALU_zerobit = acc > r1;
                    break;
                case 0x2:
                    ALU_zerobit = acc > r2;
                    break;
                case 0x3:
                    ALU_zerobit = acc > r3;
                    break;
                case 0x4:
                    ALU_zerobit = acc > tmp;
                    break; 
            }
            break;

        case 0xA:
            // Equal
            switch (operation.operand)
            {
                case 0x0:
                    ALU_zerobit = acc == r0;
                    break;
                case 0x1:
                    ALU_zerobit = acc == r1;
                    break;
                case 0x2:
                    ALU_zerobit = acc == r2;
                    break;
                case 0x3:
                    ALU_zerobit = acc == r3;
                    break;
                case 0x4:
                    ALU_zerobit = acc == tmp;
                    break; 
            }
            break;

        // --- BRANCH ---
        case 0xB:
            // Unconditional jump
            pc += (short) operation.operand;
            break;

        case 0xC:
            // Conditional, "if not-zero" jump
            if(ALU_zerobit) {
                pc += (short) operation.operand;
            }
            break;

        // --- HALT ---
        case 0xD:
            running = 0;
            break;
    }
}


int main() {
    Operation* const operation = (Operation*) malloc(sizeof(Operation));
    
    boot();

    do{
        clock_tick();
        fetch();

        clock_tick();
        *operation = readOp();

        clock_tick();
        execute(*operation);
    }while(running);

    printf("CPU powering off after %d ticks | RESULT IN ACC: %d\n", TICKS, acc);

    printf("Variable 'a' at RAM[16]: %d\n", RAM[16]);

    return 0;
}
