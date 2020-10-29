#define SDL_MAIN_HANDLED
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.c"
#define W_HEIGHT 32
#define W_WIDTH 64
int main(int argc, char **argv)
{
    uint8_t SDL_keys[16] ={
    SDLK_a,
    SDLK_z,
    SDLK_e,
    SDLK_q,
    SDLK_s,
    SDLK_d,
    SDLK_w,
    SDLK_x,
    SDLK_c,
    SDLK_r,
    SDLK_t,
    SDLK_y,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_v,
    };
    CHIP8 *c, chip8;
    c = &chip8;
    init(c);
    if(argc < 2){
        printf("Usage : chip8 <path to file>");
        return 1;
    }
    loadfile(c, argv[1]);
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        fprintf(stdout,"Failed to initialize SDL... Error : %s\n",SDL_GetError());
        return -1;
    }
    SDL_Window* window = NULL;
    window = SDL_CreateWindow("Chip8 emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 128*2, 64*2, SDL_WINDOW_SHOWN);
    if (window == NULL){
        printf( "An error occured while creating the window... SDL_Error: %s\n", SDL_GetError());
        exit(2);
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL)
    {
    printf("An error occured while creating the renderer : %s",SDL_GetError());
    return EXIT_FAILURE;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    printf("Done");
    SDL_Rect rect;
    while(true)
    {
        emulatecycle(c, renderer);
        if(chip8.drawFlag){
            for(int i = 0; i != 64; i++){
                for(int a = 0; a != 32; a++){
                    if(chip8.graphics[i][a]){
                        rect.x = i*4;
                        rect.y = a*4;
                        rect.w = rect.h = 4;
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                        SDL_RenderFillRect(renderer, &rect);
                    }
                    else{
                        rect.x = i*4;
                        rect.y = a*4;
                        rect.w = rect.h = 4;
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
            SDL_RenderPresent(renderer);
            chip8.drawFlag = false;
        }
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT)
                exit(0);
            else if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            printf("Quitting...\n");
                            SDL_Delay(100);
                            exit(0);
                            break;
                        default:
                            break;
                }
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 1;
                    }
                }    
                
            }
            else if(event.type == SDL_KEYUP){
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 0;
                    }
                }
            }

        }
    }
}
