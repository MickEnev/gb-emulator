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

        // _F register contains flags: z n h c // zero subtraction half carry carry
        // These live in the upper 4 bits of F the lower 4 bits do not get used

        uint16_t _PC, _SP; // Program counter/ Pointer and Stack Pointer

        bool _stopped = false;

        void executeOpcode(uint8_t opcode);

        uint8_t fetch8();
        uint16_t fetch16();

        void setINCFlags(uint8_t& r);
        void setDECFlags(uint8_t& r);

        void setADDFlags(uint8_t& r);
        void setADCFlags(uint8_t& r);

        void setSUBFlags(uint8_t& r);
        void setSBCFlags(uint8_t& r);

        void setANDFlags(uint8_t& r);
        void setXORFlags(uint8_t& r);

        void setORFlags(uint8_t& r);
        void setCPFlags(uint8_t& r);
        
};

#endif