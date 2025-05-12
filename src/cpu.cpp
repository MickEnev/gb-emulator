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

void CPU::setINCFlags(uint8_t& r) {
    r++;
    _F &= 0x10; // Keep C flag, clear Z, N, H
    if (r == 0) _F |= 0x80; // Z
    if ((r & 0x0F) == 0x00) _F |= 0x20;
}

void CPU::setDECFlags(uint8_t& r) {
    r--;
    _F &= 0x10; // Keep C flag, clear Z, N, H
    if (r == 0) _F |= 0x80; // Z
    _F |= 0x40; // N = 1
    if ((r & 0x0F) == 0x0F) _F |= 0x20; // H if borrow from bit 4
}

void CPU::setADDFlags(uint8_t& r) {
    uint16_t result = _A + r;
    // 1011 1010
    // 1100 1110
    // ----------
    // 1000 1000

    if (result == 0) { // Set Z flag
        _F |= 0x80;
    }
    _F &= ~(0x40); // Clear the N flag

    if ((_A & 0xF) + (r & 0xF) > 0xF) { // Set H flag
        _F |= 0x20;
    }

    if ((int)_A + (int)r > 0xFF) { // Set C flag
        _F |= 0x10;
    }
    _A = result & 0xFF;
}

void CPU::setADCFlags(uint8_t& r) {
    uint8_t carry = (_F & 0x10) ? 1 : 0;
    uint16_t result = _A + r + carry;

    if (result == 0) { // Set Z flag
        _F |= 0x80;
    }
    _F &= ~(0x40); // Clear the N flag

    if ((_A & 0xF) + (r & 0xF) > 0xF) { // Set H flag
        _F |= 0x20;
    }

    if ((int)_A + (int)r > 0xFF) { // Set C flag
        _F |= 0x10;
    }
    _A = result & 0xFF;
}

void CPU::setSUBFlags(uint8_t& r) {
    uint16_t result = _A - r;

    if (result == 0) { // Set Z flag
        _F |= 0x80;
    }

    _F |= 0x40; // Set the N flag since N == subtraction

    if ((_A & 0xF) < (r & 0xF)) { // Set H flag if we borrow from bit 4
        _F |= 0x20;
    }

    if (_A < r) { // Set C flag if we borrow from bit 8
        _F |= 0x10;
    }
    _A = result & 0xFF;
}

void CPU::setSBCFlags(uint8_t& r) {
    uint8_t carry = (_F & 0x10) ? 1 : 0;
    uint16_t result = _A - r - carry;

    if (result == 0) { // Set Z flag
        _F |= 0x80;
    }

    _F |= 0x40; // Set the N flag since N == subtraction

    if ((_A & 0xF) < (r & 0xF)) { // Set H flag if we borrow from bit 4
        _F |= 0x20;
    }

    if (_A < r) { // Set C flag if we borrow from bit 8
        _F |= 0x10;
    }
    _A = result & 0xFF;
}

void CPU::setANDFlags(uint8_t& r) {
    uint8_t result = _A & r;

    _F = 0; // Clear all flags
    
    if (!result) {
        _F |= 0x80; // Set Z flag
    }
    _F |= 0x20; // Set H flag, always 1 for AND
    
    _A = result;
}

void CPU::setXORFlags(uint8_t& r) {
    uint8_t result = _A ^ r;

    _F = 0; // Clear all flags

    if (!result) {
        _F |= 0x80;
    }

    _A = result;
}

void CPU::setORFlags(uint8_t& r) {
    uint8_t result = _A | r;

    _F = 0; // Clear all flags

    if (!result) {
        _F |= 0x80;
    }

    _A = result;
}

