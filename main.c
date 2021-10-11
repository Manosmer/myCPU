#include <stdio.h>
#include <stdlib.h>


typedef unsigned char WORD;
typedef WORD REGISTER;

typedef WORD OPCODE;
struct Operation{
    OPCODE op;
    WORD operand;
};


WORD RAM[4096];

REGISTER r0;
REGISTER r1;
REGISTER r2;
REGISTER r3;

REGISTER acc;
REGISTER tmp;
REGISTER pc;
REGISTER ir;

REGISTER zbr;

REGISTER mar[2];


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
    
    zbr = 0;

    mar[0] = 0;
    mar[1] = 0;


    // == test PROGRAM - this should be removed ==
    RAM[0] = 0x00;  // load 
    RAM[1] = 0x0C;  // from address 12 (num 34)
    
    RAM[2] = 0x20;  // move acc to r0
    
    RAM[3] = 0x00;  // load
    RAM[4] = 0x0D;  // from address 13 (num 6)

    RAM[5] = 0x50;  // acc + r0

    RAM[6] = 0x10;  // store acc
    RAM[7] = 0x0E;  // to addr 14

    RAM[8] = 0x45;  // FLUSH acc

    RAM[9] = 0x00;  // LOAD
    RAM[10] = 0x0E; // from address 14 (num 40)

    RAM[11] = 0xD0; // halt

    RAM[12] = (WORD) 34;
    RAM[13] = (WORD)  6;

    // result in the acc must be 40
}

void clock_tick() {
    TICKS++;
}

void fetch() {
    // Instruction registers loaded with op
    ir = RAM[pc++];
}

const struct Operation readOp() {
    // read in control unit
    const struct Operation operation = {
        (ir >> 4) & 0x0F, // mask opcode
        ir & 0x0F         // mask operand
    };
    
    return operation;
}

void execute(const struct Operation operation) {
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

        // --- HALT ---
        case 0xD:
            running = 0;
            break;
    }
}


int main() {
    struct Operation* const operation = (struct Operation*) malloc(sizeof(struct Operation));
    
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

    return 0;
}
