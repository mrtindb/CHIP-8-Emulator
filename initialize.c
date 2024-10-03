#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "main.h"
/** RAM Memory in bytes */
const unsigned int TOTAL_RAM_MEMORY = 4096;

const int SDL_SCREEN_WIDTH = 64;
const int SDL_SCREEN_HEIGHT = 32;
const int SDL_SCREEN_SCALE=15;

const unsigned int INSTRUCTIONS_PER_SECOND = 700;

/** Useful memory for storing program code */
const unsigned int AVAILABLE_RAM_MEMORY = TOTAL_RAM_MEMORY - 512;

/** Register array */
u_int8_t V[16];

/** Special register I */
u_int16_t I;

/** Program counter */
u_int16_t PC = 0x200;

/** Stack pointer */
u_int8_t SP = 0;

/** Delay timer */
u_int8_t DT;

/** Sound timer */
u_int8_t ST;

/** Stack array */
u_int8_t stack[16] = {0};

/** RAM Memory array */
u_int8_t memory[4096] = {0};

/** Display pixel array */
uint8_t display_array[32][64] = {0};

void update_display(SDL_Renderer *renderer) {
    for(int y=0;y<SDL_SCREEN_HEIGHT;y++) {
        for(int x=0;x<SDL_SCREEN_WIDTH;x++) {
            if(display_array[y][x]!=0) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            }
            else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            SDL_Rect rect = {x * SDL_SCREEN_SCALE, y * SDL_SCREEN_SCALE, SDL_SCREEN_SCALE, SDL_SCREEN_SCALE};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
    SDL_Delay(100);
}

int main(int argc, char *argv[])
{

    FILE *bin;

    //Sequence for pushing the program into RAM Memory
    char *arg = argv[1];
    if (arg == NULL) {
        perror("Error: Expected file name as argument.");
        return 1;
    }

    bin = fopen(arg, "rb");

    if (bin == NULL) {
        perror("Error opening file.");
        return 1;
    }

    fseek(bin, 0, SEEK_END);
    unsigned long file_size = ftell(bin);
    rewind(bin);

    //char *initial_buffer = malloc(file_size);
    //Statically allocated memory is more efficient and safe in this case, as 4096 Bytes (RAM Size) is negligible amount in modern computers
    unsigned char initial_buffer[4096];
    fread(initial_buffer, sizeof(initial_buffer), file_size, bin);
    fclose(bin);

    //Setting the termination character after the file has been read
    initial_buffer[file_size] = '\0';

    //Write the binary dump in RAM Memory
    for(int i = PC; i<file_size+PC && i<4096; i++){
        memory[i] = initial_buffer[i-PC];
    }

    //Load font from file font.bin or argument
    arg = argv[2];
    if(arg == NULL) {
        arg = "font.bin";
    }

    bin = fopen(arg, "rb");

    if(bin == NULL) {
        perror("Error opening font file");
        return 1;
    }
    fseek(bin,0,SEEK_END);
    file_size = ftell(bin);
    rewind(bin);
    fread(initial_buffer,sizeof(initial_buffer), file_size, bin);
    initial_buffer[file_size] = '\0';

    for(int i = 0x50; i<file_size+0x50;  i++){
        memory[i] = initial_buffer[i-0x50];
    }

    //Initializing the display window
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *display = SDL_CreateWindow("kuche", 10,10, SDL_SCREEN_WIDTH * SDL_SCREEN_SCALE,SDL_SCREEN_HEIGHT * SDL_SCREEN_SCALE,0);

    if (display == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    //SDL Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(display);
        SDL_Quit();
        return 1;
    }

    u_int16_t current_instruction;
    u_int8_t opcode;
    u_int16_t nnn;
    u_int8_t x,y,kk;
    u_int8_t n;
    int flag;
    int breakflag = 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///Main CPU Cycle
    while(1) {
        if(breakflag==100) break;
        flag = 0;
        current_instruction = fetch(memory, &PC);
        opcode = current_instruction >> 12;

        switch(current_instruction) {
            case 0x00E0 : CLS(renderer); flag = 1; break;
            case 0x00EE : RET(&PC, stack, &SP); flag = 1; break;
            default : breakflag++;
        }

        if(flag) continue;

        nnn = current_instruction & 0xFFF;
        x = (current_instruction & 0xF00)>>8;
        y = (current_instruction & 0xF0)>>4;
        kk = current_instruction & 0xFF;
        n = nnn & 0xF;

        switch(opcode){
            case 1 : JUMP(nnn, &PC); break;
            case 2 : if(CALL(nnn,&PC,&SP,stack)<0) { perror("Stack overflow!"); return 1;} break;
            case 3 : SE(&PC, V[x], kk); break;
            case 4 : SNE(&PC, V[x], kk); break;
            case 5 : SEr(&PC, V[x], kk); break;
            case 6 : LD(V,kk,x); break;
            case 7 : ADD(V, kk, x); break;

            case 8 :
                if(n == 0) {
                    V[x] = V[y];
                }
                else if(n == 1){
                    V[x] = V[x] | V[y];
                }
                else if(n == 2){
                    V[x] = V[x] & V[y];
                }
                else if(n == 3){
                    V[x] = V[x] ^ V[y];
                }
                else if(n == 4){
                    unsigned int tmp = V[x] + V[y];
                    if(tmp>255) V[15] = 1;
                    V[x] = tmp & 0xFF;
                }
                else if(n == 5){
                    
                }
                else if(n == 6){

                }
                else if(n == 7){

                }
                else if(n == 14){

                }break;


            case 10 : LDi(&nnn, &I); break;
            case 13 : DRW(display_array,x,y,n,I,memory,V); update_display(renderer); break;
            default: breakflag++;
        }
    }
    SDL_Delay(2000);
}
