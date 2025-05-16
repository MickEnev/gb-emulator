#include "cpu.h"

CPU::CPU() {
    _PC = 0x0100;
    _SP = 0xFFFE;
    _A = 0x01;
    _F = 0xB0;
    _B = 0x00;
    _C = 0x13;
    _D = 0x00;
    _E = 0xD8;
    _H = 0x01;
    _L = 0x4D;

    _IME = false;
    _imeScheduled = false;
    _halted = false;
    _stopped = false;
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
    // Handle interrupts first
    if (_IME && interruptPending()) {
        serviceInterrupt();
        return;
    }

    if (_halted) {
        // Wake up if an interrupt is pending (even if IME is off)
        if (interruptPending()) {
            _halted = false;
        }
        return;
    }

    uint16_t oldPC = _PC;
    uint8_t opcode = fetch8();

    //std::cout << "Executing opcode: " << std::hex << (int)opcode << " at PC: " << oldPC << std::endl;
    executeOpcode(opcode);

    //std::cout << "After executing opcode: " << std::hex << (int)opcode << " PC: " << _PC << std::endl;

    // Enable interrupts if EI was just executed
    if (_imeScheduled) {
        _IME = true;
        _imeScheduled = false;
    }
}

void CPU::serviceInterrupt() {
    uint8_t IE = _mem.read(0xFFFF);
    uint8_t IF = _mem.read(0xFF0F);
    uint8_t triggered = IE & IF;

    if (triggered == 0) return;

    for (int i = 0; i < 5; ++i) {
        if (triggered & (1 << i)) {
            _IME = false;
            _mem.write(0xFF0F, IF & ~(1 << i)); // Clear the interrupt flag
            push16(_PC); // Save current PC
            _PC = 0x40 + i * 0x08; // Jump to the interrupt vector
            return;
        }
    }
}

uint8_t CPU::fetch8() {
    return _mem.read(_PC++);
}

uint16_t CPU::fetch16() {
    uint8_t low = fetch8();
    uint8_t high = fetch8();
    return (high << 8) | low;
}

void CPU::push16(uint16_t val) {
    _SP--;
    _mem.write(_SP, (val & 0xFF));         // Low byte
    _SP--;
    _mem.write(_SP, (val >> 8) & 0xFF);    // High byte
}

uint16_t CPU::pop16() {
    uint8_t low = _mem.read(_SP++);
    uint8_t high = _mem.read(_SP++);
    return (high << 8) | low; // ✅ Correct
}

void CPU::setINCFlags(uint8_t& r) {
    uint8_t result = r + 1;

    _F &= 0x10; // Keep C flag only, clear Z, N, H

    if ((r & 0x0F) + 1 > 0x0F) {
        _F |= 0x20; // H = 1 if lower nibble overflowed
    }

    if (result == 0) {
        _F |= 0x80; // Z = 1 if result is 0
    }

    // N is cleared always by INC
    // (we already cleared it above)

    r = result;
}

void CPU::setDECFlags(uint8_t& r) {
    uint8_t old = r;
    r--;

    _F &= 0x10; // Keep C flag, clear Z, N, H

    if (r == 0) _F |= 0x80; // Z
    _F |= 0x40;             // N = 1

    if ((old & 0x0F) == 0x00) _F |= 0x20; // H if borrow from bit 4
}

void CPU::setADDFlags(uint8_t& r) {
    uint16_t result = _A + r;

    _F = 0;
    if ((result & 0xFF) == 0) _F |= 0x80; // Z
    if (((_A & 0xF) + (r & 0xF)) > 0xF) _F |= 0x20; // H
    if (result > 0xFF) _F |= 0x10; // C

    _A = result & 0xFF;
}

void CPU::setADCFlags(uint8_t& r) {
    uint8_t carry = (_F & 0x10) ? 1 : 0;
    uint16_t result = _A + r + carry;

    _F = 0;
    if ((result & 0xFF) == 0) _F |= 0x80;
    if (((_A & 0xF) + (r & 0xF) + carry) > 0xF) _F |= 0x20;
    if (result > 0xFF) _F |= 0x10;

    _A = result & 0xFF;
}

void CPU::setSUBFlags(uint8_t& r) {
    uint16_t result = _A - r;

    _F = 0x40; // N = 1

    if ((result & 0xFF) == 0) _F |= 0x80;
    if ((_A & 0xF) < (r & 0xF)) _F |= 0x20;
    if (_A < r) _F |= 0x10;

    _A = result & 0xFF;
}

void CPU::setSBCFlags(uint8_t& r) {
    uint8_t carry = (_F & 0x10) ? 1 : 0;
    uint16_t result = _A - r - carry;

    _F = 0x40; // N = 1

    if ((result & 0xFF) == 0) _F |= 0x80;
    if ((_A & 0xF) < ((r & 0xF) + carry)) _F |= 0x20;
    if (_A < (r + carry)) _F |= 0x10;

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
    _F = 0x40; // N = 1

    if ((_A - r) == 0) _F |= 0x80;
    if ((_A & 0xF) < (r & 0xF)) _F |= 0x20;
    if (_A < r) _F |= 0x10;
}

