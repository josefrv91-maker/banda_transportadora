#include "fsm_control.h"

// Columnas: NONE, START, STOP, RESET. STOP tambien protege el arranque.
constexpr Transition FSM[static_cast<uint8_t>(State::COUNT)]
                        [static_cast<uint8_t>(Events::COUNT)] = {
    {{State::IDLE, onNoop}, {State::RUNNING, onStart},
     {State::FAULT, onFault}, {State::IDLE, onReset}},
    {{State::RUNNING, onNoop}, {State::RUNNING, onNoop},
     {State::FAULT, onFault}, {State::IDLE, onReset}},
    {{State::FAULT, onNoop}, {State::RUNNING, onStart},
     {State::FAULT, onNoop}, {State::IDLE, onReset}}
};
