#include <stdint.h>
#include "defs.h" // Inclui o CHIP8_KEYS

//Autores: Guilherme Inoe, Winicius Abilio

class VM {
public:
    uint8_t RAM[4096];      // Memória 4KB
    uint16_t PC;            // Program Counter
    uint8_t V[16];          // Registradores de propósito geral
    uint8_t SP;             // Stack Pointer
    uint16_t I;             // Registrador de índice
    uint16_t stack[16];     // Pilha
    uint8_t DISPLAY[64*32]; // Tela
    uint8_t KEY[CHIP8_KEYS]; // Estado do teclado (novo)
    uint8_t delay_timer;
    uint8_t sound_timer;

    void VM_Inicializar(uint16_t pc_inicial);
    void VM_CarregarROM(char* arq_rom, uint16_t pc_inicial);
    
    void VM_ExecutarInstrucao();

    void VM_ProcessarInput(uint8_t key_index, bool is_down);
    
    void VM_ProcessarTimers(); 

    void VM_ImprimirRegistradores();
};
