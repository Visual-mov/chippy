/* chippy.c 
 *  Chip-8 interpreter written in C
 *  visual_mov 2024
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "chippy.h"
#include "chip8.h"
#include "common.h" 

int main(int argc, char *argv[]) {
    chip8_t *c8 = chip_init();
    load_rom(c8, "./roms/test/programs/Chip8 emulator Logo [Garstyciuks].ch8");

    // printf("%02X %02X\n", c8->memory[0x200], c8->memory[0x201]);
    //printf("%04X\n", (c8->memory[0x200] << 8) | c8->memory[0x201]);
    //c8_do_cycle((c8->memory[0x200] << 8) | c8->memory[0x201]);

    // set up SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        CHIP_ERR("SDL initialization failed.");
    }

    SDL_Window *window = SDL_CreateWindow(
        "chippy",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_WIDTH, WIN_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
    );

    if(!window) {
        CHIP_ERR("Could not create SDL window");
    }
    if(!renderer) {
        CHIP_ERR("Could not create SDL renderer");
    }

    SDL_Event event;
    int ticks = 0;
    u8 quit = 0;

    while(!quit) {
        ticks = SDL_GetTicks();

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: quit = 1; break;
                case SDL_KEYDOWN: chip_input(c8, event); break;
            }
        }
        
        SDL_RenderClear(renderer);
        chip_do_cycle(c8);
        chip_draw_display(c8, renderer);
        SDL_RenderPresent(renderer);

        // Cap to 60 FPS
        int delta = SDL_GetTicks() - ticks;
        if(delta < 1000 / FPS_CAP) {
            SDL_Delay((1000 / FPS_CAP) - delta);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    chip_free(c8);

    return 0;
}



