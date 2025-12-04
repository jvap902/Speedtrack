#include "miniaudio.h"
#include "types.h"

void soundInit(ma_engine& engine, ma_sound& maxSpeed, ma_sound& idle, ma_sound& accelerate, ma_sound& slowDown);
void soundControl(ma_sound& maxSpeed, ma_sound& idle, ma_sound& accelerate, ma_sound& slowDown, const InputState& input, CarState& car);