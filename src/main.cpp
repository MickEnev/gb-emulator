#include <SDL3/SDL.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "cpu.h"
#include "memory.h"

std::vector<uint8_t> readROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open ROM file: " << path << "\n";
        exit(1);
    }
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(file)), {});
}

void dumpROM(const std::vector<uint8_t>& rom) {
    for (size_t i = 0; i < rom.size(); ++i) {
        // Print each byte as two‑digit hex
        std::cout
            << std::hex 
            << std::setw(2) 
            << std::setfill('0')
            << static_cast<int>(rom[i])
            << ' ';

        // Optional: break line every 16 bytes
        if ((i + 1) % 16 == 0) 
            std::cout << '\n';
    }
    std::cout << std::dec << '\n';  // back to decimal
}

void trimTrailingZeros(std::vector<uint8_t>& rom) {
    auto it = std::find_if(rom.rbegin(), rom.rend(),
                           [](uint8_t b){ return b != 0x00; });
    rom.erase(rom.begin() + (rom.rend() - it), rom.end());
}

void printMessageFrom(uint16_t startAddr, CPU& cpu) {
    std::cout << "Message:\n";
    for (uint16_t addr = startAddr; addr < 0xC100; ++addr) {
        char c = static_cast<char>(cpu.peek(addr));
        if (c == '\0') break;
        if (std::isprint(c) || c == '\n') {
            std::cout << c;
        }
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    
    CPU cpu;
    std::vector<uint8_t> rom = readROM("ROMS/cpu_instrs.gb");
    std::vector<uint8_t> rom2 = readROM("ROMS/cpu_instrs.gb");
    
    trimTrailingZeros(rom2);
    dumpROM(rom2);

    cpu.loadROM(rom);

    for (int i = 0; i < 2215000; ++i) {
        if (cpu.stop()) {
            std::cout << "lajskhdahs" << std::endl;
            std::cerr << "\n[DEBUG] Serial output started with 'f' — test failed\n";
            break;
        }
        cpu.step();

        if (cpu.isHalted() && !cpu.interruptPending()) {
            std::cout << "CPU halted cleanly with no interrupts.\n";
            break;
        }
    }
    



    return 0;
}