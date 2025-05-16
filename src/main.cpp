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

int main(int argc, char* argv[]) {
    
    CPU cpu;
    std::vector<uint8_t> rom = readROM("ROMS/cpu_instrs.gb");
    cpu.loadROM(rom);

 std::cout << "First 16 bytes in memory after loading ROM: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::hex << (int)cpu.peek(i) << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < 100000; ++i) {
    cpu.step();
    if (cpu.isHalted()) break;

    if (i % 10000 == 0) {
        std::string result;
        for (uint16_t addr = 0xC000; addr < 0xC000 + 128; ++addr) {
            char c = (char)cpu.peek(addr);
            if (c == '\0') break;
            if (std::isprint(c) || c == '\n') result += c;
        }

        if (result.find("Passed") != std::string::npos) {
            std::cout << "✅ TEST PASSED!\n";
        } else {
            std::cout << "❌ TEST FAILED or NOT COMPLETED\n";
        }
    }
}

    std::cout << "\n=== Memory Dump @0xC000 ===\n";
    for (uint16_t addr = 0xC000; addr < 0xC000 + 128; ++addr) {
        char c = static_cast<char>(cpu.peek(addr));
        if (std::isprint(c) || c == '\n') std::cout << c;
    }
    std::cout << "\n===========================\n";


    return 0;
}