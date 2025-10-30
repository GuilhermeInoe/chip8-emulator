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

    // Limpa registradores, display, pilha e RAM
    for (int i = 0; i < 16; i++) this->V[i] = 0;
    for (int i = 0; i < 64 * 32; i++) this->DISPLAY[i] = 0;
    for (int i = 0; i < 4096; i++) this->RAM[i] = 0;
    for (int i = 0; i < 16; i++) this->stack[i] = 0;
    for (int i = 0; i < CHIP8_KEYS; i++) this->KEY[i] = 0; 

    // Carrega a fonte na RAM (início da memória)
    for (int i = 0; i < 80; i++) {
        this->RAM[i] = chip8_fontset[i];
    }
    
    // Inicializa o gerador de números aleatórios
    srand(time(NULL));
}

void VM::VM_CarregarROM(char* arq_rom, uint16_t pc_inicial) {
    FILE* rom = fopen(arq_rom, "rb");
    if (!rom) {
        perror("Arquivo ROM está vazio ou não existe");
        exit(EXIT_FAILURE);
    }

    fseek(rom, 0, SEEK_END);
    long tam_rom_long = ftell(rom); 
    rewind(rom);

    if (tam_rom_long < 0) {
        fprintf(stderr, "Erro ao determinar o tamanho da ROM.\n");
        fclose(rom);
        exit(EXIT_FAILURE);
    }

    size_t tam_rom = (size_t)tam_rom_long; 

    // Verifica se a ROM cabe na memória a partir do endereço inicial
    if (pc_inicial + tam_rom > sizeof(this->RAM)) {
        fprintf(stderr, "a ROM deve caber em 4KB\n");
        fclose(rom);
        exit(EXIT_FAILURE);
    }

    // Lê a ROM para a RAM
    size_t lido = fread(&this->RAM[pc_inicial], 1, tam_rom, rom);
    
    if (lido != tam_rom) {
        fprintf(stderr, "Erro ao ler o arquivo ROM.\n");
        fclose(rom);
        exit(EXIT_FAILURE);
    }
    fclose(rom);

    printf("ROM '%s' carregada (%zu bytes) em 0x%03X\n", arq_rom, tam_rom, pc_inicial);
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
    // Fazer o cast para 'size_t' antes de comparar com 'sizeof'
    if ((size_t)(this->PC + 1) >= sizeof(this->RAM)) {
        fprintf(stderr, "ERRO: PC fora dos limites da memória (4KB)\n");
        exit(EXIT_FAILURE);
    }

    // FETCH: Busca 2 bytes da memória e junta em uma instrução
    uint16_t inst = (this->RAM[this->PC] << 8) | this->RAM[this->PC + 1];
    #ifdef DEBUG
    printf("Instrução: 0x%04X\n", inst);
    #endif

    // DECODE: Extrai os nibbles e bytes da instrução
    uint8_t grupo = inst >> 12;      // Os 4 bits mais altos (Ex: 0xD...)
    uint8_t X = (inst & 0x0F00) >> 8; // O 2º nibble (Ex: ...X..)
    uint8_t Y = (inst & 0x00F0) >> 4; // O 3º nibble (Ex: ....Y.)
    uint8_t N = inst & 0x000F;       // Os 4 bits mais baixos (Ex: .....N)
    uint8_t NN = inst & 0x00FF;      // Os 8 bits mais baixos (Ex: ...NN)
    uint16_t NNN = inst & 0x0FFF;    // Os 12 bits mais baixos (Ex: .NNN)

    // Incrementa o PC ANTES de executar. Jumps/Calls irão sobrescrever isso.
    this->PC += 2;

    // EXECUTE:
    switch (grupo) {
        case 0:
            if (inst == 0x00E0) {
                // 00E0: Limpa a tela
                for (int i = 0; i < 64 * 32; i++) this->DISPLAY[i] = 0;
            } else if (inst == 0x00EE) {
                // 00EE: Retorna de uma sub-rotina
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
            // 1NNN: Pula para o endereço NNN
            this->PC = NNN;
            break;

        case 2:
            // 2NNN: Chama sub-rotina em NNN
            if (this->SP >= 16) {
                fprintf(stderr, "Stack overflow em CALL!\n");
                exit(EXIT_FAILURE);
            }
            this->stack[this->SP++] = this->PC;
            this->PC = NNN;
            break;

        case 3:
            // 3XNN: Pula a próxima instrução se VX == NN
            if (this->V[X] == NN) this->PC += 2;
            break;

        case 4:
            // 4XNN: Pula a próxima instrução se VX != NN
            if (this->V[X] != NN) this->PC += 2;
            break;

        case 5:
            // 5XY0: Pula a próxima instrução se VX == VY
            if (this->V[X] == this->V[Y]) this->PC += 2;
            break;

        case 6:
            // 6XNN: Define VX = NN
            this->V[X] = NN;
            break;

        case 7:
            // 7XNN: Adiciona NN a VX (VX = VX + NN)
            this->V[X] += NN;
            break;

        case 8:
            // Instruções lógicas e matemáticas (8XYN)
            switch (N) {
                case 0: // 8XY0: VX = VY
                    this->V[X] = this->V[Y]; break;
                case 1: // 8XY1: VX = VX | VY
                    this->V[X] |= this->V[Y]; break;
                case 2: // 8XY2: VX = VX & VY
                    this->V[X] &= this->V[Y]; break;
                case 3: // 8XY3: VX = VX ^ VY
                    this->V[X] ^= this->V[Y]; break;
                case 4: { // 8XY4: VX = VX + VY, VF = carry
                    uint16_t soma = this->V[X] + this->V[Y];
                    this->V[0xF] = soma > 0xFF; // Define VF (carry)
                    this->V[X] = soma & 0xFF;
                } break;
                case 5: { // 8XY5: VX = VX - VY, VF = NOT borrow
                    this->V[0xF] = this->V[X] > this->V[Y]; // Define VF
                    this->V[X] -= this->V[Y];
                } break;
                case 6: // 8XY6: VX = VX >> 1, VF = bit menos significativo
                    this->V[0xF] = this->V[X] & 0x1;
                    this->V[X] >>= 1;
                    break;
                case 7: { // 8XY7: VX = VY - VX, VF = NOT borrow
                    this->V[0xF] = this->V[Y] > this->V[X]; // Define VF
                    this->V[X] = this->V[Y] - this->V[X];
                } break;
                case 0xE: // 8XYE: VX = VX << 1, VF = bit mais significativo
                    this->V[0xF] = (this->V[X] & 0x80) >> 7;
                    this->V[X] <<= 1;
                    break;
                default:
                    printf("Instrução 8XY%X não implementada.\n", N);
                    exit(EXIT_FAILURE);
            }
            break;

        case 9:
            // 9XY0: Pula a próxima instrução se VX != VY
            if (this->V[X] != this->V[Y]) this->PC += 2;
            break;

        case 0xA:
            // ANNN: Define I = NNN
            this->I = NNN;
            break;

        case 0xB:
            // BNNN: Pula para NNN + V0
            this->PC = NNN + this->V[0];
            break;

        case 0xC:
            // CXNN: Define VX = random_byte & NN
            this->V[X] = (rand() % 256) & NN;
            break;

        case 0xD: {
            // DXYN: Desenha sprite em (VX, VY) de altura N
            uint8_t x_pos = this->V[X] % 64;
            uint8_t y_pos = this->V[Y] % 32;
            this->V[0xF] = 0; // Reseta VF (flag de colisão)

            // Itera pelas N linhas do sprite
            for (int i = 0; i < N; i++) {
                uint8_t sprite_line = this->RAM[this->I + i];
                // Itera pelos 8 bits (colunas) da linha
                for (int j = 0; j < 8; j++) {
                    int x = (x_pos + j);
                    int y = (y_pos + i);

                    // Ignora pixels fora da tela
                    if (x >= 64 || y >= 32) continue; 
                    
                    uint8_t sprite_bit = (sprite_line & (0x80 >> j)) != 0;
                    int screen_index = y * 64 + x;

                    if (sprite_bit) {
                        // Se o bit do sprite é 1 e o pixel na tela também é 1
                        if (this->DISPLAY[screen_index] == 1)
                            this->V[0xF] = 1; // Colisão!
                        
                        // Desenha o pixel (XOR)
                        this->DISPLAY[screen_index] ^= 1;
                    }
                }
            }
            break;
        }

        case 0xE:
            // Instruções de teclado
            switch (NN) {
                case 0x9E: 
                    // EX9E: Pula a próxima instrução se a tecla em VX estiver pressionada
                    if (this->V[X] < CHIP8_KEYS && this->KEY[this->V[X]] == 1) {
                        this->PC += 2;
                    }
                    break;
                case 0xA1: 
                    // EXA1: Pula a próxima instrução se a tecla em VX NÃO estiver pressionada
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
            // Funções diversas
            switch (NN) {
                case 0x07: // FX07: VX = delay_timer
                    this->V[X] = this->delay_timer; break;
                case 0x0A: // FX0A: Aguarda pressionar tecla, armazena em VX
                    {
                        bool key_pressed = false;
                        for (int i = 0; i < CHIP8_KEYS; ++i) {
                            if (this->KEY[i] == 1) {
                                this->V[X] = i; 
                                key_pressed = true;
                                break;
                            }
                        }
                        
                        // Se nenhuma tecla foi pressionada, repete esta instrução
                        if (!key_pressed) {
                            this->PC -= 2; 
                        }
                    }
                    break;
                case 0x15: // FX15: delay_timer = VX
                    this->delay_timer = this->V[X]; break;
                case 0x18: // FX18: sound_timer = VX
                    this->sound_timer = this->V[X]; break;
                case 0x1E: // FX1E: I = I + VX
                    this->I += this->V[X]; break;
                case 0x29: // FX29: I = endereço do sprite para o dígito em VX
                    this->I = this->V[X] * 5; break; // (Cada sprite tem 5 bytes)
                case 0x33: // FX33: Armazena BCD de VX em I, I+1, I+2
                    this->RAM[this->I]     = this->V[X] / 100;
                    this->RAM[this->I + 1] = (this->V[X] / 10) % 10;
                    this->RAM[this->I + 2] = this->V[X] % 10;
                    break;
                case 0x55: // FX55: Armazena V0 a VX na memória a partir de I
                    for (int i = 0; i <= X; i++) this->RAM[this->I + i] = this->V[i];
                    break;
                case 0x65: // FX65: Lê V0 a VX da memória a partir de I
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