#include <SDL2/SDL.h>
#include "headers/sound.h"
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 128;
SDL_AudioSpec sound;
uint8_t audioBuffer[44100];

void sound_init(void) {
    printf("sound init");
    sound.freq = SAMPLE_RATE;
    sound.format = AUDIO_U8;
    sound.channels = 1;
    sound.samples = 4096;
    sound.callback = NULL;
    sound.userdata = NULL;
    for (int i = 0; i < 44100; ++i) {
        audioBuffer[i] = AMPLITUDE;
    }

}
///Plays a 1sec sound
void play_sound(void) {
    printf("playing sound");
    SDL_QueueAudio(1, audioBuffer, 44100); // Queue the audio
    SDL_PauseAudio(0);
}