void CPU::setCPFlags(uint8_t& r) {
    uint16_t result = _A - r;

    if (result == 0) { // Set Z flag
        _F |= 0x80;
    }

    _F |= 0x40; // Set the N flag since N == subtraction

    if ((_A & 0xF) < (r & 0xF)) { // Set H flag if we borrow from bit 4
        _F |= 0x20;
    }

    if (_A < r) { // Set C flag if we borrow from bit 8
        _F |= 0x10;
    }
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
            setINCFlags(_B);
            break;
        case 0x05: // DEC B
            setDECFlags(_B);
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
            uint16_t result = getBC() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getBC() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;
        case 0x0A: // LD A, (BC)
            _A = _mem.read(getBC());
        break;
        case 0x0B: // DEC BC
            setBC(getBC() - 1);
            break;
        case 0x0C: // INC C
            setINCFlags(_C);
            break;
        case 0x0D: // DEC C
            setDECFlags(_C);
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
            setINCFlags(_D);
            break;
        case 0x15: // DEC D
            setDECFlags(_D);
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
            uint16_t result = getDE() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getDE() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;
        case 0x1A: // LD A, (DE)
            _A = _mem.read(getDE());
            break;
        case 0x1B: // DEC DE
            setDE(getDE() - 1);
            break;
        case 0x1C: // INC E
            setINCFlags(_E);
            break;
        case 0x1D: // DEC E
            _E -= 1;
            break;
        case 0x1E: // LD E, d8
            setDECFlags(_E);
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
            setINCFlags(_H);
            break;
        case 0x25: // DEC H
            setDECFlags(_H);
            break;
        case 0x26: // LD H, d8
            _H = fetch8();
            break;
        case 0x27: // DAA
            int adjustment = 0;
            bool carry = false;

            if (_F & 0x40) {
                if (_F & 0x20) {
                    adjustment |= 0x06;
                }
                
                if (_F & 0x10) {
                    adjustment |= 0x60;
                    carry = true;
                }
                _A -= adjustment;
            } else {
                if ((_F & 0x20) || (_A & 0x0F) > 9) {
                    adjustment |= 0x06;
                }

                if ((_F & 0x10) || (_A > 0x99)) {
                    adjustment |= 0x60;
                    carry = true;
                }
                _A += adjustment;
            }

            _F &= ~(0x20 | 0x10); // Clear H and C
            if (adjustment & 0x60) {
                _F |= 0x10; // Set C if adjustment > 0x60
            }
            if (_A == 0) {
                _F |= 0x80;
            }
            break;
        case 0x28: // JR Z, r8
            uint8_t z = (_F & 0x80) >> 7;
            int8_t offset = (int8_t)fetch8();
            if (z) {
                _PC += offset;
            }
            break;
        case 0x29: // ADD HL, HL
            uint16_t result = getHL() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getHL() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;
        case 0x2A: // LD A, (HL+)
            _A = _mem.read(getHL() + 1);
            break;
        case 0x2B: // DEC HL
            setHL(getHL() - 1);
            break;
        case 0x2C: // INC L
            setINCFlags(_L);
            break;
        case 0x2D: // DEC L
            setDECFlags(_L);
            break;
        case 0x2E: // LD L, d8
            _L = fetch8();
            break;
        case 0x2F: // CPL
            _A = ~_A;
            break;
        case 0x30: // JR NC, r8
            uint8_t c = (_F & 0x50) >> 4;
            int8_t offset = (int8_t)fetch8();
            if (!c) {
                _PC += offset;
            }
            break;
        case 0x31: // LD SP, d16
            setSP(fetch16());
            break;
        case 0x32: // LD (HL-), A
            setHL(_A);
            setHL(getHL() - 1);
            break;
        case 0x33: // INC SP
            setSP(getSP() + 1);
            break;
        case 0x34: // INC (HL)
            uint8_t val = _mem.read(getHL());
            setINCFlags(val);
            _mem.write(getHL(), val);
            break;
        case 0x35: // DEC (HL)
            uint8_t val = _mem.read(getHL());
            setDECFlags(val);
            _mem.write(getHL(), val);
            break;
        case 0x36: // LD (HL), d8 COULD BE SOURCE OF ERROR
            uint8_t addr = _mem.read(getHL());
            _mem.write(addr, fetch8());
            break;
        case 0x37: // SCF
            _F &= 0x10;
            break;
        case 0x38: // JR C, r8
            uint8_t z = (_F & 0x10) >> 4;
            int8_t offset = (int8_t)fetch8();
            if (z) {
                _PC += offset;
            }
            break;
        case 0x39: // ADD HL, SP
            uint16_t result = getSP() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getSP() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;
        case 0x3A: // LD A, (HL-)
            _A = _mem.read(getHL() - 1);
            break;
        case 0x3B: // DEC SP
            setSP(getSP() - 1);
            break;
        case 0x3C: // INC A WITH FLAGS
            setINCFlags(_A);
            break;
        case 0x3D: // DEC A WITH FLAGS
            setDECFlags(_A);
            break;
        case 0x3E: // LD A, d8
            _A = fetch8();
            break;
        case 0x3F: // CCF
            _F &= ~(0x60);
            _F ^= 0x10;
            break;
        case 0x40: // LD B, B
            break;
        case 0x41: // LD B, C
            _B = _C;
            break;
        case 0x42: // LD B, D
            _B = _D;
            break;
        case 0x43: // LD B, E
            _B = _E;
            break;
        case 0x44: // LD B, H
            _B = _H;
            break;
        case 0x45: // LD B, L
            _B = _L;
            break;
        case 0x46: // LD B, (HL)
            uint8_t val = _mem.read(getHL());
            _B = val;
            break;
        case 0x47: // LD B, A
            _B = _A;
            break;
        case 0x48: // LD C, B
            _C = _B;
            break;
        case 0x49: // LD C, C
            break;
        case 0x4A: // LD C, D
            _C = _D;
            break;
        case 0x4B: // LD C, E
            _C = _E;
            break;
        case 0x4C: // LD C, H
            _C = _H;
            break;
        case 0x4D: // LD C, L
            _C = _L;
            break;
        case 0x4E: // LD C, (HL)
            uint8_t val = _mem.read(getHL());
            _C = val;
            break;
        case 0x4F: // LD C, A
            _C = _A;
            break;
        case 0x50: // LD D, B
            _D = _B;
            break;
        case 0x51: // LD D, C
            _D = _C;
            break;
        case 0x52: // LD D, D
            break;
        case 0x53: // LD D, E
            _D = _E;
            break;  
        case 0x54: // LD D, H
            _D = _H;
            break;
        case 0x55: // LD D, L
            _D = _L;
            break;
        case 0x56: // LD D, (HL)
            uint8_t val = _mem.read(getHL());
            _D = val;
            break;
        case 0x57: // LD D, A
            _D = _A;
            break;
        case 0x58: // LD E, B
            _E = _B;
            break;
        case 0x59: // LD E, C
            _E = _C;
            break;
        case 0x5A: // LD E, D
            _E = _D;
            break;
        case 0x5B: // LD E, E
            break;
        case 0x5C: // LD E, H
            _E = _H;
            break;
        case 0x5D: // LD E, L
            _E = _L;
            break;
        case 0x5E: // LD E, (HL)
            uint8_t val = _mem.read(getHL());
            _E = val;
            break;
        case 0x5F: // LD E, (HL)
            _E = _A;
            break;
        case 0x60: // LD H, B
            _H = _B;
            break;
        case 0x61: // LD H, C
            _H = _C;
            break;
        case 0x62: // LD H, D
            _H = _D;
            break;
        case 0x63: // LD H, E
            _H = _E;
            break;
        case 0x64: // LD H, H
            break;
        case 0x65: // LD H, L
            _H = _L;
            break;
        case 0x66: // LD H, (HL)
            uint8_t val = _mem.read(getHL());
            _H = val;
            break;
        case 0x67: // LD H, A
            _H = _A;
        case 0x68: // LD L, B
            _L = _B;
            break;
        case 0x69: // LD L, C
            _L = _C;
            break;
        case 0x6A: // LD L, D
            _L = _D;
            break;
        case 0x6B: // LD L, E
            _L = _E;
            break;
        case 0x6C: // lD L , H
            _L = _H;
            break;
        case 0x6D: // LD L, L
            break;
        case 0x6E: // LD L, (HL)
            uint8_t val = _mem.read(getHL());
            break;
        case 0x6F: // LD L, A
            _L = _A;
            break;
        case 0x70: // LD (HL), B
            _mem.write(_mem.read(getHL()), _B);
            break;
        case 0x71: // LD (HL), C
            _mem.write(_mem.read(getHL()), _C);
            break;
        case 0x72: // LD (HL), D
            _mem.write(_mem.read(getHL()), _D);
            break;
        case 0x73: // LD (HL), E
            _mem.write(_mem.read(getHL()), _E);
            break;
        case 0x74: // LD (HL), H
            _mem.write(_mem.read(getHL()), _H);
            break;
        case 0x75: // LD (HL), L
            _mem.write(_mem.read(getHL()), _L);
            break;
        case 0x76: // HALT
            // TODO: Figure out what IME flag is and how it works then implement this
        case 0x77: // LD (HL), A
            _mem.write(_mem.read(getHL()), _A);
            break;
        case 0x78: // LD A, B
            _A = _B;
            break;
        case 0x79: // LD A, C
            _A = _C;
            break;
        case 0x7A: // LD A, D
            _A = _D;
            break;
        case 0x7B: // LD A, E
            _A = _E;
            break;
        case 0x7C: // LD A, H
            _A = _H;
            break;
        case 0x7D: // LD A, L
            _A = _L;
            break;
        case 0x7E: // LD A, (HL)
            uint8_t val = _mem.read(getHL());
            _A = val;
            break;
        case 0x7F: // LD A, A
            break;
        case 0x80: // ADD A,B
            setADDFlags(_B);
            break;
        case 0x81: // ADD A, C
            setADDFlags(_C);
            break;
        case 0x82: // ADD A, D
            setADDFlags(_D);
            break;
        case 0x83: // ADD A, E
            setADDFlags(_E);
            break;
        case 0x84: // ADD A, H
            setADDFlags(_H);
            break;
        case 0x85: // ADD A, L
            setADDFlags(_L);
            break;
        case 0x86: // ADD A, (HL)
            uint8_t val = _mem.read(getHL());
            setADDFlags(val);
            break;
        case 0x87: // ADD A, A
            setADDFlags(_A);
            break;
        case 0x88: // ADC A, B
            setADCFlags(_B);
            break;
        case 0x89: // ADC A, C
            setADCFlags(_C);
            break;
        case 0x8A: // ADC A, D
            setADCFlags(_D);
            break;
        case 0x8B: // ADC A, E
            setADCFlags(_E);
            break;
        case 0x8C: // ADC A, H
            setADCFlags(_H);
            break;
        case 0x8D: // ADC A, L
            setADCFlags(_L); 
            break;
        case 0x8E: // ADC A, (HL)
            uint8_t val = _mem.read(getHL());
            setADCFlags(val);
            break;
        case 0x8F: // ADC A, A
            setADCFlags(_A);
            break;
        case 0x90: // SUB B
            setSUBFlags(_B);
            break;
        case 0x91: // SUB C
            setSUBFlags(_C);
            break;
        case 0x92: // SUB D
            setSUBFlags(_D);
            break;
        case 0x93: // SUB E
            setSUBFlags(_E);
            break;
        case 0x94: // SUB H
            setSUBFlags(_H);
            break;
        case 0x95: // SUB L
            setSUBFlags(_L);
            break;
        case 0x96: // SUB (HL)
            uint8_t val = _mem.read(getHL());
            setSUBFlags(val);
            break;
        case 0x97: // SUB A
            setSUBFlags(_A);
            break;
        case 0x98: // SBC B
            setSBCFlags(_B);
            break;
        case 0x99: // SBC C
            setSBCFlags(_C);
            break;
        case 0x9A: // SBC D
            setSBCFlags(_D);
            break;
        case 0x9B: // SBC E
            setSBCFlags(_E);
            break;
        case 0x9C: // SBC H
            setSBCFlags(_H);
            break;
        case 0x9D: // SBC L
            setSBCFlags(_L);
            break;
        case 0x9E: // SBC (HL)
            uint8_t val = _mem.read(getHL());
            setSBCFlags(val);
            break;
        case 0x9F: // SBC A
            setSBCFlags(_A);
            break;
        case 0xA0: // AND B
            setANDFlags(_B);
            break;
        case 0xA1: // AND C
            setANDFlags(_C);
            break;
        case 0xA2: // AND D
            setANDFlags(_D);
            break;
        case 0xA3: // AND E
            setANDFlags(_E);
            break;
        case 0xA4: // AND H
            setANDFlags(_H);
            break;
        case 0xA5: // and L
            setANDFlags(_L);
            break;
        case 0xA6: // AND (HL)
            uint8_t val = _mem.read(getHL());
            setANDFlags(val);
            break;
        case 0xA7: // AND A
            setANDFlags(_A);
            break;
        case 0xA8: // XOR B
            setXORFlags(_B);
            break;
        case 0xA9: // XOR C
            setXORFlags(_C);
            break;
        case 0xAA: // XOR D
            setXORFlags(_D);
            break;
        case 0xAB: // XOR E
            setXORFlags(_E);
            break;
        case 0xAC: // XOR H
            setXORFlags(_H);
            break;
        case 0xAD: // XOR L
            setXORFlags(_L);
            break;
        case 0xAE: // XOR (HL)
            uint8_t val = _mem.read(getHL());
            setXORFlags(val);
            break;
        case 0xAF: // XOR A
            setXORFlags(_A);
            break;
        case 0xB0: // OR B
            setORFlags(_B);
            break;
        case 0xB1: // OR C
            setORFlags(_C);
            break;
        case 0xB2: // or D
            setORFlags(_D);
            break;
        case 0xB3: // or E
            setORFlags(_E);
            break;
        case 0xB4: // OR H
            setORFlags(_H);
            break;
        case 0xB5: // OR L
            setORFlags(_L);
            break;
        case 0xB6: // or (HL);
            uint8_t val = _mem.read(getHL());
            setORFlags(val);
            break;
        case 0xB7: // OR A
            setORFlags(_A);
            break;
        case 0xB8: // CP B
            setCPFlags(_B);
            break;
        case 0xB9: // CP C
            setCPFlags(_C);
            break;
        case 0xBA: // CP D
            setCPFlags(_D);
            break;
        case 0xBB: // CP E
            setCPFlags(_E);
            break;
        case 0xBC: // CP H
            setCPFlags(_H);
            break;
        case 0xBD: // CP L
            setCPFlags(_L);
            break;
        case 0xBE: // CP (HL)
            uint8_t val = _mem.read(getHL());
            setCPFlags(val);
            break;
        case 0xBF: // CP A
            setCPFlags(_A);
            break;
        case 0xC0: // RET NZ

            // TODO: Implement stack class
            
            if (!(_F & 0x80)) {
                uint16_t addr = fetch16();
                _PC = addr;
            }
    }   
}