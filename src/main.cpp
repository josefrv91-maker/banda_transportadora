#include <Arduino.h>
#include <xtensa/core-macros.h>
#include "fsm_control.h"
#include "hal_motor.h"

namespace {
constexpr uint8_t PIN_STOP = 33;
constexpr uint32_t CPU_MHZ = 160;
constexpr uint32_t DEBOUNCE_CYCLES = CPU_MHZ * 1000U * 25U;
constexpr uint32_t MOTOR_DUTY = 512; // 50 % aprox.; ajustar con la banda real.

portMUX_TYPE eventMux = portMUX_INITIALIZER_UNLOCKED;
volatile bool stopPending = false;
volatile uint64_t pressCount = 0;
volatile uint32_t lastCycle = 0;
volatile bool haveLastCycle = false;
State currentState = State::IDLE;
bool startPending = true;
bool resetPending = false;

void IRAM_ATTR stopISR() {
    const uint32_t now = XTHAL_GET_CCOUNT();
    portENTER_CRITICAL_ISR(&eventMux);
    if (!haveLastCycle || uint32_t(now - lastCycle) >= DEBOUNCE_CYCLES) {
        lastCycle = now;
        haveLastCycle = true;
        stopPending = true;
        ++pressCount;
    }
    portEXIT_CRITICAL_ISR(&eventMux);
}

void dispatch(Events event) {
    const auto s = static_cast<uint8_t>(currentState);
    const auto e = static_cast<uint8_t>(event);
    if (s >= static_cast<uint8_t>(State::COUNT) ||
        e >= static_cast<uint8_t>(Events::COUNT)) return;
    const Transition &transition = FSM[s][e];
    currentState = transition.nextState;
    transition.action();
}
}

void setup() {
    motorInicial();
    pinMode(PIN_STOP, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_STOP), stopISR, FALLING);
    Serial.begin(115200);
    Serial.println("Banda: R = RESET completo. Paro por microswitch IO33.");
    // START se atiende en loop; el paro tiene prioridad.
}

void loop() {
    // Leer como maximo un caracter por vuelta: trabajo acotado.
    if (Serial.available() > 0) {
        const int c = Serial.read();
        if (c == 'R' || c == 'r') resetPending = true;
    }

    bool stop;
    portENTER_CRITICAL(&eventMux);
    stop = stopPending;
    stopPending = false;
    // Tras un intervalo largo, aceptar sin depender de vueltas de CCOUNT.
    if (haveLastCycle && uint32_t(XTHAL_GET_CCOUNT() - lastCycle) >= DEBOUNCE_CYCLES)
        haveLastCycle = false;
    portEXIT_CRITICAL(&eventMux);

    const bool pressed = digitalRead(PIN_STOP) == LOW;
    if (stop || (pressed && currentState != State::FAULT)) {
        startPending = false;
        dispatch(Events::STOP);
    } else if (resetPending) {
        resetPending = false;
        dispatch(Events::RESET);
    } else if (startPending) {
        startPending = false;
        dispatch(Events::START);
    } else {
        dispatch(Events::NONE);
    }
}

void onStart() {
    // Segunda comprobacion inmediata antes de aplicar PWM.
    if (digitalRead(PIN_STOP) == LOW) {
        dispatch(Events::STOP);
        return;
    }
    motorPWM(MOTOR_DUTY);
}
void onFault() { motorStop(); }
void onReset() {
    motorStop();
    ESP.restart();
}
void onNoop() {}
