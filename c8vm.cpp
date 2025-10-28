#include "c8vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const unsigned char chip8_fontset[80] =
{
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


void VM::VM_Inicializar(uint16_t pc_inicial) {
    this->PC = pc_inicial;
    this->I = 0;
    this->SP = 0;
    this->delay_timer = 0;
    this->sound_timer = 0;

    for (int i = 0; i < 16; i++) this->V[i] = 0;
    for (int i = 0; i < 64 * 32; i++) this->DISPLAY[i] = 0;
    for (int i = 0; i < 4096; i++) this->RAM[i] = 0;
    for (int i = 0; i < 16; i++) this->stack[i] = 0;
    for (int i = 0; i < CHIP8_KEYS; i++) this->KEY[i] = 0; 

    for (int i = 0; i < 80; i++) {
        this->RAM[i] = chip8_fontset[i];
    }
    
    srand(time(NULL));
}

void VM::VM_CarregarROM(char* arq_rom, uint16_t pc_inicial) {
    FILE* rom = fopen(arq_rom, "rb");
    if (!rom) {
        perror("ERRO! arquivo ROM está vazio ou não existe!");
        exit(EXIT_FAILURE);
    }

    fseek(rom, 0, SEEK_END);
    long tam_rom = ftell(rom);
    rewind(rom);

    if (pc_inicial + tam_rom > sizeof(this->RAM)) {
        fprintf(stderr, "ROM muito grande! Deve caber em 4KB\n");
        fclose(rom);
        exit(EXIT_FAILURE);
    }

    size_t lido = fread(&this->RAM[pc_inicial], 1, tam_rom, rom);
    if (lido != tam_rom) {
        fprintf(stderr, "Erro ao ler o arquivo ROM.\n");
        fclose(rom);
        exit(EXIT_FAILURE);
    }
    fclose(rom);

    printf("ROM '%s' carregada (%ld bytes) em 0x%03X\n", arq_rom, tam_rom, pc_inicial);
}

void VM::VM_ProcessarTimers() {
    if (this->delay_timer > 0) this->delay_timer--;
    if (this->sound_timer > 0) this->sound_timer--;
}

void VM::VM_ProcessarInput(uint8_t key_index, bool is_down) {
    if (key_index < CHIP8_KEYS) {
        this->KEY[key_index] = is_down ? 1 : 0;
    }
}

void VM::VM_ExecutarInstrucao() {
    if (this->PC + 1 >= sizeof(this->RAM)) {
        fprintf(stderr, "ERRO: PC fora dos limites da memória (4KB)\n");
        exit(EXIT_FAILURE);
    }

    uint16_t inst = (this->RAM[this->PC] << 8) | this->RAM[this->PC + 1];
    #ifdef DEBUG
    printf("Instrução: 0x%04X\n", inst);
    #endif

    uint8_t grupo = inst >> 12;
    uint8_t X = (inst & 0x0F00) >> 8;
    uint8_t Y = (inst & 0x00F0) >> 4;
    uint8_t N = inst & 0x000F;
    uint8_t NN = inst & 0x00FF;
    uint16_t NNN = inst & 0x0FFF;

    this->PC += 2;

    switch (grupo) {
        case 0:
            if (inst == 0x00E0) {
                for (int i = 0; i < 64 * 32; i++) this->DISPLAY[i] = 0;
            } else if (inst == 0x00EE) {
                if (this->SP == 0) {
                    fprintf(stderr, "Stack underflow em RET!\n");
                    exit(EXIT_FAILURE);
                }
                this->SP--;
                this->PC = this->stack[this->SP];
            } else {
                #ifdef DEBUG
                printf("NOP / SYS não implementado (0x%04X)\n", inst);
                #endif
            }
            break;

        case 1:
            this->PC = NNN;
            break;

        case 2:
            if (this->SP >= 16) {
                fprintf(stderr, "Stack overflow em CALL!\n");
                exit(EXIT_FAILURE);
            }
            this->stack[this->SP++] = this->PC;
            this->PC = NNN;
            break;

        case 3:
            if (this->V[X] == NN) this->PC += 2;
            break;

        case 4:
            if (this->V[X] != NN) this->PC += 2;
            break;

        case 5:
            if (this->V[X] == this->V[Y]) this->PC += 2;
            break;

        case 6:
            this->V[X] = NN;
            break;

        case 7:
            this->V[X] += NN;
            break;

        case 8:
            switch (N) {
                case 0: this->V[X] = this->V[Y]; break;
                case 1: this->V[X] |= this->V[Y]; break;
                case 2: this->V[X] &= this->V[Y]; break;
                case 3: this->V[X] ^= this->V[Y]; break;
                case 4: {
                    uint16_t soma = this->V[X] + this->V[Y];
                    this->V[0xF] = soma > 0xFF;
                    this->V[X] = soma & 0xFF;
                } break;
                case 5: {
                    this->V[0xF] = this->V[X] > this->V[Y];
                    this->V[X] -= this->V[Y];
                } break;
                case 6: 
                    this->V[0xF] = this->V[X] & 0x1;
                    this->V[X] >>= 1;
                    break;
                case 7: {
                    this->V[0xF] = this->V[Y] > this->V[X];
                    this->V[X] = this->V[Y] - this->V[X];
                } break;
                case 0xE:
                    this->V[0xF] = (this->V[X] & 0x80) >> 7;
                    this->V[X] <<= 1;
                    break;
                default:
                    printf("Instrução 8XY%X não implementada.\n", N);
                    exit(EXIT_FAILURE);
            }
            break;

        case 9:
            if (this->V[X] != this->V[Y]) this->PC += 2;
            break;

        case 0xA:
            this->I = NNN;
            break;

        case 0xB:
            this->PC = NNN + this->V[0];
            break;

        case 0xC:
            this->V[X] = (rand() % 256) & NN;
            break;

        case 0xD: {
            uint8_t x_pos = this->V[X] % 64;
            uint8_t y_pos = this->V[Y] % 32;
            this->V[0xF] = 0;

            for (int i = 0; i < N; i++) {
                uint8_t sprite_line = this->RAM[this->I + i];
                for (int j = 0; j < 8; j++) {
                    int x = (x_pos + j);
                    int y = (y_pos + i);

                    if (x >= 64 || y >= 32) continue; 
                    
                    uint8_t sprite_bit = (sprite_line & (0x80 >> j)) != 0;
                    int screen_index = y * 64 + x;

                    if (sprite_bit) {
                        if (this->DISPLAY[screen_index] == 1)
                            this->V[0xF] = 1; 
                        this->DISPLAY[screen_index] ^= 1;
                    }
                }
            }
            break;
        }

        case 0xE:
            switch (NN) {
                case 0x9E: 
                    if (this->V[X] < CHIP8_KEYS && this->KEY[this->V[X]] == 1) {
                        this->PC += 2;
                    }
                    break;
                case 0xA1: 
                    if (this->V[X] < CHIP8_KEYS && this->KEY[this->V[X]] == 0) {
                        this->PC += 2;
                    }
                    break;
                default:
                    printf("Instrução Ex%02X não implementada.\n", NN);
                    exit(EXIT_FAILURE);
            }
            break;

        case 0xF:
            switch (NN) {
                case 0x07: this->V[X] = this->delay_timer; break;
                case 0x0A: 
                    {
                        bool key_pressed = false;
                        for (int i = 0; i < CHIP8_KEYS; ++i) {
                            if (this->KEY[i] == 1) {
                                this->V[X] = i; 
                                key_pressed = true;
                                break;
                            }
                        }
                        
                        if (!key_pressed) {
                            this->PC -= 2; 
                        }
                    }
                    break;
                case 0x15: this->delay_timer = this->V[X]; break;
                case 0x18: this->sound_timer = this->V[X]; break;
                case 0x1E: this->I += this->V[X]; break;
                case 0x29: this->I = this->V[X] * 5; break;
                case 0x33:
                    this->RAM[this->I]     = this->V[X] / 100;
                    this->RAM[this->I + 1] = (this->V[X] / 10) % 10;
                    this->RAM[this->I + 2] = this->V[X] % 10;
                    break;
                case 0x55:
                    for (int i = 0; i <= X; i++) this->RAM[this->I + i] = this->V[i];
                    break;
                case 0x65:
                    for (int i = 0; i <= X; i++) this->V[i] = this->RAM[this->I + i];
                    break;
                default:
                    printf("Instrução Fx%02X não implementada.\n", NN);
                    exit(EXIT_FAILURE);
            }
            break;

        default:
            printf("Grupo não implementado! Instrução: 0x%04X\n", inst);
            exit(EXIT_FAILURE);
    }
}


void VM::VM_ImprimirRegistradores() {
    printf("PC: 0x%04X I: 0x%04X SP: 0x%02X\n", this->PC, this->I, this->SP);
    for (int i = 0; i < 16; i++) printf("V[%X]: 0x%02X ", i, this->V[i]);
    printf("\n");
}
