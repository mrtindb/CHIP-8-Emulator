#ifndef CHIP8EMULATOR_INSTRUCTIONS_H
#define CHIP8EMULATOR_INSTRUCTIONS_H
#include<SDL.h>

//Functions, that represent some of the more complicated CHIP-8 instructions.
u_int16_t fetch(u_int8_t *mem, u_int16_t *pc);


int CLS(u_int8_t (*display_array)[64]);
int RET(u_int16_t *pc,u_int16_t *stk, u_int8_t *sp);
int JUMP(u_int16_t N, u_int16_t *pc);

///2nnn
int CALL(u_int16_t N, u_int16_t *pc, u_int8_t *sp, u_int16_t *stk);
int SE(u_int16_t *pc, u_int8_t v, u_int8_t k);
int SNE(u_int16_t *pc, u_int8_t v, u_int8_t k);
int SEr(u_int16_t *pc, u_int8_t v, u_int8_t y);
int LD(u_int8_t *v, u_int8_t kk, u_int8_t vi);
int ADD(u_int8_t *v, u_int8_t kk, u_int8_t vi);
int LDi(u_int16_t *nnn, u_int16_t *i);
int DRW(u_int8_t (*display_array)[64], unsigned int xi, unsigned int yi, unsigned int n, u_int16_t i_addr, u_int8_t *memory, u_int8_t *V);
int SNE9(u_int16_t *pc, u_int8_t x, u_int8_t y);
int JUMPv0(u_int16_t N, u_int16_t *pc,u_int8_t v0);
int REG_STORE(u_int16_t I, u_int8_t *registers, u_int8_t *mem, u_int8_t x);
int REG_LOAD(u_int16_t I, u_int8_t *registers, u_int8_t *mem, u_int8_t x);
int BCD(u_int8_t *memory, u_int8_t v, u_int16_t i);

#endif