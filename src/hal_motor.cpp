#include "hal_motor.h"
#include <Arduino.h>
#include <driver/ledc.h>
#include <esp_err.h>

namespace {
constexpr int PIN_A = 12;
constexpr int PIN_B = 14;
constexpr uint32_t MAX_DUTY = 1023;
constexpr ledc_mode_t MODE = LEDC_HIGH_SPEED_MODE;

void duty(ledc_channel_t channel, uint32_t value) {
    ESP_ERROR_CHECK(ledc_set_duty(MODE, channel, value));
    ESP_ERROR_CHECK(ledc_update_duty(MODE, channel));
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
    if (cicloTrabajo > MAX_DUTY) cicloTrabajo = MAX_DUTY;
    // Salir del freno antes de aplicar avance.
    duty(LEDC_CHANNEL_0, 0);
    duty(LEDC_CHANNEL_1, 0);
    duty(LEDC_CHANNEL_0, cicloTrabajo);
}

void motorStop() {
    // Ambas entradas HIGH: frenado dinamico. Sin espera de software.
    ESP_ERROR_CHECK(ledc_stop(MODE, LEDC_CHANNEL_0, 1));
    ESP_ERROR_CHECK(ledc_stop(MODE, LEDC_CHANNEL_1, 1));
}
