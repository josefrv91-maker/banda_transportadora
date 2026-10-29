#include "hal_motor.h"
#include <Arduino.h>
#include <driver/ledc.h>
#include <esp_err.h>

namespace {
constexpr int PIN_A = 12;
constexpr int PIN_B = 14;
constexpr uint32_t MAX_DUTY = 1023;
constexpr ledc_mode_t MODE = LEDC_HIGH_SPEED_MODE;
// Intervalo inicial de prueba: validar con la inercia y corriente reales.
constexpr uint32_t BRAKE_MS = 300;
int8_t activeDirection = 0;
int8_t pendingDirection = 0;
uint32_t pendingDuty = 0;
uint32_t brakeSince = 0;

void duty(ledc_channel_t channel, uint32_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(MODE, channel, value));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, channel));
}

void requestDirection(int8_t direction, uint32_t value) {
    if (value > MAX_DUTY) value = MAX_DUTY;
    if (value == 0) { motorStop(); return; }
    if (activeDirection == direction) {
        duty(direction > 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1, value);
        return;
    }
    if (pendingDirection == direction) {
        pendingDuty = value;
        return;
    }
    // Frenar y programar el cambio; nunca esperar con delay().
    if (activeDirection != 0) motorStop();
    pendingDirection = direction;
    pendingDuty = value;
}
}

void motorInicial() {
    // No movimiento antes de habilitar LEDC.
    digitalWrite(PIN_A, LOW);
    digitalWrite(PIN_B, LOW);
    pinMode(PIN_A, OUTPUT);
    pinMode(PIN_B, OUTPUT);
    ledc_timer_config_t timer = {};
    timer.speed_mode = MODE;
    timer.duty_resolution = LEDC_TIMER_10_BIT;
    timer.timer_num = LEDC_TIMER_0;
    timer.freq_hz = 1000;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ESP_ERROR_CHECK(ledc_timer_config(&timer));
    const int pins[] = {PIN_A, PIN_B};
    for (int i = 0; i < 2; ++i) {
        ledc_channel_config_t channel = {};
        channel.gpio_num = pins[i];
        channel.speed_mode = MODE;
        channel.channel = static_cast<ledc_channel_t>(i);
        channel.intr_type = LEDC_INTR_DISABLE;
        channel.timer_sel = LEDC_TIMER_0;
        channel.duty = 0;
        channel.hpoint = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&channel));
    }
    motorStop();
}

void motorPWM(uint32_t cicloTrabajo) {
    requestDirection(1, cicloTrabajo);
}

void motorReversaPWM(uint32_t cicloTrabajo) {
    requestDirection(-1, cicloTrabajo);
}

void motorActualizar() {
    if (pendingDirection == 0 || uint32_t(millis() - brakeSince) < BRAKE_MS) return;
    // Quitar ambas excitaciones antes de activar solo el canal del sentido elegido.
    duty(LEDC_CHANNEL_0, 0);
    duty(LEDC_CHANNEL_1, 0);
    duty(pendingDirection > 0 ? LEDC_CHANNEL_0 : LEDC_CHANNEL_1, pendingDuty);
    activeDirection = pendingDirection;
    pendingDirection = 0;
}

void motorStop() {
    // Ambas entradas HIGH: frenado dinamico. Sin espera de software.
    ESP_ERROR_CHECK(ledc_stop(MODE, LEDC_CHANNEL_0, 1));
    ESP_ERROR_CHECK(ledc_stop(MODE, LEDC_CHANNEL_1, 1));
    activeDirection = 0;
    pendingDirection = 0;
    brakeSince = millis();
}
