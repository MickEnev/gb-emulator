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

void CPU::executeOpcode(uint8_t opcode) {
    // TODO: Figure out cycles!!
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
            uint8_t carry = (_A & 0x80) >> 7;   // Get bit 7
            _A = (_A << 1) | carry;             // Rotate left circular 

            _F = 0; // Clear the upper 4 F bits which house the flags

            if (carry) { // If carry set the 000*0000 to 1
                _F |= (1 << 4);
            }
            break;
        case 0x08: // LD (a16), SP
            uint16_t addr = fetch16();
            uint16_t sp = getSP();
            _mem.write(addr, sp & 0xFF);
            _mem.write(addr + 1, sp >> 8);
            break;
        case 0x09: // ADD HL, BC
            setHL(getBC() + getHL());
            break;
        case 0x0A: // LD A, (BC)
            _A = _mem.read(getBC());
        break;
        case 0x0B: // DEC BC
            setBC(getBC() - 1);
            break;
        case 0x0C: // INC C
            _C += 1;
            break;
        case 0x0D: // DEC C
            _C -= 1;
            break;
        case 0x0E: // LD C, d8
            _C = fetch8();
            break;
        case 0x0F: // RRCA
            uint8_t carry = (_A & 0x01);
            _A = (_A >> 1) | (carry << 7);

            _F = 0;

            if (carry) {
                _F |= (1 << 4);
            }
            break;
        case 0x10: // STOP 0
            fetch8();
            _stopped = true;
            break;
        case 0x11: // LD DE, d16
            setDE(fetch16());
            break;
        case 0x12: // LD (DE), A
            _mem.write(getDE(), _A);
            break;
        case 0x13: // INC DE
            setDE(getDE() + 1);
            break;
        case 0x14: // INC D
            _D += 1;
            break;
        case 0x15: // DEC D
            _D -= 1;
            break;
        case 0x16: // LD D, d8
            _D = fetch8();
            break;
        case 0x17: // RLA
            uint8_t carry = (_A & 0x80) >> 7; // Get the highest bit
            uint8_t old_carry = (_F & 0x10) >> 4;

            _A = (_A << 1) | old_carry;

            _F = 0;

            if (carry) {
                _F |= (1 << 4);
            }
            break;
        case 0x18: // JR r8
        int8_t offset = (int8_t)fetch8();
            _PC += offset;
            break;
        case 0x19: // ADD HL, DE
            setHL(getHL() + getDE());
            break;
        case 0x1A: // LD A, (DE)
            _A = _mem.read(getDE());
            break;
        case 0x1B: // DEC DE
            setDE(getDE() - 1);
            break;
        case 0x1C: // INC E
            _E += 1;
            break;
        case 0x1D: // DEC E
            _E -= 1;
            break;
        case 0x1E: // LD E, d8
            _E = fetch8();
            break;
        case 0x1F: // RRA
            uint8_t carry = (_A & 0x01);
            uint8_t old_carry = (_F & 0x10) >> 4;

            _A = (_A >> 1) | (old_carry << 7);

            _F = 0;

            if (carry) {
                _F |= (1 << 4);
            }
            break;
        case 0x20: // JR NZ, r8
            uint8_t z = (_F & 0x80) >> 7;
            int8_t offset = (int8_t)fetch8();
            if (!z) {
                _PC += offset;
            }
            break;
        case 0x21: // LD HL, d16
            setHL(fetch16());
            break;
        case 0x22: // LD (HL+), A
            setHL(_A);
            setHL(getHL() + 1);
            break;
        case 0x23: // INC HL
            setHL(getHL() + 1);
            break;
        case 0x24: // INC H
            _H += 1;
            break;
        case 0x25: // DEC H
            _H -= 1;
            break;
        case 0x26: // LD H, d8
            _H = fetch8();
            break;
        case 0x27: // DAA
            uint8_t n = (_F & 0x70) >> 6;
            int adjustment = 0; // Type is definitley wrong
            if (n) {
                
            } else {

            }
    }
}