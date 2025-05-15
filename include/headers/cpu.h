#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <map>
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

        void loadROM(const std::vector<uint8_t>& rom);

        uint8_t peek(uint16_t addr) const;
        bool isHalted() const;


    private:
        Memory _mem;
        // Registers
        uint8_t _A, _B, _C, _D, _E, _F, _H, _L;
        // _F register contains flags: z n h c // zero subtraction half carry carry
        // These live in the upper 4 bits of F the lower 4 bits do not get used

        // Interrupt flag
        bool _IME;
        bool _imeScheduled;

        uint16_t _PC, _SP; // Program counter/ Pointer and Stack Pointer

        void push16(uint16_t val);
        uint16_t pop16();

        bool _stopped = false;
        bool _halted = false;

        bool interruptPending();

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

        // CB Operations

        // Rotates and shifts
        void RLC(uint8_t& r);
        void RL(uint8_t& r);

        void RRC(uint8_t& r);
        void RR(uint8_t& r);

        void SLA(uint8_t& r);
        void SRA(uint8_t& r);
        void SRL(uint8_t& r);

        void SWAP(uint8_t& r);

        // Bit Ops
        void BIT(uint8_t& r, int n);
        void SET(uint8_t& r, int n);
        void RES(uint8_t& r, int n);

        void serviceInterrupt();


        
};

#endif