#include <stdio.h>
#include <SDL2/SDL.h>


u_int16_t fetch(u_int8_t *mem, u_int16_t *pc) {
    u_int16_t result = 0;
    result = mem[*pc];
    *pc = *pc+1;
    result = result << 8;
    result += mem[*pc];
    *pc = *pc+1;
    return result;
}

///CLears the display
int CLS(SDL_Renderer *renderer){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    return 0;
}

int RET(u_int16_t *pc,u_int8_t *stk, u_int8_t *sp) {
    if(*sp==0) return 1;
    *sp = *sp - 1;
    *pc = stk[*sp];
    stk[*sp] = 0;
    return 0;
}

int JUMP(u_int16_t N, u_int16_t *pc) {
    *pc = N;
    return 0;
}

int CALL(u_int16_t N, u_int16_t *pc, u_int8_t *sp, u_int8_t *stk) {
    stk[*sp] = *pc;
    if(*sp==16) return -1;
    *sp = *sp + 1;
    *pc = N;
    return 0;
}

int SE(u_int16_t *pc, u_int8_t v, u_int8_t k){
    if(k==v) *pc = *pc + 2;
    return 0;
}

int SNE(u_int16_t *pc, u_int8_t v, u_int8_t k){
    if(k!=v) *pc = *pc + 2;
    return 0;
}

int SEr(u_int16_t *pc, u_int8_t v, u_int8_t y) {
    if(v==y) *pc = *pc + 2;
    return 0;
}

int LD(u_int8_t *v, u_int8_t kk, u_int8_t vi) {
    v[vi] = kk;
    return 0;
}

int ADD(u_int8_t *v, u_int8_t kk, u_int8_t vi) {
    v[vi] = v[vi] + kk;
    return 0;
}

int LDi(u_int16_t *nnn, u_int16_t *i) {
    *i = *nnn;
    return 0;
}

int DRW(u_int8_t (*display_array)[64], unsigned int xi, unsigned int yi, unsigned int n, u_int16_t i_addr, u_int8_t *memory, u_int8_t *V) {
    u_int8_t b[8];
    unsigned int x = V[xi];
    unsigned int y = V[yi];
    for(unsigned int ct = y; ct<n+y;ct++, i_addr++) {
        b[7] = memory[i_addr] & 1;
        b[6] = (memory[i_addr] & 2)>>1;
        b[5] = (memory[i_addr] & 4)>>2;
        b[4] = (memory[i_addr] & 8)>>3;
        b[3] = (memory[i_addr] & 0x10)>>4;
        b[2] = (memory[i_addr] & 0x20)>>5;
        b[1] = (memory[i_addr] & 0x40)>>6;
        b[0] = (memory[i_addr] & 0x80)>>7;
        for(unsigned int bit = x; bit<8+x; bit++){
            if(display_array[ct%32][bit%64] & b[bit-x]) V[16] = 1;
            display_array[ct%32][bit%64] = display_array[ct%32][bit%64] ^ b[bit-x];
        }
    }
    return 0;
}

int SNE9(u_int16_t *pc, u_int8_t x, u_int8_t y) {
    if(x!=y) *pc = *pc + 2;
    return 0;
}

int JUMPv0(u_int16_t N, u_int16_t *pc,u_int8_t v0) {
    *pc = *pc + N + v0;
    return 0;
}

int REG_STORE(u_int16_t I, u_int8_t *registers, u_int8_t *mem) {

    for(int i=0;i<16;i++) {
        mem[I] = registers[i];
        I++;
    }
    return 0;
}

int REG_LOAD(u_int16_t I, u_int8_t *registers, u_int8_t *mem){
    for(int i=0;i<16;i++) {
        registers[i] = mem[I];
        I++;
    }
    return 0;
}

