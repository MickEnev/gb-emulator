#include <SDL3/SDL.h>
#include <iostream>
#include "cpu.h"

int main(int argc, char* argv[]) {
    
    std::cout << "hello world" << std::endl;
    CPU cpu;
    std::cout << cpu.getA() << std::endl;
    cpu.setA(0b1111);
    std::cout << cpu.getA() << std::endl;
    return 0;
}