//
// Created by martin on 10/9/24.
//

#ifndef CHIP8EMULATOR_KEYMAPPING_H
#define CHIP8EMULATOR_KEYMAPPING_H
#include<SDL2/SDL.h>

///Given a hexadecimal value, the function returns its modern qwerty mapping
SDL_Scancode hex_to_qwerty(u_int8_t key);

///Given an SDL_KeyCode (char wrapper), the function returns its original hex value, according to the CHIP-8 Layout
u_int8_t qwerty_to_hex(SDL_Scancode key);

#endif //CHIP8EMULATOR_KEYMAPPING_H
