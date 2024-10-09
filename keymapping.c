#include "headers/keymapping.h"
//hex -> qwerty
SDL_Scancode translate[16] = {
        SDL_SCANCODE_X,
        SDL_SCANCODE_1,
        SDL_SCANCODE_2,
        SDL_SCANCODE_3,
        SDL_SCANCODE_Q,
        SDL_SCANCODE_W,
        SDL_SCANCODE_E,
        SDL_SCANCODE_A,
        SDL_SCANCODE_S,
        SDL_SCANCODE_D,
        SDL_SCANCODE_Z,
        SDL_SCANCODE_C,
        SDL_SCANCODE_4,
        SDL_SCANCODE_R,
        SDL_SCANCODE_F,
        SDL_SCANCODE_V
};


SDL_Scancode hex_to_qwerty(u_int8_t key){
    if(key>15) return '\0';
    return translate[key];
}

u_int8_t qwerty_to_hex(SDL_Scancode key){
    switch (key) {
        case SDL_SCANCODE_X : return 0;
        case SDL_SCANCODE_1 : return 1;
        case SDL_SCANCODE_2 : return 2;
        case SDL_SCANCODE_3 : return 3;
        case SDL_SCANCODE_Q : return 4;
        case SDL_SCANCODE_W : return 5;
        case SDL_SCANCODE_E : return 6;
        case SDL_SCANCODE_A : return 7;
        case SDL_SCANCODE_S : return 8;
        case SDL_SCANCODE_D : return 9;
        case SDL_SCANCODE_Z : return 10;
        case SDL_SCANCODE_C : return 11;
        case SDL_SCANCODE_4 : return 12;
        case SDL_SCANCODE_R : return 13;
        case SDL_SCANCODE_F : return 14;
        case SDL_SCANCODE_V : return 15;
        default : return -1;
    }
}