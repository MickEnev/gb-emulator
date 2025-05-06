#include "memory.h"

Memory::Memory() {
    _mem.fill(0);
}

uint8_t Memory::read(uint16_t address) const {
    return _mem[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    if (address >= _mem.size()) {
        std::cerr << "Memory write otu of bounds at: 0x" << std::hex << address << "\n";
    }
    if (address < 0x8000) {
        std::cerr << "Attempt to write to ROM area at: " << std::hex << address << "\n";
        return;
    }

    _mem[address] = value;
}   

void Memory::loadROM(const std::vector<uint8_t>& rom) {
    size_t loadSize = std::min(rom.size(), static_cast<size_t>(0x8000));
    std::copy(rom.begin(), rom.begin() + loadSize, _mem.begin());
}