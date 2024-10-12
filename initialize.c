#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <time.h>
#include "headers/instructions.h"
#include "headers/sound.h"
#include "headers/keymapping.h"

///If this variable is set to 0, the emulator will terminate upon completing its last main loop iteration
int continue_execution = 1;

/** Start address for each of the 15 sprites, associared with the hexadecimal digits */
const u_int16_t SPRITE_ADDRESS[16] = {0x50, 0x55, 0x5A, 0x5F, 0x64, 0x69, 0x6E, 0x73, 0x78, 0x7D, 0x82, 0x87, 0x8C, 0x91, 0x96, 0x9B};

///Stores the last time when the delay timer was altered
u_int32_t current_time_delay;

///Stores the last time when the sound timer was altered
u_int32_t current_time_sound;

///Stores currently measured time
u_int32_t tmp_time;

/** RAM Memory in bytes */
const unsigned int TOTAL_RAM_MEMORY = 4096;

/** Screen Parameters */
const int SDL_SCREEN_WIDTH = 64;
const int SDL_SCREEN_HEIGHT = 32;
const int SDL_SCREEN_SCALE=25;

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
u_int16_t stack[16] = {0};

/** RAM Memory array */
u_int8_t memory[4096] = {0};

/** Display pixel array */
uint8_t display_array[32][64] = {0};

/** Upon receiving instruction Fx0A, the program execution should be paused. What we do instead is skip all instruction phases and
 * continue looping. This is needed in order to preserve the one-threaded nature of the timers (delay and sound).
 * This is the boolean flag that indicates if we are allowed to execute further instructions.
 */
int  allow_instruction_execution = 1;

/// Target register, where instruction Fx0A will store the pressed key
int keypress_register = 0;



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
    //SDL_Delay(100);
}

int RND(u_int8_t *v, u_int8_t k, u_int8_t x) {
    v[x] = (rand()%256) & k;
}

