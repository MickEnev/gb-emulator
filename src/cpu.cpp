#include "cpu.h"

CPU::CPU() {
    _PC = 0x0100;
    _SP = 0xFFFE;
}

uint16_t CPU::getAF() const {
    return (_A << 8) | (_F & 0xF0);
}

void CPU::setAF(uint16_t val) {
    _A = (val >> 8) & 0xFF;
    _F = val & 0xF0;
}

uint16_t CPU::getBC() const {
    return (_B << 8) | _C;
}

void CPU::setBC(uint16_t val) {
    _B = (val >> 8) & 0xFF;
    _C = val & 0xFF;
}

uint16_t CPU::getDE() const {
    return (_D << 8) | _E;
}

void CPU::setDE(uint16_t val) {
    _D = (val >> 8) & 0xFF;
    _E = val & 0xFF;
}

uint16_t CPU::getHL() const {
    return (_H << 8) | _L;
}

void CPU::setHL(uint16_t val) {
    _H = (val >> 8) & 0xFF;
    _L = val & 0xFF;
}

uint16_t CPU::getPC() const {
    return _PC;
}

void CPU::setPC(uint16_t val) {
    _PC = val;
}

uint16_t CPU::getSP() const {
    return _SP;
}

void CPU::setSP(uint16_t val) {
    _SP = val;
}

void CPU::step() {
    uint8_t opcode = _mem.read(_PC++);
    executeOpcode(opcode);
}

uint8_t CPU::fetch8() {
    return _mem.read(_PC++);
}

uint16_t CPU::fetch16() {
    uint8_t low = fetch8();
    uint8_t high = fetch8();
    return (high << 8) | low;
}

void CPU::executeOpcode(uint16_t opcode) {
    // Fill with switches and opcodes 
    switch (opcode) {
        case 0x00: // NOP
            break;
        case 0x01: // LD BC,d16
            setBC(fetch16());
            break;
        case 0x02: // LD (BC), A)
            _mem.write(getBC(), _A);
            break;
        case 0x03: // INC BC
            setBC(getBC() + 1);
            break;
        case 0x04: // INC B
            _B++;
            break;
        case 0x05: // DEC B
            _B--;
            break;
        case 0x06: // LD B d8
            _B = fetch8();
            break;
        case 0x07: // RLCA?
            uint8_t carry = (_A & 0x80) >> 8;   // Get bit 7
            _A = (_A << 1) | carry;             // Rotate left circular 

            _F &= 0x0F;
            _F &= ~(1 << 7);
            _F &= ~(1 << 6);
            _F &= ~(1 << 5);

            if (carry) {
                _F |= (1 << 4);
            } else {
                _F &= ~(1 << 4);
            }
            break;
        case 0x08: // LD (a16), SP
            uint16_t addr = fetch16();
            uint16_t sp = getSP();
            _mem.write(addr, sp & 0xFF);
            _mem.write(addr + 1, sp >> 8);
            break;

    }
}