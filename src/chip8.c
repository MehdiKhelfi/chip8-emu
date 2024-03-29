#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
//Our Chip8 struct
typedef struct CHIP8{
    // The chip 8 can store up to 16 addresses (for calls)
    uint16_t stack[16];
    // chip 8 has 4kb of mem
    uint8_t memory[4096];
    //14 all purpose 8bit registers and one "Flag" register (for borrow, carry, collisions ...)
    uint8_t V[0xF];
    //I register to store 12bit addresses
    uint16_t I;
    //Program counter, indicates which instruction should be executed
    uint16_t pc;
    //Variable where we store the current opcode
    uint16_t opcode;
    //Stack pointer, points to the top of the stack
    int sp;
    //2d array for graphics (64 * 32 = 2048 of vram)
    uint8_t graphics[64*32];
    //pressed keys
    uint8_t key[16];
    //delay timer, decrements every cycle
    uint8_t delay_t;
    //sound timer, beeps when non zero (TODO)
    uint8_t sound_t;
    //If it's true, the program draw on screen 
    bool drawFlag;
    //True if a key is pressed
    bool keyPressed;
} CHIP8;

unsigned char font[80] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};
void init(CHIP8 *c)
{
    //Filling all arrays with zeroes
    memset(c->graphics, 0, (64*32)*sizeof(uint8_t));
    memset(c->memory, 0, 4096*sizeof(uint8_t));
    memset(c->stack, 0, 16*sizeof(uint8_t));
    memset(c->key, 0, 16*sizeof(uint8_t));
    memset(c->V, 0, 16*sizeof(uint8_t));
    //load font in mem
    for(int i = 0; i < 80; i++){
        c->memory[i] = font[i];
    }
    //chip 8 programs start at 0x200 in mem
    c->pc = 0x200;
    c->sp = -1;
    c->I = c->opcode = c->keyPressed = c->drawFlag = c->delay_t = c->sound_t = 0;
    //seed the random function
    srand(time(NULL));
}

int loadfile(CHIP8 *c, const char *argv)
{
    FILE *f;
    f = fopen(argv, "rb");
    fseek(f, 0L, SEEK_END); 
    int size = ftell(f);
    if(size > 4096 - 512){
        printf("The rom file is too big...");
        return 1;
    }
    rewind(f);
    // loads the program at 0x200 in mem
    fread(c->memory+0x200, size, 1, f);
    fclose(f);
}
//Function to clear the rectangle before each use
void clearRect(SDL_Rect *rect){
    rect->h = rect->w = rect->x = rect->y = 0;
}

