#include "fsm_control.h"

// Columnas: NONE, START, STOP, RESET, REVERSE.
constexpr Transition FSM[static_cast<uint8_t>(State::COUNT)]
                        [static_cast<uint8_t>(Events::COUNT)] = {
    {{State::IDLE, onNoop}, {State::RUNNING, onStart},
     {State::FAULT, onFault}, {State::IDLE, onReset}, {State::RUNNING, onReverse}},
    {{State::RUNNING, onNoop}, {State::RUNNING, onStart},
     {State::FAULT, onFault}, {State::IDLE, onReset}, {State::RUNNING, onReverse}},
    {{State::FAULT, onNoop}, {State::RUNNING, onStart},
     {State::FAULT, onNoop}, {State::IDLE, onReset}, {State::RUNNING, onReverse}}
};
