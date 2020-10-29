#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
typedef struct CHIP8{
    uint16_t stack[16];
    uint8_t memory[4096]; // chip 8 has 4kb of mem
    uint8_t V[0xF];
    uint16_t I;
    uint16_t pc;
    uint16_t opcode;
    int sp;
    uint8_t graphics[64][32];
    uint8_t key[16];
    uint8_t delay_t; //delay timer
    uint8_t sound_t; // sound timer
    bool drawFlag;
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
    memset(c->graphics, 0, (64*32)*sizeof(uint8_t));
    memset(c->memory, 0, 4096*sizeof(uint8_t));
    memset(c->stack, 0, 16*sizeof(uint8_t));
    memset(c->key, 0, 16*sizeof(uint8_t));
    memset(c->V, 0, 16*sizeof(uint8_t));
    for(int i = 0; i < 80; i++){
        c->memory[i] = font[i]; //load font in mem
    }
    c->pc = 0x200;
    c->sp = -1;
    c->I = c->opcode = c->keyPressed = c->drawFlag = c->delay_t = c->sound_t = 0;
    srandom(time(NULL));
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
    fread(c->memory+0x200, size, 1, f);
    fclose(f);
}

void emulatecycle(CHIP8 *c, SDL_Renderer* renderer)
{
    c->opcode = c->memory[c->pc] << 8 | c->memory[c->pc + 1];
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
                memset(c->graphics, 0, (64*32)*sizeof(uint8_t));
                c->drawFlag = true;
                c->pc += 2;
                break;
            case 0x000E:
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
            c->pc = nnn;
            break;

        case 0x2000:
            c->sp++;
            c->stack[c->sp] = c->pc;
            c->pc = nnn;
            break;
        case 0x3000:
            if(c->V[x] == kk)
                c->pc += 2;
            c->pc+=2;
            break;
        case 0x4000:
            if(c->V[x] != kk)
                c->pc += 4;
            else
                c->pc += 2;
            break;
        case 0x5000:
            if(c->V[x] == c->V[y])
                c->pc += 2;
            c->pc += 2;
            break;
        case 0x6000:
            c->V[x] = kk;
            c->pc += 2;
            break;
        case 0x7000:
            c->V[x] += kk;
            c->pc += 2;
            break;
        case 0x8000:
            switch (c->opcode & 0x000F)
            {
                case 0x0:
                    c->V[x] = c->V[y];
                    c->pc += 2;
                    break;
                case 0x1:
                    c->V[x] |= c->V[y];
                    c->pc += 2;
                    break;
                case 0x2:
                    c->V[x] &= c->V[y];
                    c->pc += 2;
                    break;
                case 0x3:
                    c->V[x] ^= c->V[y];
                    c->pc += 2;
                    break;
                case 0x4:
                    c->V[0xF] = ((int)c->V[x] + (int)c->V[y]) > 255 ? 1 : 0;
                    c->V[x] += c->V[y];
                    c->pc += 2;
                    break;
                case 0x5:
                    c->V[0xF] = (c->V[x] > c->V[y]) ? 1 : 0;
                    c->V[x] -= c->V[y];
                    c->pc += 2;
                    break;
                case 0x6:
                    c->V[0xF] = c->V[x] & 1;
                    c->V[x] >>= 1;
                    c->pc += 2;
                    break;
                case 0x7:
                    c->V[0xF] = c->V[y] > c->V[x] ? 1 : 0;
                    c->V[x] = c->V[y] - c->V[x];
                    c->pc += 2;
                    break;
                case 0xE:
                    c->V[0xF] = (c->V[x] & 0x80) >> 7; // 0x80 = 0b10000000
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
            if(c->V[x] != c->V[y]){
                c->pc += 2;
            }
            c->pc +=2;
            break;
        case 0xA000:
            c->I = nnn;
            c->pc += 2;
            break;
        case 0xB000:
            c->pc = nnn + c->V[0];
            break;
        case 0xC000:{
            uint8_t rand = random();
            c->V[x] = rand & kk;
            c->pc += 2;
            break;
        }
        case 0xD000:
        {
            uint8_t row;
            c->V[0xF] = 0;
            for(int i = 0; i != n; i++){
                row = c->memory[c->I+i];
                for(int a = 0; a != 8; a++){
                    int pixel = row & (0x80 >> a);
                    if(pixel && c->V[y]+i <= 64*2 && c->V[x]+a <= 32*2){
                        c->V[0xF] = (c->graphics[c->V[x]+a][c->V[y]+i] > 0) ? 1 : 0;
                        c->graphics[c->V[x]+a][c->V[y]+i] ^= 1;
                        c->drawFlag = true;
                    }
                    
                }
            }
            c->pc += 2;
            break;
        }
        case 0xE000:
            switch(c->opcode & 0x000F){
                case 0xE:
                    if(c->key[c->V[x]]){
                        c->pc += 2;
                    }
                    c->pc += 2;
                    break;
                case 0x1:
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
                            c->V[x] = c->delay_t;
                            c->pc += 2;
                            break;
                        case 0xA:
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
                        c->delay_t = c->V[x];
                        c->pc += 2;
                        break;
                    case 0x8:
                        c->sound_t = c->V[x];
                        c->pc += 2; 
                        break;
                    case 0xE:
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
                    c->I = c->V[x] * 5;
                    c->pc += 2;
                    break;
                case 3:
                    for(int i = 2; i != -1; i--){
                        c->memory[c->I+i] = num%10;
                        num /= 10;
                    }
                    c->pc += 2;
                    break;
                case 5:
                    for(int i = 0; i <= x; i++){
                        c->memory[c->I+i] = c->V[i];
                    }
                    c->pc += 2;
                    break;
                case 6:
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
    if(c->delay_t)
        c->delay_t--;
}
