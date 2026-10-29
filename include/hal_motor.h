#pragma once
#include <stdint.h>

void motorInicial();
void motorPWM(uint32_t cicloTrabajo);
void motorStop();
void motorReversaPWM(uint32_t cicloTrabajo);
void motorActualizar();
