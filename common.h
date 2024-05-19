#pragma once
#include <stdint.h>

#define CHIP_ERR(msg) printf("error: %s", msg); exit(1)
#define DPRINTF(DBG_FLAG, ...) if(DBG_FLAG) { printf("chippy: "); printf(__VA_ARGS__); }

#define RES_MULT    12
#define DISP_X      64
#define DISP_Y      32
#define WIN_WIDTH   RES_MULT * DISP_X
#define WIN_HEIGHT  RES_MULT * DISP_Y

typedef uint8_t  u8;
typedef uint16_t u16;