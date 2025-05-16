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

    if (address >= 0xC000 && address < 0xC080) {
        std::cout << "RAM[0x" << std::hex << address << "] = " << (char)value << "\n";
    }

    _mem[address] = value;

    if (address == 0xFF02 && (value & 0x81) == 0x81) {
        char c = _mem[0xFF01];
        std::cout << c << std::flush;
        _mem[0xFF02] = 0; // Reset transfer control
    }

    
}   

void Memory::loadROM(const std::vector<uint8_t>& rom) {
    std::cout << "Loading ROM, size: " << rom.size() << " bytes\n";
    size_t loadSize = std::min(rom.size(), static_cast<size_t>(0x8000));
    std::cout << "Loading " << loadSize << " bytes into memory\n";
    std::copy(rom.begin(), rom.begin() + loadSize, _mem.begin());

    std::cout << "First 16 bytes of ROM: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << (int)rom[i] << " ";
    }
    std::cout << std::endl;
}