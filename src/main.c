#define SDL_MAIN_HANDLED
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.c"
int main(int argc, char **argv)
{
    //declare all keys
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
    //Create a chip8 struct and struct pointer to use it in functions
    CHIP8 *c, chip8;
    c = &chip8;
    // initialize our chip8 struct
    init(c); 
    if(argc < 2){
        printf("Usage : chip8 <path to file>");
        return 1;
    }

    //load the rom in mem
    loadfile(c, argv[1]);
    
    //initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 )
    {
        fprintf(stdout,"Failed to initialize SDL... Error : %s\n",SDL_GetError());
        return -1;
    }
    
    //create the window
    SDL_Window* window = NULL;
    window = SDL_CreateWindow("Chip8 emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 64*4, 32*4, SDL_WINDOW_SHOWN);
    if (window == NULL){
        printf( "An error occured while creating the window... SDL_Error: %s\n", SDL_GetError());
        exit(2);
    }
    
    //create the renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == NULL)
    {
        printf("An error occured while creating the renderer : %s",SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    //rectangle for drawing
    SDL_Rect rect;
    while(true)
    {
        emulatecycle(c, renderer);
        //Draws on screen if a draw occured
        if(chip8.drawFlag){
            clearRect(&rect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0 ,255);
            SDL_RenderClear(renderer);
            for(int i = 0; i < 64*32; i++){
                if(chip8.graphics[i]){
                    // Draw white pixels
                    int y = (i/64);
                    int x = (i - y * 64);
                    rect.y = y * 4;
                    rect.x = x * 4;
                    rect.w = rect.h = 4;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(renderer, &rect);
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
            //render the drawings to screen
            SDL_RenderPresent(renderer);
            chip8.drawFlag = false;
        }
        SDL_Event event;
        // Check sdl events (keys)
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
                }
                //set pressed keys to one
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 1;
                    }
                }    
                
            }
            //set released keys to zero
            else if(event.type == SDL_KEYUP){
                for (int i = 0; i < 16; i++){
                    if (event.key.keysym.sym == SDL_keys[i]){
                        chip8.key[i] = 0; 
                    }
                }
            }

        }
        SDL_Delay(1);
    }
}