void emulatecycle(CHIP8 *c, SDL_Renderer* renderer)
{
    // merge the bytes into a single, two bytes opcode variable
    c->opcode = c->memory[c->pc] << 8 | c->memory[c->pc + 1];
    //declare commonly used values in opcodes
    uint8_t x = (c->opcode & 0x0F00) >> 8;
    uint8_t y = (c->opcode & 0x00F0) >> 4;
    uint8_t n = (c->opcode & 0x000F);
    uint16_t nnn = (c->opcode & 0x0FFF);
    uint8_t kk = c->opcode & 0x00FF;
    int num = c->V[x];
    switch (c->opcode & 0xF000)
    {
        case 0x0000:
            switch (c->opcode & 0x000F)
            {
            case 0x0000:
                // CLS, simply clears the screen (so the graphics buffer here)
                memset(c->graphics, 0, (64*32)*sizeof(uint8_t));
                c->drawFlag = true;
                c->pc += 2;
                break;
            case 0x000E:
                // RET, pops address from stack and loads it into pc
                c->pc = c->stack[c->sp];
                c->sp--;
                c->pc += 2;
                break;
            default:
                printf("Unknown opcode %#X", c->opcode);
                exit(1);
                break;
            }
            break;

        case 0x1000:
            //JP NNN, jumps to the address NNN
            c->pc = nnn;
            break;

        case 0x2000:
            //CALL NNN, pushes pc in the stack and jumps to the subroutine at NNN
            c->sp++;
            c->stack[c->sp] = c->pc;
            c->pc = nnn;
            break;
        case 0x3000:
            //Skips the next instruction if VX equals NN.
            if(c->V[x] == kk)
                c->pc += 2;
            c->pc+=2;
            break;
        case 0x4000:
            //Skips the next instruction if VX doesn't equal NN.
            if(c->V[x] != kk)
                c->pc += 4;
            else
                c->pc += 2;
            break;
        case 0x5000:
            //Skips the next instruction if VX equals VY.
            if(c->V[x] == c->V[y])
                c->pc += 2;
            c->pc += 2;
            break;
        case 0x6000:
            //Sets VX to NN.
            c->V[x] = kk;
            c->pc += 2;
            break;
        case 0x7000:
            //Adds NN to VX. (Carry flag is not changed)
            c->V[x] += kk;
            c->pc += 2;
            break;
        case 0x8000:
            switch (c->opcode & 0x000F)
            {
                case 0x0:
                    //Sets VX to the value of VY.
                    c->V[x] = c->V[y];
                    c->pc += 2;
                    break;
                case 0x1:
                    //Sets VX to VX OR VY. (Bitwise OR operation)
                    c->V[x] |= c->V[y];
                    c->pc += 2;
                    break;
                case 0x2:
                    //Sets VX to VX and VY. (Bitwise AND operation)
                    c->V[x] &= c->V[y];
                    c->pc += 2;
                    break;
                case 0x3:
                    //Sets VX to VX xor VY.
                    c->V[x] ^= c->V[y];
                    c->pc += 2;
                    break;
                case 0x4:
                    //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
                    c->V[0xF] = ((int)c->V[x] + (int)c->V[y]) > 255 ? 1 : 0;
                    c->V[x] += c->V[y];
                    c->pc += 2;
                    break;
                case 0x5:
                    //VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    c->V[0xF] = (c->V[x] > c->V[y]) ? 1 : 0;
                    c->V[x] -= c->V[y];
                    c->pc += 2;
                    break;
                case 0x6:
                    //Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                    c->V[0xF] = c->V[x] & 1;
                    c->V[x] >>= 1;
                    c->pc += 2;
                    break;
                case 0x7:
                    //Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
                    c->V[0xF] = c->V[y] > c->V[x] ? 1 : 0;
                    c->V[x] = c->V[y] - c->V[x];
                    c->pc += 2;
                    break;
                case 0xE:
                    //Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
                    c->V[0xF] = (c->V[x] & 0x80) >> 7;
                    c->V[x] <<= 1;
                    c->pc += 2;
                    break;
                default:
                    printf("Unknown opcode %#X", c->opcode);
                    exit(1);
                    break;
            }
            break;
        case 0x9000:
            // Skips the next instruction if VX doesn't equal VY.
            if(c->V[x] != c->V[y]){
                c->pc += 2;
            }
            c->pc +=2;
            break;
        case 0xA000:
            // Sets I to the address NNN.
            c->I = nnn;
            c->pc += 2;
            break;
        case 0xB000:
            // Jumps to the address NNN plus V0.
            c->pc = nnn + c->V[0];
            break;
        case 0xC000:{ 
            // Sets VX to the result of a bitwise AND operation on a random number and NN.
            uint8_t random = rand();
            c->V[x] = random & kk;
            c->pc += 2;
            break;
        }
        case 0xD000:
        {
            /* Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
               Each row of 8 pixels is read as bit-coded starting from memory location I
               set VF to one if a bit is flipped from set to unset (collision basically) */
            uint8_t row;
            c->V[0xF] = 0;
            for(int ysprite = 0; ysprite < n; ysprite++){
                row = c->memory[c->I+ysprite];
                for(int xsprite = 0; xsprite < 8; xsprite++){
                    if(row & (0x80 >> xsprite)){
                        if(c->graphics[(xsprite + c->V[x]) + ((ysprite + c->V[y]) * 64)] != 0){
                            c->V[0xF] = 1;
                        }
                        c->graphics[(xsprite + c->V[x]) + ((ysprite + c->V[y]) * 64)] ^= 1;
                    }
                }
            }
            c->drawFlag = true;
            c->pc += 2;
            break;
        }
        case 0xE000:
            switch(c->opcode & 0x000F){
                case 0xE:
                    //Skips the next instruction if the key stored in VX is pressed.
                    if(c->key[c->V[x]]){
                        c->pc += 2;
                    }
                    c->pc += 2;
                    break;
                case 0x1: 
                    //Skips the next instruction if the key stored in VX isn't pressed.
                    if(c->key[c->V[x]] == 0){
                        c->pc += 2;
                    }
                    c->pc += 2;
                    break;
            }
            break;
        case 0xF000:
            switch ((c->opcode & 0x00F0) >> 4)
            {
                case 0:
                    switch (c->opcode & 0x000F)
                    {
                        case 0x7:
                            // Sets VX to the value of the delay timer.
                            c->V[x] = c->delay_t;
                            c->pc += 2;
                            break;
                        case 0xA:
                            //A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                            c->keyPressed = false;
                            for(int i = 0; i != 16; i++){
                                if(c->key[i]){
                                    c->V[x] = i;
                                    c->keyPressed = true;
                                }
                                else if(!c->keyPressed)
                                    return;
                            }
                            c->pc += 2;
                            break;
                        default:
                            printf("Unknown opcode %#X", c->opcode);
                            exit(1);
                            break;
                    }
                    break;
                case 1:
                    switch (c->opcode & 0x000F)
                    {
                    case 0x5:
                        // Sets the delay timer to VX.
                        c->delay_t = c->V[x];
                        c->pc += 2;
                        break;
                    case 0x8:
                        // Sets the sound timer to VX.
                        c->sound_t = c->V[x];
                        c->pc += 2; 
                        break;
                    case 0xE:
                        // Adds VX to I. VF is not affected
                        c->I += c->V[x];
                        c->V[0xF] = c->I > 0x0FFF ? 1 : 0;
                        c->pc += 2;
                        break;
                    default:
                        printf("Unknown opcode %#X", c->opcode);
                        exit(1);
                        break;
                    }
                   break;
                case 2:
                    // Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                    c->I = c->V[x] * 5;
                    c->pc += 2;
                    break;
                case 3:
                    /* Stores the binary-coded decimal representation of VX
                    with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2.
                    (place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.) */
                    for(int i = 2; i != -1; i--){
                        c->memory[c->I+i] = num%10;
                        num /= 10;
                    }
                    c->pc += 2;
                    break;
                case 5:
                    //Stores V0 to VX (including VX) in memory starting at address I.
                    for(int i = 0; i <= x; i++){
                        c->memory[c->I+i] = c->V[i];
                    }
                    c->pc += 2;
                    break;
                case 6:
                    /* Fills V0 to VX (including VX) with values from memory starting at address I.
                    I is then set to I + X + 1 */
                    for(int i = 0; i <= x; i++){
                        c->V[i] = c->memory[c->I+i];
                    }
                    c->I += x+1;
                    c->pc += 2;
                    break;
                default:
                    printf("Unknown opcode %#X", c->opcode);
                    exit(1);
                    break;
            }
            break;
        default:
            printf("Unknown opcode %#X", c->opcode);
            exit(1);
            break;
    }
    //decrement delay timer if it's non-zero
    if(c->delay_t)
        c->delay_t--;
}
