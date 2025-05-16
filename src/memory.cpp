#include "memory.h"

Memory::Memory() {
    _mem.fill(0);
}

uint8_t Memory::read(uint16_t address) const {
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (ram_enabled) {
            return ext_ram[address - 0xA000];
        } else {
            return 0xFF; // Open bus behavior
        }
    }
    return _mem[address];
}

void Memory::write(uint16_t address, uint8_t value) {
    // --- RAM enable (for MBC1/MBC3 test ROMs) ---
    if (address >= 0x0000 && address <= 0x1FFF) {
        ram_enabled = (value & 0x0F) == 0x0A;
        return;
    }
    // --- External RAM write ---
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (ram_enabled) {
            ext_ram[address - 0xA000] = value;
        }
        return;
    }

    if (address >= _mem.size()) {
        std::cerr << "Memory write otu of bounds at: 0x" << std::hex << address << "\n";
    }
    if (address < 0x8000) {
        std::cerr << "Attempt to write to ROM area at: " << std::hex << address << "\n";
        return;
    }

    _mem[address] = value;

    if (address == 0xFF02 && (value & 0x81) == 0x81) {
        char c = _mem[0xFF01];
        if (c == 'f') {
            stop = true;
        }
        std::cout << c << std::flush;
        _mem[0xFF02] = 0; // Reset transfer control
    }
    
}   

void Memory::loadROM(const std::vector<uint8_t>& rom) {
    std::cout << "Loading ROM, size: " << rom.size() << " bytes\n";
    size_t loadSize = std::min(rom.size(), static_cast<size_t>(0x8000));
    std::cout << "Loading " << loadSize << " bytes into memory\n";
    std::copy(rom.begin(), rom.begin() + loadSize, _mem.begin());
}