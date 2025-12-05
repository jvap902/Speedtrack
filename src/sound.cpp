#pragma once

#include "types.h"
#include "sound.h"


void soundInit(ma_engine& engine, ma_sound& maxSpeed, ma_sound& idle, ma_sound& accelerate, ma_sound& slowDown)
{

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        printf("ERROR initializing audio engine.\n");
    }
    if (ma_sound_init_from_file(&engine, "../../sounds/idle.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &idle) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&idle, MA_TRUE);
    if (ma_sound_init_from_file(&engine, "../../sounds/accelerate.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &accelerate) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&accelerate, MA_TRUE);
    if (ma_sound_init_from_file(&engine, "../../sounds/slow-down.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &slowDown) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&slowDown, MA_TRUE);
    if (ma_sound_init_from_file(&engine, "../../sounds/max_speed.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &maxSpeed) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&maxSpeed, MA_TRUE);
}

void soundControl(ma_sound& maxSpeed, ma_sound& idle, ma_sound& accelerate, ma_sound& slowDown, const InputState& input, CarState& car)
{    
    if (!input.w){
        ma_sound_stop(&accelerate);
        ma_sound_stop(&maxSpeed);

        if(car.speed != 0.0f)
            ma_sound_start(&slowDown);
        else{
            ma_sound_stop(&slowDown);
            ma_sound_start(&idle);
        }
    }
    else{
        ma_sound_stop(&slowDown);
        ma_sound_stop(&idle);

        if(car.speed == 40.0f) //velocidade máxima
            ma_sound_start(&maxSpeed);
        else
            ma_sound_start(&accelerate);
    }
}