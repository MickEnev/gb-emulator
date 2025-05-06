#include "cpu.h"

CPU::CPU() {

}

uint16_t CPU::getAF() const {
    return (_A << 8) | (_F & 0xF0);
}
