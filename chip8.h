#pragma once
#include <stdint.h>
#include "common.h"

// default clockspeed is 600hz
#define CLOCK = 600

#define MEM_SIZE 0xFFF
#define FONTSET_SIZE 0x080
#define PROG_MEM 0x200

// instruction decoding
// #define NNN(op)  op & 0x0FFF        
// #define   N(op)  op & 0x000F        
// #define   X(op) (op & 0x0F00) >> 8  
// #define   Y(op) (op & 0x00F0) >> 4  
// #define  KK(op)  op & 0x00FF        

static const SDL_Color fg_color = {255, 255, 255};
static const SDL_Color bg_color = {0, 0, 0};

typedef struct cpu_state_t {
    u8 V[16], I;   // V0-VE: general purpose registers, VF: flag register, I: memory address register 
    u16 PC;        // program counter
    u16 ST, DT;    // sound timer, delay timer.
    u16 stack[16]; // stack and stack pointer
    u8 SP;
} cpu_state_t;

typedef struct chip8_t {
    cpu_state_t *cpu;
    u8 memory[MEM_SIZE];
    u8 vram[DISP_X * DISP_Y];
    u8 keys[15];
} chip8_t;

chip8_t *chip_init();
void chip_do_cycle(chip8_t *c8);
void chip_draw_display(chip8_t *c8, SDL_Renderer *renderer);
void chip_input(chip8_t *c8, SDL_Event event);
void chip_free(chip8_t *c8);

void load_rom(chip8_t *c8, char *path);