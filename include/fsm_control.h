#pragma once
#include <stdint.h>

enum class State : uint8_t { IDLE = 0, RUNNING, FAULT, COUNT };
enum class Events : uint8_t { NONE = 0, START, STOP, RESET, COUNT };

struct Transition {
    State nextState;
    void (*action)();
};

extern const Transition FSM[static_cast<uint8_t>(State::COUNT)]
                           [static_cast<uint8_t>(Events::COUNT)];
extern void onStart();
extern void onFault();
extern void onReset();
extern void onNoop();