void CPU::RLC(uint8_t& r) {
    uint8_t carry = (r & 0x80) >> 7;   // Get bit 7
            r = (r << 1) | carry;             // Rotate left circular 

            _F = 0; // Clear the upper 4 F bits which house the flags

            if (r == 0) {
                _F |= (1 << 7);
            }

            if (carry) { // If carry set the 000*0000 to 1
                _F |= (1 << 4);
            }
}

void CPU::RL(uint8_t& r) {
        uint8_t carry = (r & 0x80) >> 7; // Get the highest bit
        uint8_t old_carry = (_F & 0x10) >> 4;

        r = (r << 1) | old_carry;

        _F = 0;

        if (r == 0) {
            _F |= (1 << 7);
        }

        if (carry) {
            _F |= (1 << 4);
        }
}

void CPU::RRC(uint8_t& r) {
    uint8_t carry = (r & 0x01);
    r = (r >> 1) | (carry << 7);

    _F = 0;

    if (r == 0) {
        _F |= (1 << 7);
    }

    if (carry) {
        _F |= (1 << 4);
    }
}

void CPU::RR(uint8_t& r) {
    uint8_t carry = (r & 0x01);
    uint8_t old_carry = (_F & 0x10) >> 4;

    r = (r >> 1) | (old_carry << 7);

    _F = 0;

    if (r == 0) {
        _F |= (1 << 7);
    }

    if (carry) {
        _F |= (1 << 4);
    }
}

void CPU::SLA(uint8_t& r) {
    uint8_t carry = (r & 0x80) >> 7; // Get the highest bit
    r <<= 1;
    _F = 0;

    if (r == 0) {
        _F |= (1 << 7);
    }

    if (carry) {
        _F |= (1 << 4);
    }
}

void CPU::SRA(uint8_t& r) {
    uint8_t carry = (r & 0x01);
    uint8_t bit7 = r & 0x80;
    r >>= 1;
    r |= bit7;
    _F = 0;

    if (r == 0) {
        _F |= (1 << 7);
    }

    if (carry) {
        _F |= (1 << 4);
    }
}

void CPU::SRL(uint8_t& r) {
    uint8_t carry = (r & 0x01);
    r >>= 1;
    _F = 0;

    if (r == 0) {
        _F |= (1 << 7);
    }

    if (carry) {
        _F |= (1 << 4);
    }
}

void CPU::SWAP(uint8_t& r) {
    r = (r >> 4) | (r << 4);
    _F = 0;
    if (r == 0) {
        _F |= (1 << 7); // Set Z
    }
}

void CPU::BIT(uint8_t& r, int n) { // Check if bit n is set in r if not set z = 1 || set n = 0 h = 1
    _F &= ~(1 << 7); // Clear Z
    _F &= ~(1 << 6); // Clear H
    _F &= ~(1 << 5); // Clear N


    if (!(r & (1 << n))) {
        _F |= (1 << 7);
    }

    _F |= (1 << 6); // Set H (always set)
}

void CPU::SET(uint8_t& r, int n) {
    r |= (1 << n);
}

void CPU::RES(uint8_t& r, int n) {
    r &= ~(1 << n);
}

bool CPU::interruptPending() {
    uint8_t IE = _mem.read(0xFFFF); // Interrupt Enable Register
    uint8_t IF = _mem.read(0xFF0F); // Interrupt Flag Register
    return (IE & IF) != 0;
}

void CPU::loadROM(const std::vector<uint8_t>& rom) {
    _mem.loadROM(rom);
}

uint8_t CPU::peek(uint16_t addr) const {
    return _mem.read(addr);
}

bool CPU::isHalted() const {
    return _halted;
}

