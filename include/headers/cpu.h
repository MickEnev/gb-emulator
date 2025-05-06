#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <cstdint>
#include <vector>

class CPU {
    public:
        CPU();
        
        uint16_t getAF() const;
        void setAF(uint16_t val);

        

    private:
    // Registers
        uint8_t _A, _B, _C, _D, _E, _F, _H, _L;

        uint16_t _PC, _SP;
        
};

#endif