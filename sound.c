//
//A file containing boilerplate code for the sound functions. Calling the method play_sound plays a one second sound,
// corresponding to the CHIP-8 instruction set and sound timer
//
#include <SDL2/SDL.h>
#include <unistd.h>
#include<stdio.h>
#include<math.h>
#include "headers/sound.h"
SDL_AudioDeviceID deviceId;
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 128;
const double FREQUENCY = 600.0;
SDL_AudioSpec sound;
u_int8_t audioBuffer[44100 * 4];

void sound_init(void) {
    sin(2);
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(1);
    }
    deviceId = SDL_OpenAudioDevice(NULL, 0, &sound, NULL, 0);
    if (deviceId == 0) {
        fprintf(stderr, "Failed to open audio device: %s\n", SDL_GetError());
        exit(1);
    }
    sound.freq = SAMPLE_RATE;
    sound.format = AUDIO_U8;
    sound.channels = 1;
    sound.samples = 4096;
    sound.callback = NULL;
    sound.userdata = NULL;
    double t;
    for (int i = 0; i < 88200*2; ++i) {
        t = (double)i / SAMPLE_RATE;
        audioBuffer[i] = (u_int8_t)(AMPLITUDE * (0.5 + 0.5 * sin(2.0 * M_PI * FREQUENCY * t)));
    }
    SDL_PauseAudioDevice(deviceId, 0);
    fflush(stdout);
}

///Plays a 1sec sound
void play_sound(void) {
    for(int i=0;i<=20000;i++) {
        if (SDL_QueueAudio(deviceId, audioBuffer, 44100 * 4) < 0) {
            fprintf(stderr, "Failed to queue audio: %s\n", SDL_GetError());
        }

    }


    SDL_PauseAudioDevice(deviceId, 1);


}