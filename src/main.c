#define SDL_MAIN_HANDLED
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.c"
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
    }; //defines all keys
    //Create a chip8 struct and struct pointer to use it in functions
    CHIP8 *c, chip8;
    c = &chip8;
    init(c); // initialize our chip8 struct 
    if(argc < 2){
        printf("Usage : chip8 <path to file>");
        return 1;
    }
    loadfile(c, argv[1]); //load the rom gave as an argument in mem
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        fprintf(stdout,"Failed to initialize SDL... Error : %s\n",SDL_GetError());
        return -1;
    }
    SDL_Window* window = NULL;
    window = SDL_CreateWindow("Chip8 emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*4, 32*4, SDL_WINDOW_SHOWN); //create the window with 64*4 by 32*4 res
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
    SDL_RenderClear(renderer); //fill the window in black
    SDL_Rect rect; //declare rect for drawing
    while(true)
    {
        emulatecycle(c, renderer);
        if(chip8.drawFlag){
            for(int i = 0; i != 64; i++){
                for(int a = 0; a != 32; a++){ //loop through the graphics buffer to draw pixel
                    if(chip8.graphics[i][a]){
                        rect.x = i*4;
                        rect.y = a*4; //set the coordinates of the rectangle
                        rect.w = rect.h = 4; //set the size of the rectangle to 4 which is the scaling factor used in this program (the window is 64 * 4 and 32 * 4)
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                        SDL_RenderFillRect(renderer, &rect); // draw white rects
                    }
                    else{
                        rect.x = i*4;
                        rect.y = a*4;
                        rect.w = rect.h = 4;
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        SDL_RenderDrawRect(renderer, &rect);
                        SDL_RenderFillRect(renderer, &rect); //draw black rects
                    }
                }
            }
            SDL_RenderPresent(renderer); //render the drawings to screen
            chip8.drawFlag = false;// set the drawflag to false back
        }
        SDL_Event event;
        while(SDL_PollEvent(&event)){ // Check sdl events (keys)
            if(event.type == SDL_QUIT)
                exit(0);
            else if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            printf("Quitting...\n");
                            SDL_Delay(100);
                            exit(0);
                            break;
                }
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 1;//set pressed keys to one
                    }
                }    
                
            }
            else if(event.type == SDL_KEYUP){
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 0; //set released keys to zero
                    }
                }
            }

        }
    }
}
