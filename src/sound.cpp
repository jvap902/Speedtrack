#include "types.h"
#include "sound.h"

//FONTE: Sons utilizados são edições dos disponíveis nos seguintes links: https://freesound.org/people/GiocoSound/sounds/401550/ , https://freesound.org/people/ikbenraar/sounds/415276/

void soundInit(Sound& sounds)
{

    if (ma_engine_init(NULL, &sounds.engine) != MA_SUCCESS) {
        printf("ERROR initializing audio engine.\n");
    }
    if (ma_sound_init_from_file(&sounds.engine, "../../sounds/idle.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds.idle) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&sounds.idle, MA_TRUE);
    if (ma_sound_init_from_file(&sounds.engine, "../../sounds/accelerate.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds.accelerate) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&sounds.accelerate, MA_TRUE);
    if (ma_sound_init_from_file(&sounds.engine, "../../sounds/slow-down.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds.slowDown) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&sounds.slowDown, MA_TRUE);
    if (ma_sound_init_from_file(&sounds.engine, "../../sounds/max_speed.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &sounds.maxSpeed) != MA_SUCCESS) {
        printf("ERROR: could not load engine sound.\n");
    }
    ma_sound_set_looping(&sounds.maxSpeed, MA_TRUE);

    ma_sound_get_length_in_pcm_frames(&sounds.accelerate, &sounds.accelerateFrames);

    sounds.releasedW = false;
}

void soundControl(Sound& sounds, const InputState& input, CarState& car)
{
    uint64_t frameIndex = sounds.accelerateFrames * (car.speed / 40.0f);
    
    if (!input.w){
        sounds.releasedW = true;
        ma_sound_stop(&sounds.accelerate);
        ma_sound_stop(&sounds.maxSpeed);

        if(car.speed != 0.0f)
            ma_sound_start(&sounds.slowDown);
        else{
            ma_sound_stop(&sounds.slowDown);
            ma_sound_start(&sounds.idle);
        }
    }
    else{
        ma_sound_stop(&sounds.slowDown);
        ma_sound_stop(&sounds.idle);

        if(car.speed == 40.0f){ //velocidade máxima
            ma_sound_stop(&sounds.accelerate);
            ma_sound_start(&sounds.maxSpeed);
        }
        else if (sounds.releasedW){
            ma_sound_stop(&sounds.accelerate);


            ma_sound_seek_to_pcm_frame(&sounds.accelerate, frameIndex);
            ma_sound_start(&sounds.accelerate);

            sounds.releasedW = false;
        }
        else
         ma_sound_start(&sounds.accelerate);
    }

    //sounds.accelerateFrames leva para final do som, quando a vel deve trocar para maxSpeed
    //0 deve levar para o início
}
