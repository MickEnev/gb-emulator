#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstdint>
#include <vector>
#include "memory.h"

class CPU {
    public:
        CPU();

        uint16_t getAF() const;
        void setAF(uint16_t val);

        uint16_t getBC() const;
        void setBC(uint16_t val);

        uint16_t getDE() const;
        void setDE(uint16_t val);

        uint16_t getHL() const;
        void setHL(uint16_t val);

        uint16_t getPC() const;
        void setPC(uint16_t val);

        uint16_t getSP() const;
        void setSP(uint16_t val);

        void step();


    private:
        Memory _mem;
        // Registers
        uint8_t _A, _B, _C, _D, _E, _F, _H, _L;

        uint16_t _PC, _SP;

        void executeOpcode(uint16_t opcode);

        uint8_t fetch8();
        uint16_t fetch16();
        
};

#endif