int main(int argc, char *argv[]) {


    srand(time(NULL));
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

    //This is a way of finding file length, in order to read all the data
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

    //Initializing SDL Library (video)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize video. Error: %s\n", SDL_GetError());
        return 1;
    }

    ///SDL Display
    SDL_Window *display = SDL_CreateWindow("kuche", 10,10, SDL_SCREEN_WIDTH * SDL_SCREEN_SCALE,SDL_SCREEN_HEIGHT * SDL_SCREEN_SCALE,SDL_WINDOW_SHOWN);

    if (display == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    ///SDL Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(display);
        SDL_Quit();
        return 1;
    }

    //Event struct object
    SDL_Event event;

    u_int16_t current_instruction;

    //Parameters, that may be part of the instruction. They are extracted in the decode stage
    u_int8_t opcode;
    u_int16_t nnn;
    u_int8_t x,y,kk;
    u_int8_t n;

    /// A flag that indicates whether or not a basic instruction without parameters was executed (opcode 0x00)
    int flag;

    /// Upon reaching 100 nops, the program is terminated with exit code 0.
    int breakflag = 0;
    sound_init();
    play_sound();

    u_int32_t last_frame = 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    ///Main CPU Cycle
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    while(continue_execution) {


        //End of RAM Memory reached
        if(PC>=4096) break;
        tmp_time = SDL_GetTicks();

    while(SDL_GetTicks() - last_frame <= 1) {
        //Event pool loop
        while (SDL_PollEvent(&event) != 0) {
            //Keypress
            if (event.type == SDL_KEYDOWN) {
                // Exit key
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    continue_execution = 0;
                }
                //Program is waiting for a keypress due to Fx0A
                if (!allow_instruction_execution) {
                    V[keypress_register] = qwerty_to_hex(event.key.keysym.sym);
                    allow_instruction_execution = 1;
                }

            }

        }
    }
    last_frame = SDL_GetTicks();
        //Optional delay to reduce CPU stress


        //Update internal program counter


        //Check for Delay Timer
        if(DT!=0) {
            if((double)tmp_time - (double)current_time_delay >= 16.66666) {
                current_time_delay = SDL_GetTicks();
                DT--;
            }
        }

        //Check for Sound Timer
        if(ST!=0) {
            if(tmp_time - current_time_sound >= 1000) {
                current_time_sound = SDL_GetTicks();
                ST--;
            }
        }



        //flag for the instruction Fx0A
        if(!allow_instruction_execution) continue;
        //if(breakflag==100) break;
        flag = 0;
        current_instruction = fetch(memory, &PC);
        opcode = current_instruction >> 12;

        switch(current_instruction) {
            case 0x00E0 : CLS(display_array); update_display(renderer); flag = 1; break;
            case 0x00EE : RET(&PC, stack, &SP); flag = 1; break;
        }
        if(flag) continue;


        nnn = current_instruction & 0xFFF;
        x = (current_instruction & 0xF00)>>8;
        y = (current_instruction & 0xF0)>>4;
        kk = current_instruction & 0xFF;
        n = nnn & 0xF;

        switch(opcode){
            case 1 : JUMP(nnn, &PC); break; // 1nnn
            case 2 : if(CALL(nnn,&PC,&SP,stack)<0) { perror("Stack overflow!"); return 1;} break; //2nnn
            case 3 : SE(&PC, V[x], kk); break; //3xkk
            case 4 : SNE(&PC, V[x], kk); break; //4xkk
            case 5 : if(n == 0) SEr(&PC, V[x], kk); break; //5xy0
            case 6 : LD(V,kk,x); break; //6xkk
            case 7 : ADD(V, kk, x); break; //7xkk
            case 8 :
                if(n == 0) {  //8xy0
                    V[x] = V[y];
                }

                else if(n == 1){  //8xy1
                    V[x] = V[x] | V[y];
                }

                else if(n == 2){  //8xy2
                    V[x] = V[x] & V[y];
                }

                else if(n == 3){  //8xy3
                    V[x] = V[x] ^ V[y];
                }

                else if(n == 4){  //8xy4
                    unsigned int tmp = V[x] + V[y];

                    V[x] = tmp & 0xFF;
                    if(tmp>255) V[15] = 1;
                    else V[15] = 0;
                }

                else if(n == 5){  //8xy5
                    int l = V[x] - V[y];
                    V[x] = V[x] - V[y];
                    if(l<0) {
                        V[15] = 0;
                    }
                    else {
                        V[15] = 1;
                    }

                }

                else if(n == 6){  // 8xy6
                    unsigned int tmp = V[x] & 1;
                    V[x] = V[x] >> 1;
                    V[15] = tmp;
                }

                else if(n == 7){  //8xy7
                    int l = V[y] - V[x];
                    V[x] = V[y] - V[x];
                    if(l<0) {
                        V[15] = 0;
                    }
                    else {
                        V[15] = 1;
                    }

                }

                else if(n == 14){  //8xyE
                    unsigned int tmp = (V[x] & 0x80) >> 7;
                    V[x] = V[x] << 1;
                    V[15] = tmp;
                }
                break;

            case 9 : if(n==0) SNE9(&PC, V[x], V[y]); break;  //9xy0
            case 0xA : LDi(&nnn, &I); break;  //Annn
            case 0xB : JUMPv0(nnn,&PC,V[0]); break;  //Bnnn
            case 0xC : RND(V,kk,x); break;  //Cxkk
            case 0xD : DRW(display_array,x,y,n,I,memory,V); update_display(renderer); break;  //Dxyn
            case 0xE :
                if((nnn & 0xFF) == 0x9E) { //Ex9E
                    const u_int8_t *keyState = SDL_GetKeyboardState(NULL);
                    if(keyState[hex_to_qwerty(V[x])]) PC+=2;
                }
                else if ((nnn & 0xFF) == 0xA1) { //ExA1
                    const u_int8_t *keyState = SDL_GetKeyboardState(NULL);
                    if(!keyState[hex_to_qwerty(V[x])]) PC+=2;
                }
                break;
            case 0xF :
                if((nnn & 0xFF) == 0x07) V[x] = DT;  //Fx07
                else if((nnn & 0xFF) == 0x0A) {  //Fx0A
                     keypress_register = x;
                     allow_instruction_execution = 0;
                 }
                else if ((nnn & 0xFF) == 0x15) {  //Fx15
                    DT = V[x];
                    if(DT!=0) {
                        current_time_delay = SDL_GetTicks();
                    }
                }
                else if ((nnn & 0xFF) == 0x18) {  //Fx18
                    ST = V[x];
                    if(ST!=0) {
                        current_time_sound = SDL_GetTicks();
                    }
                }
                else if ((nnn & 0xFF) == 0x1E) I = I + V[x];  //Fx1E
                else if ((nnn & 0xFF) == 0x29) { I = SPRITE_ADDRESS[V[x]];} //Fx29
                else if ((nnn & 0xFF) == 0x33) { BCD(memory, V[x], I); } //Fx33
                else if ((nnn & 0xFF) == 0x055) { REG_STORE(I,V,memory, x); } //Fx55
                else if ((nnn & 0xFF) == 0x65) { REG_LOAD(I, V, memory, x); }  //Fx65
                break;
            default: breakflag++;
        }

    }
    SDL_DestroyWindow(display);
    SDL_Quit();
}
