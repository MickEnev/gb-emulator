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
        case 
    }
}