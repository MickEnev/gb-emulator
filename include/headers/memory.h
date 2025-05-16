#ifndef MEMORY_H
#define MEMORY_H


#include <vector>
#include <cstdint>
#include <iostream>
#include <array>


class Memory {
    public:
        static const unsigned int SIZE = 0x10000; // 8 KiB of working RAM
        Memory();
        // Read data
        uint8_t read(uint16_t address) const;
        // Write data
        void write(uint16_t address, uint8_t value);
        void loadROM(const std::vector<uint8_t>& rom); // Check size of roms and make sure all these values are correct

        std::string serial_log;

        bool stop;

        bool ram_enabled = false;
        std::array<uint8_t, 0x2000> ext_ram;
    private:
        std::array<uint8_t, SIZE> _mem;
};
#endif