void CPU::executeOpcode(uint8_t opcode) {
    // TODO: Figure out cycles!!
    // Fill with switches and opcodes 
    switch (opcode) {
        case 0x00: // NOP
            _PC++;
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
            {uint8_t carry = (_A & 0x80) >> 7;   // Get bit 7
            _A = (_A << 1) | carry;             // Rotate left circular 

            _F = 0; // Clear the upper 4 F bits which house the flags

            if (_A == 0) {
                _F |= (1 << 7);
            }

            if (carry) { // If carry set the 000*0000 to 1
                _F |= (1 << 4);
            }
            break;}
        case 0x08: // LD (a16), SP
            {uint16_t addr = fetch16();
            uint16_t sp = getSP();
            _mem.write(addr, sp & 0xFF);
            _mem.write(addr + 1, sp >> 8);
            break;}
        case 0x09: // ADD HL, BC
            {uint16_t result = getBC() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getBC() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;}
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
            {uint8_t carry = (_A & 0x01);
            _A = (_A >> 1) | (carry << 7);

            _F = 0;

            if (_A == 0) {
                _F |= (1 << 7);
            }

            if (carry) {
                _F |= (1 << 4);
            }
            break;}
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
            {uint8_t carry = (_A & 0x80) >> 7; // Get the highest bit
            uint8_t old_carry = (_F & 0x10) >> 4;

            _A = (_A << 1) | old_carry;

            _F = 0;
            
            if (_A == 0) {
                _F |= (1 << 7);
            }

            if (carry) {
                _F |= (1 << 4);
            }
            break;}
        case 0x18: // JR r8
            {int8_t offset = (int8_t)fetch8();
            _PC += offset;
            break;}
        case 0x19: // ADD HL, DE
            {uint16_t result = getDE() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getDE() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;}
        case 0x1A: // LD A, (DE)
            _A = _mem.read(getDE());
            break;
        case 0x1B: // DEC DE
            setDE(getDE() - 1);
            break;
        case 0x1C: // INC E
            //std::cout << "E before INC: " << std::hex << (int)_E << "\n";
            setINCFlags(_E);
            /*std::cout << "E after INC: " << std::hex << (int)_E 
                    << " F: " << std::hex << (int)_F 
                    << " Z: " << ((_F & 0x80) ? "1" : "0") << "\n";*/
            break;
        case 0x1D: // DEC E
            _E -= 1;
            break;
        case 0x1E: // LD E, d8
            setDECFlags(_E);
            break;
        case 0x1F: // RRA
            {uint8_t carry = (_A & 0x01);
            uint8_t old_carry = (_F & 0x10) >> 4;

            _A = (_A >> 1) | (old_carry << 7);

            _F = 0;

            if (_A == 0) {
                _F |= (1 << 7);
            }

            if (carry) {
                _F |= (1 << 4);
            }
            break;}
        case 0x20: // JR NZ, r8
            {
            int8_t offset = (int8_t)fetch8();
            if (!(_F & 0x80)) { // If Z == 0, jump
                _PC += offset;
            }
            break;
        }
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
            {int adjustment = 0;
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
            break;}
        case 0x28: // JR Z, r8
            {uint8_t z = (_F & 0x80) >> 7;
            int8_t offset = (int8_t)fetch8();
            if (z) {
                _PC += offset;
            }
            break;}
        case 0x29: // ADD HL, HL
            {uint16_t result = getHL() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getHL() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;}
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
            {uint8_t c = (_F & 0x50) >> 4;
            int8_t offset = (int8_t)fetch8();
            if (!c) {
                _PC += offset;
            }
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setINCFlags(val);
            _mem.write(getHL(), val);
            break;}
        case 0x35: // DEC (HL)
            {uint8_t val = _mem.read(getHL());
            setDECFlags(val);
            _mem.write(getHL(), val);
            break;}
        case 0x36: // LD (HL), d8 COULD BE SOURCE OF ERROR
            {
            uint8_t val = fetch8();
            _mem.write(getHL(), val);
            break;
        }
        case 0x37: // SCF
            _F &= 0x10;
            break;
        case 0x38: // JR C, r8
            {uint8_t z = (_F & 0x10) >> 4;
            int8_t offset = (int8_t)fetch8();
            if (z) {
                _PC += offset;
            }
            break;}
        case 0x39: // ADD HL, SP
            {uint16_t result = getSP() + getHL();

            _F &= ~(0x40); // Clear N flag

            if (result < getHL()) { 
                _F |= 0x10;  // Set C flag
            }

            if ((getHL() & 0x0FFF) + (getSP() &0x0FFF) > 0x0FFF) {
                _F |= 0x20; // Set H flag
            }

            setHL(result);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            _B = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
            _C = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
            _D = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
            _E = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
            _H = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
                _L = val;
            break;}
        case 0x6F: // LD L, A
            _L = _A;
            break;
        case 0x70: // LD (HL), B
            _mem.write(getHL(), _B);
            break;
        case 0x71: // LD (HL), C
            _mem.write(getHL(), _C);
            break;
        case 0x72: // LD (HL), D
            _mem.write(getHL(), _D);
            break;
        case 0x73: // LD (HL), E
            _mem.write(getHL(), _E);
            break;
        case 0x74: // LD (HL), H
            _mem.write(getHL(), _H);
            break;
        case 0x75: // LD (HL), L
            _mem.write(getHL(), _L);
            break;
        case 0x76: // HALT
            // TODO: Figure out what IME flag is and how it works then implement this
            if (!_IME && interruptPending()) {
            // HALT bug could occur here (not implemented yet)

            } else {
        _halted = true;
            }
    break;
        case 0x77: // LD (HL), A
            _mem.write(getHL(), _A);
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
            {uint8_t val = _mem.read(getHL());
            _A = val;
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setADDFlags(val);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setADCFlags(val);}
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
            {uint8_t val = _mem.read(getHL());
            setSUBFlags(val);}
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
            {uint8_t val = _mem.read(getHL());
            setSBCFlags(val);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setANDFlags(val);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setXORFlags(val);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setORFlags(val);
            break;}
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
            {uint8_t val = _mem.read(getHL());
            setCPFlags(val);
            break;}
        case 0xBF: // CP A
            setCPFlags(_A);
            break;
        case 0xC0: // RET NZ
            if (!(_F & 0x80)) {
                _PC = pop16();
            }
            break;
        case 0xC1: // POP BC
            setBC(pop16());
            break;
        case 0xC2: // JP NZ, a16
           { uint16_t addr = fetch16();
            if (!(_F & 0x80)) {
                _PC = addr;
            }
            break;}
        case 0xC3: // JP a16
            _PC = fetch16();
            break;
        case 0XC4: // CALL NZ, a16
            {uint16_t addr = fetch16();
            if (!(_F & 0x80)) {
                push16(_PC);
                _PC = addr;
            }
            break;}
        case 0xC5: // PUSH BC
            push16(getBC());
            break;
        case 0xC6: // ADD A, d8
            {uint8_t val = fetch8();
            setADDFlags(val);
            break;}
        case 0xC7: // RST 00H
            {uint16_t addr = 0x00;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xC8: // RET Z
            if ((_F & 0x80)) {
                _PC = pop16();
            }
            break;
        case 0xC9: // RET 
            {
            uint16_t ret = peek(_SP) | (peek(_SP + 1) << 8);
            std::cout << "RET to: 0x" << std::hex << ret << " from SP: 0x" << _SP << "\n";
            _PC = pop16();
            break;}
        case 0xCA: // JP Z, a16
            if (!(_F & 0x80)) {
                _PC = fetch16();
            }
            break;
        case 0xCB: // PREFIX CB
            {uint8_t cb_opcode = fetch8();
            switch (cb_opcode & 0x00FF) {
                case 0x00:
                    RLC(_B);
                    break;
                case 0x01:
                    RLC(_C);
                    break;
                case 0x02:
                    RLC(_D);
                    break;
                case 0x03: 
                    RLC(_E);
                    break;
                case 0x04:
                    RLC(_H);
                    break;
                case 0x05:
                    RLC(_L);
                    break;
                case 0x06:
                    {uint8_t val = _mem.read(getHL());
                    RLC(val);
                    break;}
                case 0x07:
                    RLC(_A);
                    break;
                case 0x08:
                    RRC(_B);
                    break;
                case 0x09:
                    RRC(_C);
                    break;
                case 0x0A:
                    RRC(_D);
                    break;
                case 0x0B:
                    RRC(_E);
                    break;
                case 0x0C:
                    RRC(_H);
                    break;
                case 0x0D:
                    RRC(_L);
                    break;
                case 0x0E:
                    {uint8_t val = _mem.read(getHL());
                    RRC(val);
                    break;}
                case 0x0F:
                    RRC(_A);
                    break;
                case 0x10:
                    RL(_B);
                    break;
                case 0x11:
                    RL(_C);
                    break;
                case 0x12:
                    RL(_D);
                    break;
                case 0x13:
                    RL(_E);
                    break;
                case 0x14:
                    RL(_H);
                    break;
                case 0x15:
                    RL(_L);
                    break;
                case 0x16:
                    {uint8_t val = _mem.read(getHL());
                    RL(val);
                    break;}
                case 0x17:
                    RL(_A);
                    break;
                case 0x18:
                    RR(_B);
                    break;
                case 0x19:
                    RR(_C);
                    break;
                case 0x1A:
                    RR(_D);
                    break;
                case 0x1B:
                    RR(_E);
                    break;
                case 0x1C:
                    RR(_H);
                    break;
                case 0x1D:
                    RR(_L);
                    break;
                case 0x1E:
                    {uint8_t val = _mem.read(getHL());
                    RR(val);
                    break;}
                case 0X1F:
                    RR(_A);
                    break;
                case 0x20:
                    SLA(_B);
                    break;
                case 0x21:
                    SLA(_C);
                    break;
                case 0x22:
                    SLA(_D);
                    break;
                case 0x23:
                    SLA(_E);
                    break;
                case 0x24:
                    SLA(_H);
                    break;
                case 0x25:
                    SLA(_L);
                    break;
                case 0x26:
                    {uint8_t val = _mem.read(getHL());
                    SLA(val);
                    break;}
                case 0x27:
                    SLA(_A);
                    break;
                case 0x28:
                    SRA(_B);
                    break;
                case 0x29:
                    SRA(_C);
                    break;
                case 0x2A:
                    SRA(_D);
                    break;
                case 0x2B:
                    SRA(_E);
                    break;
                case 0x2C:
                    SRA(_H);
                    break;
                case 0x2D:
                    SRA(_L);
                    break;
                case 0x2E:
                    {uint8_t val = _mem.read(getHL());
                    SRA(val);
                    break;}
                case 0x2F:
                    SRA(_A);
                    break;
                case 0x30:
                    SWAP(_B);
                    break;
                case 0x31: 
                    SWAP(_C);
                    break;
                case 0x32:
                    SWAP(_D);
                    break;
                case 0x33:
                    SWAP(_E);
                    break;
                case 0x34:
                    SWAP(_H);
                    break;
                case 0x35:
                    SWAP(_L);
                    break;
                case 0x36:
                    {uint8_t val = _mem.read(getHL());
                    SWAP(val);
                    break;}
                case 0x37:
                    SWAP(_A);
                    break;
                case 0x38:
                    SRL(_B);
                    break;
                case 0x39:
                    SRL(_C);
                    break;
                case 0x3A:
                    SRL(_D);
                    break;
                case 0x3B:
                    SRL(_E);
                    break;
                case 0x3C:
                    SRL(_H);
                    break;
                case 0x3D:
                    SRL(_L);
                    break;
                case 0x3E:
                    {uint8_t val = _mem.read(getHL());
                    SRL(val);
                    break;}
                case 0x3F:
                    SRL(_A);
                    break;
                case 0x40: 
                    BIT(_B, 0);
                    break;
                case 0x41:
                    BIT(_C, 0);
                    break;
                case 0x42:
                    BIT(_D, 0);
                    break;
                case 0x43:
                    BIT(_E, 0);
                    break;
                case 0x44:
                    BIT(_H, 0);
                    break;
                case 0x45:
                    BIT(_L, 0);
                    break;
                case 0x46:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 0);
                    break;}
                case 0x47:
                    BIT(_A, 0);
                    break;
                case 0x48: 
                    BIT(_B, 1);
                    break;
                case 0x49:
                    BIT(_C, 1);
                    break;
                case 0x4A:
                    BIT(_D, 1);
                    break;
                case 0x4B:
                    BIT(_E, 1);
                    break;
                case 0x4C:
                    BIT(_H, 1);
                    break;
                case 0x4D:
                    BIT(_L, 1);
                    break;
                case 0x4E:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 1);
                    break;}
                case 0x4F:
                    BIT(_A, 1);
                    break;
                case 0x50: 
                    BIT(_B, 2);
                    break;
                case 0x51:
                    BIT(_C, 2);
                    break;
                case 0x52:
                    BIT(_D, 2);
                    break;
                case 0x53:
                    BIT(_E, 2);
                    break;
                case 0x54:
                    BIT(_H, 2);
                    break;
                case 0x55:
                    BIT(_L, 2);
                    break;
                case 0x56:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 2);
                    break;}
                case 0x57:
                    BIT(_A, 2);
                    break;
                case 0x58: 
                    BIT(_B, 3);
                    break;
                case 0x59:
                    BIT(_C, 3);
                    break;
                case 0x5A:
                    BIT(_D, 3);
                    break;
                case 0x5B:
                    BIT(_E, 3);
                    break;
                case 0x5C:
                    BIT(_H, 3);
                    break;
                case 0x5D:
                    BIT(_L, 3);
                    break;
                case 0x5E:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 3);
                    break;}
                case 0x5F:
                    BIT(_A, 3);
                    break;
                case 0x60: 
                    BIT(_B, 4);
                    break;
                case 0x61:
                    BIT(_C, 4);
                    break;
                case 0x62:
                    BIT(_D, 4);
                    break;
                case 0x63:
                    BIT(_E, 4);
                    break;
                case 0x64:
                    BIT(_H, 4);
                    break;
                case 0x65:
                    BIT(_L, 4);
                    break;
                case 0x66:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 4);
                    break;}
                case 0x67:
                    BIT(_A, 4);
                    break;
                case 0x68: 
                    BIT(_B, 5);
                    break;
                case 0x69:
                    BIT(_C, 5);
                    break;
                case 0x6A:
                    BIT(_D, 5);
                    break;
                case 0x6B:
                    BIT(_E, 5);
                    break;
                case 0x6C:
                    BIT(_H, 5);
                    break;
                case 0x6D:
                    BIT(_L, 5);
                    break;
                case 0x6E:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 5);
                    break;}
                case 0x6F:
                    BIT(_A, 5);
                    break;
                case 0x70: 
                    BIT(_B, 6);
                    break;
                case 0x71:
                    BIT(_C, 6);
                    break;
                case 0x72:
                    BIT(_D, 6);
                    break;
                case 0x73:
                    BIT(_E, 6);
                    break;
                case 0x74:
                    BIT(_H, 6);
                    break;
                case 0x75:
                    BIT(_L, 6);
                    break;
                case 0x76:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 6);
                    break;}
                case 0x77:
                    BIT(_A, 6);
                    break;
                case 0x78: 
                    BIT(_B, 7);
                    break;
                case 0x79:
                    BIT(_C, 7);
                    break;
                case 0x7A:
                    BIT(_D, 7);
                    break;
                case 0x7B:
                    BIT(_E, 7);
                    break;
                case 0x7C:
                    BIT(_H, 7);
                    break;
                case 0x7D:
                    BIT(_L, 7);
                    break;
                case 0x7E:
                    {uint8_t val = _mem.read(getHL());
                    BIT(val, 7);
                    break;}
                case 0x7F:
                    BIT(_A, 7);
                    break;
                case 0x80:
                    RES(_B, 0);
                    break;
                case 0x81:
                    RES(_C, 0);
                    break;
                case 0x82:
                    RES(_D, 0);
                    break;
                case 0x83: 
                    RES(_E, 0);
                    break;
                case 0x84:
                    RES(_H, 0);
                    break;
                case 0x85:
                    RES(_L, 0);
                    break;
                case 0x86:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 0);
                    break;}
                case 0x87:
                    RES(_A, 0);
                    break;
                case 0x88:
                    RES(_B, 1);
                    break;
                case 0x89:
                    RES(_C, 1);
                    break;
                case 0x8A:
                    RES(_D, 1);
                    break;
                case 0x8B:
                    RES(_E, 1);
                    break;
                case 0x8C:
                    RES(_H, 1);
                    break;
                case 0x8D:
                    RES(_L, 1);
                    break;
                case 0x8E:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 1);
                    break;}
                case 0x8F:
                    RES(_A, 1);
                    break;
                case 0x90:
                    RES(_B, 2);
                    break;
                case 0x91:
                    RES(_C, 2);
                    break;
                case 0x92:
                    RES(_D, 2);
                    break;
                case 0x93:
                    RES(_E,2);
                    break;
                case 0x94:
                    RES(_H, 2);
                    break;
                case 0x95:
                    RES(_L, 2);
                    break;
                case 0x96:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 2);
                    break;}
                case 0x97:
                    RES(_A, 2);
                    break;
                case 0x98:
                    RES(_B, 3);
                    break;
                case 0x99:
                    RES(_C, 3);
                    break;
                case 0x9A:
                    RES(_D, 3);
                    break;
                case 0x9B:
                    RES(_E, 3);
                    break;
                case 0x9C:
                    RES(_H, 3);
                    break;
                case 0x9D:
                    RES(_L, 3);
                    break;
                case 0x9E:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 3);
                    break;}
                case 0x9F:
                    RES(_A, 3);
                    break;
                case 0xA0:
                    RES(_B, 4);
                    break;
                case 0xA1:
                    RES(_C, 4);
                    break;
                case 0xA2:
                    RES(_D, 4);
                    break;
                case 0xA3:
                    RES(_E, 4);
                    break;
                case 0xA4:
                    RES(_H, 4);
                    break;
                case 0xA5:
                    RES(_L, 4);
                    break;
                case 0xA6:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 4);
                    break;}
                case 0xA7:
                    RES(_A, 4);
                    break;
                case 0xA8:
                    RES(_B, 5);
                    break;
                case 0xA9:
                    RES(_C, 5);
                    break;
                case 0xAA:
                    RES(_D, 5);
                    break;
                case 0xAB:
                    RES(_E, 5);
                    break;
                case 0xAC:
                    RES(_H, 5);
                    break;
                case 0xAD:
                    RES(_L, 5);
                    break;
                case 0xAE:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 5);
                    break;}
                case 0xAF:
                    RES(_A, 5);
                    break;
                case 0xB0:
                    RES(_B, 6);
                    break;
                case 0xB1:
                    RES(_C, 6);
                    break;
                case 0xB2:
                    RES(_D, 6);
                    break;
                case 0xB3:
                    RES(_E, 6);
                    break;
                case 0xB4:
                    RES(_H, 6);
                    break;
                case 0xB5:
                    RES(_L, 6);
                    break;
                case 0xB6:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 6);
                    break;}
                case 0xB7:
                    RES(_A, 6);
                    break;
                case 0xB8:
                    RES(_B, 7);
                    break;
                case 0xB9:
                    RES(_C, 7);
                    break;
                case 0xBA:
                    RES(_D, 7);
                    break;
                case 0xBB:
                    RES(_E, 7);
                    break;
                case 0xBC:
                    RES(_H, 7);
                    break;
                case 0xBD:
                    RES(_L, 7);
                    break;
                case 0xBE:
                    {uint8_t val = _mem.read(getHL());
                    RES(val, 7);
                    break;}
                case 0xBF:
                    RES(_A, 7);
                    break;
                case 0xC0:
                    SET(_B, 0);
                    break;
                case 0xC1:
                    SET(_C, 0);
                    break;
                case 0xC2:
                    SET(_D, 0);
                    break;
                case 0xC3:
                    SET(_E, 0);
                    break;
                case 0xC4:
                    SET(_H, 0);
                    break;
                case 0xC5:
                    SET(_L, 0);
                    break;
                case 0xC6:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 0);
                    break;}
                case 0xC7:
                    SET(_A, 0);
                    break;
                case 0xC8:
                    SET(_B, 1);
                    break;
                case 0xC9:
                    SET(_C, 1);
                    break;
                case 0xCA:
                    SET(_D, 1);
                    break;
                case 0xCB:
                    SET(_E, 1);
                    break;
                case 0xCC:
                    SET(_H, 1);
                    break;
                case 0xCD:
                    SET(_L, 1);
                    break;
                case 0xCE:
                    {uint8_t val = _mem.read(getHL());
                    SET(val ,1);
                    break;}
                case 0xCF:
                    SET(_A, 1);
                    break;
                case 0xD0:
                    SET(_B, 2);
                    break;
                case 0xD1:
                    SET(_C, 2);
                    break;
                case 0xD2:
                    SET(_D, 2);
                    break;
                case 0xD3:
                    SET(_E, 2);
                    break;
                case 0xD4:
                    SET(_H, 2);
                    break;
                case 0xD5:
                    SET(_L, 2);
                    break;
                case 0xD6:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 2);
                    break;}
                case 0xD7:
                    SET(_A, 2);
                    break;
                case 0xD8:
                    SET(_B, 3);
                    break;
                case 0xD9:
                    SET(_C, 3);
                    break;
                case 0xDA:
                    SET(_D, 3);
                    break;
                case 0xDB:
                    SET(_E, 3);
                    break;
                case 0xDC:
                    SET(_H, 3);
                    break;
                case 0xDD:
                    SET(_L, 3);
                    break;
                case 0xDE:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 3);
                    break;}
                case 0xDF:
                    SET(_A, 3);
                    break;
                case 0xE0:
                    SET(_B, 4);
                    break;
                case 0xE1:
                    SET(_C, 4);
                    break;
                case 0xE2:
                    SET(_D, 4);
                    break;
                case 0xE3:
                    SET(_E, 4);
                    break;
                case 0xE4:
                    SET(_H, 4);
                    break;
                case 0xE5:
                    SET(_L, 4);
                    break;
                case 0xE6:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 4);
                    break;}
                case 0xE7:
                    SET(_A, 4);
                    break;
                case 0xE8:
                    SET(_B, 5);
                    break;
                case 0xE9:
                    SET(_C, 5);
                    break;
                case 0xEA:
                    SET(_D, 5);
                    break;
                case 0xEB:
                    SET(_E, 5);
                    break;
                case 0xEC:
                    SET(_H, 5);
                    break;
                case 0xED:
                    SET(_L, 5);
                    break;
                case 0xEE:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 5);
                    break;}
                case 0xEF:
                    SET(_A, 5);
                    break;
                case 0xF0:
                    SET(_B, 6);
                    break;
                case 0xF1:
                    SET(_C, 6);
                    break;
                case 0xF2:
                    SET(_D, 6);
                    break;
                case 0xF3:
                    SET(_E, 6);
                    break;
                case 0xF4:
                    SET(_H, 6);
                    break;
                case 0xF5:
                    SET(_L, 6);
                    break;
                case 0xF6:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 6);
                    break;}
                case 0xF7:
                    SET(_A, 6);
                    break;
                case 0xF8:
                    SET(_B, 7);
                    break;
                case 0xF9:
                    SET(_C, 7);
                    break;
                case 0xFA:
                    SET(_D, 7);
                    break;
                case 0xFB:
                    SET(_E, 7);
                    break;
                case 0xFC:
                    SET(_H, 7);
                    break;
                case 0xFD:
                    SET(_L, 7);
                    break;
                case 0xFE:
                    {uint8_t val = _mem.read(getHL());
                    SET(val, 7);
                    break;}
                case 0xFF:
                    SET(_A, 7);
                    break;
            }
            break;}
            case 0xCC: // CALL Z, a16
            {uint16_t addr = fetch16();
            if ((_F & 0x80)) {
                push16(_PC);
                _PC = addr;
            }
            break;}
        case 0xCD: // CALL a16
            {uint16_t addr = fetch16();
            std::cout << "CALL to: 0x" << std::hex << addr << " from: 0x" << _PC << "\n";
            push16(_PC);
            _PC = addr;
            break;}
        case 0xCE: // ADC A, d8
            {uint8_t val = fetch8();
            setADCFlags(val);
            break;}
        case 0XCF: // RST 08H
            {uint16_t addr = 0x08;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xD0: // RET NC
            if (!(_F & 0x10)) {
                _PC = pop16();
            }
            break;
        case 0xD1: // POP DE
            setDE(pop16());
            break;
        case 0xD2: // JP NC, a16
            {uint16_t addr = fetch16();
            if (!(_F & 0x10)) {
                _PC = addr;
            }
            break;}
        case 0xD4: // CALL NC, a16
            {uint16_t addr = fetch16();
            if (!(_F & 0x10)) {
                push16(_PC);
                _PC = addr;
            }
            break;}
        case 0xD5: // PUSH DE
            push16(getDE());
            break;
        case 0xD6: // SUB d8
            {uint8_t val = fetch8();
            setSUBFlags(val);
            break;}
        case 0xD7: // RST 10H
           { uint16_t addr = 0x10;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xD8: // RET C
            if ((_F & 0x10)) {
                _PC = pop16();
            }
            break;
        case 0xD9: // RETI
            {
                uint8_t low = _mem.read(_SP++);
                uint8_t high = _mem.read(_SP++);
                _PC = (high << 8) | low;
                _IME = true;
                break;
            }
        case 0xDA: // JP C, a16
            {uint16_t addr = fetch16();
            if ((_F & 0x10)) {
                _PC = addr;
            }
            break;}
        case 0xDC: // CALL C, a16
            {uint16_t addr = fetch16();
            if ((_F & 0x10)) {
                push16(_PC);
                _PC = addr;
            }
            break;}
        case 0xDE: // SBC A, d8
            {uint8_t val = fetch8();
            setSBCFlags(val);
            break;}
        case 0xDF: // RST 18H
            {uint16_t addr = 0x18;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xE0: // LDH (a8), A
            _mem.write(0xFF00 + fetch8(), _A);
            break;
        case 0xE1: // POP HL
            setHL(pop16());
            break;
        case 0xE2: // LD (C), A
            _mem.write(0xFF00 + _C, _A);
            break;
        case 0xE5: // PUSH HL
            push16(getHL());
            break;
        case 0xE6: // AND d8
            {uint8_t val = fetch8();
            setANDFlags(val);
            break;}
        case 0xE7: // RST 20H
            {uint16_t addr = 0x20;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xE8: // ADD SP, r8
            {int8_t r8 = (int8_t)fetch8();
            uint16_t result = _SP + r8;
            _F &= 0x10; // Clear all flags except for the carry flag

            // Set the Carry flag if the result exceeds 16-bit bounds (i.e., if there's a carry from bit 8)
            if ((int16_t)(_SP + r8) > 0xFFFF || (int16_t)(_SP + r8) < 0) {
                _F |= 0x10;  // Set the Carry flag
            }

            // Half carry flag check (carry from bit 4)
            if (((_SP & 0xF) + (r8 & 0xF)) > 0xF) {
                _F |= 0x20;  // Set the Half Carry flag
            }

            // Update SP with the result
            _SP = result;
            break;}
        case 0xE9: // JP (HL)
            _PC = getHL();
            break;
        case 0xEA: // LD (a16), A
            _mem.write(fetch16(), _A);
            break;
        case 0xEE: // XOR d8
            {uint8_t val = fetch8();
            setXORFlags(val);
            break;}
        case 0xEF: // RST 28H
            {uint16_t addr = 0x28;
            push16(_PC);
            _PC = addr;
            break;}
        case 0XF0: // LD A, (a8)
            _A = _mem.read(0xFF00 + fetch8());
            break;
        case 0xF1: // POP AF
            setAF(pop16());
            break;
        case 0xF2: // LD A, (C)
            _A = _mem.read(0xFF00 + _C);
            break;
        case 0xF3: // DI
            _IME = false;
            break;
        case 0xF5: // PUSH AF
            push16(getAF());
            break;
        case 0xF6: // OR d8
           { uint8_t val = fetch8();
            setORFlags(val);
            break;}
        case 0xF7: // RST 30H
            {uint16_t addr = 0x30;
            push16(_PC);
            _PC = addr;
            break;}
        case 0xF8: // LD HL, SP+r8
            {
            int8_t r8 = static_cast<int8_t>(fetch8());  // Fetch the signed 8-bit immediate value
            uint32_t result = _SP + r8;  // Add SP and r8
            setHL(result & 0xFFFF);  // Store the lower 16 bits of the result in HL
            // Update the flags:
            _F = 0;  // Clear flags before setting them
            
            if ((result & 0x10000) != 0) {  // Carry out of bit 15 (overflow)
                _F |= 0x10;  // Set the C flag
            }

            // Set the H flag if there is a carry from bit 11 to bit 12 (half carry)
            if (((_SP & 0xFFF) + (r8 & 0xFFF)) & 0x1000) {
                _F |= 0x20;  // Set H flag
            }
        }
            break;
        case 0xF9: // LD SP, HL
            _SP = getHL();
            break;
        case 0xFA: // LD A, (a16)
            _A = _mem.read(0xFF00 + fetch16());
            break;
        case 0xFB: // EI
            _imeScheduled = true;
            break;
        case 0xFE: // CP d8
            {uint8_t val = fetch8();
            setCPFlags(val);
            break;}
        case 0xFF: // RST 38H
            {
                uint16_t addr = 0x38;
            push16(_PC);
            _PC = addr;
            break;
        }
        default:
            std::cerr << "Unhandled opcode: 0x" << std::hex << (int)opcode << " at PC: 0x" << _PC - 1 << "\n";
            _halted = true;
            break;
    }   
}