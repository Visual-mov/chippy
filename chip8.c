#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#define DEBUG_LOG 1
#define DEBUG_CPU 0
#define DEBUG_MEM 0

static void debug_print_mem(chip8_t *c8);

// chip-8 fontset
static const u8 fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip_do_cycle(chip8_t *c8) {
    cpu_state_t *cpu = c8->cpu;
    u16 op = (c8->memory[cpu->PC] << 8) | c8->memory[cpu->PC + 1];

    u16 nnn = op & 0x0FFF,       // lowest 3 bytes
        n  =  op & 0x000F,       // lowest nibble
        x  = (op & 0x0F00) >> 8, // lowest nibble in the high byte
        y  = (op & 0x00F0) >> 4, // highest nibble in the low byte
        kk =  op & 0x00FF;       // lowest byte

    DPRINTF(DEBUG_CPU, "%04X\n", op);

    switch(op & 0xF000) {
        default: CHIP_ERR("Unimplemented opcode\n"); break;
        case 0x0000:
            switch(kk) {
                default: /* SYS is unimplemented, NOP */ break;
                case 0xE0: // CLS
                    memset(c8->vram, 0, DISP_X * DISP_Y);
                    break;
                case 0xEE: // RET
                    cpu->PC = cpu->stack[cpu->SP];
                    cpu->SP--;
                    break;
            }
            break;
        case 0x1000: 
            cpu->PC = nnn;
            break; // JMP (jump)
        case 0x2000: // CALL (call subroutine)
            cpu->stack[++cpu->PC] = cpu->PC;
            cpu->PC = nnn;
            break;
        case 0x3000: // SE (skip if Vx = kk)
            if(cpu->V[x] == kk)
                cpu->PC += 2;
            break;
        case 0x4000: // SNE (skip if Vx != kk)
            if(cpu->V[x] != kk)
                cpu->PC += 2;
            break;
        case 0x5000: // SE (skip if Vx = Vy)
            if(cpu->V[x] == cpu->V[y])
                cpu->PC += 2;
            break;
        case 0x6000: // LD (load kk into Vx)
            cpu->V[x] = kk;
            break;
        case 0x7000: // ADD (add kk to Vx)
            cpu->V[x] += kk;
            break;
        case 0x8000:
            switch(n) {
                case 0x0: // LD (load Vy into Vx)
                    cpu->V[x] = cpu->V[y];
                    break;
                case 0x1: // OR
                    cpu->V[x] |= cpu->V[y];
                    break;
                case 0x2: // AND
                    cpu->V[x] &= cpu->V[y];
                    break;
                case 0x3: // XOR
                    cpu->V[x] ^= cpu->V[y];
                    break;
                case 0x4: // ADD (Vx += Vy, Vf = carry)
                    cpu->V[0xF] = (cpu->V[x] + cpu->V[y] > 0xFF) ? 1 : 0; // unnecessary
                    cpu->V[x] += cpu->V[y];
                    break;
                case 0x5: // SUB (Vx -= Vy, Vf = !borrow)
                    cpu->V[0xF] = (cpu->V[x] > cpu->V[y]) ? 1 : 0;
                    cpu->V[x] -= cpu->V[y];
                    break;
                case 0x6: // SHR (Vf = 1 if lsb of Vx is 1, div Vx by 2)
                    cpu->V[0xF] = cpu->V[x] & 1;
                    cpu->V[x] /= 2;
                    break;
                case 0x7: // SUBN (Vx = Vy - Vx, Vf = !borrow)
                    cpu->V[0xF] = (cpu->V[y] > cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    break;
                case 0xE: // SHL (Vf = 1 if msb of Vx is 1, mult Vx by 2)
                    cpu->V[0xF] = cpu->V[x] >> 7;
                    cpu->V[x] *= 2;
                    break;
            }
            break;
        case 0x9000: // SNE (skip if Vx != Vy)
            if(cpu->V[x] != cpu->V[y])
                cpu->PC += 2;
            break;
        case 0xA000: // LD (I = nnn)
            cpu->I = nnn;
            break;
        case 0xB000: // JMP (PC = V0+nnn)
            cpu->PC = cpu->V[0] + nnn;
            break;
        case 0xC000: // RND
            cpu->V[x] = (rand() % 0xFF) & kk;
            break;
        case 0xD000:
            for(int j = 0; j < n; j++) {
                u8 byte = c8->memory[cpu->I + j];
                for(int i = 0; i < 8; i++) {
                    u8 pixel = (byte >> i) & 1;
                    
                }
            }
            break;
        case 0xE000:
            switch(kk) {
                case 0x9E: // SKP (keyboard skip)
                    if(c8->keys[cpu->V[x]])
                        cpu->PC += 2;
                    break;
                case 0xA1: // SKNP (keyboard not skip)
                    if(c8->keys[cpu->V[x]] == 0)
                        cpu->PC += 2;
                    break;
            }
            break;
        case 0xF000:
            switch(kk) {
                
            }
            break;
    }
    cpu->PC += 2;
}

void chip_draw_display(chip8_t *c8, SDL_Renderer *renderer) {
    u8 y = 0;
    for(int i = 0; i < DISP_X * DISP_Y; i++) {
        if(i % DISP_X == 0 && i != 0)
            y++;
        SDL_Color c = (c8->vram[i] == 1) ? fg_color : bg_color;
        SDL_Rect pixel = {
            .x = i % DISP_X * RES_MULT,
            .y = y * RES_MULT,
            .w = RES_MULT, .h = RES_MULT
        };
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(renderer, &pixel);
    }
}

void chip_input(chip8_t *c8, SDL_Event event) {
    switch(event.key.keysym.sym) {
        case SDLK_1:
            printf("DOWN\n");
            break;
    }
}

chip8_t *chip_init() {
    chip8_t *c8 = (chip8_t*) malloc(sizeof(chip8_t));

    // initialize cpu
    c8->cpu = (cpu_state_t*) malloc(sizeof(cpu_state_t));
    c8->cpu->ST = c8->cpu->DT = 0;
    c8->cpu->SP = 0;
    c8->cpu->PC = PROG_MEM; // PC begins at the first instruction of prog mem
    memset(c8->cpu->V, 0, 16); // init registers
    memset(c8->cpu->stack, 0, 16 * sizeof(u16)); // init stack

    // initialize 4kb ram and load fontset    
    memset(c8->memory, 0, MEM_SIZE);
    memset(c8->vram, 0, DISP_X * DISP_Y);
    for(int i = 0; i < FONTSET_SIZE; i++)
        c8->memory[i] = fontset[i];

    time_t t;
    srand((unsigned int) time(&t));

    return c8;
}

void chip_free(chip8_t *c8) {
    free(c8->cpu);
    free(c8);
}

void load_rom(chip8_t *c8, char *path) {
    FILE *fp;
    int i = 0;
    if(c8 == NULL)
        return;
    if((fp = fopen(path, "r")) == NULL) {
        CHIP_ERR("Failed to open file.\n");
    }
    DPRINTF(DEBUG_LOG, "Loading ROM from %s ...\n", path);
    do {
        fread(&(c8->memory[PROG_MEM + (i++)]), 1, 1, fp);
    } while(!feof(fp) && PROG_MEM + i < MEM_SIZE);
    DPRINTF(DEBUG_LOG, "Loaded ROM of size 0x%03X. range: (0x%03X - 0x%03X)\n", i, PROG_MEM, PROG_MEM+i);
    fclose(fp);
}

// ------------ debug ------------- 

static void debug_print_memory(chip8_t *c8) {
    for(u16 i = 0; i < MEM_SIZE; i++) {
        if(i % 16 == 0) {
            if(i > 0x200 && i <= 0x210) printf("<- PROG_MEM");
            if(i > 0x000 && i <= 0x010) printf("<- FONTSET");
            printf("\n0x%03X | ", i);
        }
        printf("%02X ", c8->memory[i]);
    }
    printf("\n");
}

static void debug_print_cpu(chip8_t *c8) {

}