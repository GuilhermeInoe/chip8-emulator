#include "c8vm.h"
#include <stdio.h>
#include <stdlib.h>

void VM::VM_Inicializar(uint16_t pc_inicial){
    this->PC = pc_inicial;
}

void VM::VM_CarregarROM(char* arq_rom, uint16_t pc_inicial){

    FILE* rom = fopen(arq_rom, "rb");
    if (rom == NULL) {
        perror("ERRO! arquivo ROM está vazio ou não existe!");
        exit(EXIT_FAILURE);
    }

    fseek(rom, 0, SEEK_END);
    int tam_rom = ftell(rom);
    rewind(rom);

    fread(&this->RAM[pc_inicial], 1, tam_rom, rom);
    if (pc_inicial + tam_rom > sizeof(this->RAM)) {
        fprintf(stderr, "ROM muito grande para a memória da VM! Deve ser menor que 4KB\n");
        exit(EXIT_FAILURE);
    }

    fclose(rom);
}

void VM::VM_ExecutarInstrucao(){
    if (this->PC + 1 >= sizeof(this->RAM)) {
        fprintf(stderr, "ERRO: PC fora dos limites da memória (4KB)\n");
        exit(EXIT_FAILURE);
    }
    uint16_t inst = (this->RAM[this->PC] << 8) 
                    | this->RAM[this->PC+1];

    printf("Instrução: 0x%04X\n", inst);
    this->PC+=2;
    
    uint8_t grupo = inst >> 12;
    uint8_t X     = (inst & 0x0F00) >> 8;
    uint8_t Y     = (inst & 0x00F0) >> 4;
    uint8_t N     = inst & 0x000F;
    uint8_t NN    = inst & 0x00FF;
    uint8_t NNN   = inst & 0x0FFF;

    switch(grupo){
        case 0:
            // CLS: Clear the display.
            if(inst == 0x00E0){
                for(int i = 0; i < 64 * 32; i++){
                    this->DISPLAY[i] = 0;
                }
                break;
            }else if(inst == 0x00EE){
                //RET: The interpreter sets the program counter to the address at the top of the stack, then subtracts 1 from the stack pointer.
                //Adição de validação para evitar falha de segmentação
                if (this->SP == 0) {
                    fprintf(stderr, "ERRO: Stack underflow em RET!\n");
                    exit(EXIT_FAILURE);
                }
                this->SP--;
                this->PC = this->stack[this->SP];
            }
            break;

        case 6:
            this->V[X] = NN;
            break;

        default:
            printf("Grupo não implementado! Instrução: 0x%04X\n",
            inst);
            exit(1); //stdlib
    }
}

void VM::VM_ImprimirRegistradores(){
    printf("PC: 0x%04X I: 0x%04X SP: 0x%02X\n", this->PC, this->I, this->SP);
    for(int i = 0; i < 16; i++) {
        printf("V[%X]: 0x%02X ", i, this->V[i]);
    }
    printf("\n");
}
