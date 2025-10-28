#include "c8vm.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// Definições de SDL
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 320
#define PIXEL_SIZE 10 // 640/64 = 10, 320/32 = 10
#define CHIP8_FREQUENCY 500 // 500 instruções/segundo é um valor comum
#define TIMER_FREQUENCY 60 // Timers diminuem a 60Hz

// Mapeamento de teclas SDL para CHIP-8 (0-F)
uint8_t key_map[SDL_NUM_SCANCODES] = {0};

void inicializar_key_map() {
    // Mapeamento comum de um teclado QWERTY para o teclado CHIP-8 (0-F)
    key_map[SDL_SCANCODE_1] = 0x1;
    key_map[SDL_SCANCODE_2] = 0x2;
    key_map[SDL_SCANCODE_3] = 0x3;
    key_map[SDL_SCANCODE_4] = 0xC;
    
    key_map[SDL_SCANCODE_Q] = 0x4;
    key_map[SDL_SCANCODE_W] = 0x5;
    key_map[SDL_SCANCODE_E] = 0x6;
    key_map[SDL_SCANCODE_R] = 0xD;
    
    key_map[SDL_SCANCODE_A] = 0x7;
    key_map[SDL_SCANCODE_S] = 0x8;
    key_map[SDL_SCANCODE_D] = 0x9;
    key_map[SDL_SCANCODE_F] = 0xE;
    
    key_map[SDL_SCANCODE_Z] = 0xA;
    key_map[SDL_SCANCODE_X] = 0x0;
    key_map[SDL_SCANCODE_C] = 0xB;
    key_map[SDL_SCANCODE_V] = 0xF;
}

void desenhar_display(SDL_Renderer* renderer, VM* vm) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (vm->DISPLAY[y * 64 + x]) {
                SDL_Rect pixel;
                pixel.x = x * PIXEL_SIZE;
                pixel.y = y * PIXEL_SIZE;
                pixel.w = PIXEL_SIZE;
                pixel.h = PIXEL_SIZE;
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char** argv){
    if (argc < 2) {
        fprintf(stderr, "Execute o comando seguido do arquivo ROM Ex.: %s <arquivo_rom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    VM vm;
    vm.VM_Inicializar(0x200); 
    vm.VM_CarregarROM(argv[1], 0x200);
    inicializar_key_map();

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!window || !renderer) {
        fprintf(stderr, "Erro ao criar janela/renderizador SDL: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    SDL_Event e;
    int running = 1;

    int cycles_per_frame = CHIP8_FREQUENCY / TIMER_FREQUENCY;

    Uint32 timer_delay_ms = 1000 / TIMER_FREQUENCY;

    while(running){

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    running = 0;
                }
                uint8_t chip8_key = key_map[e.key.keysym.scancode];
                if (chip8_key != 0) vm.VM_ProcessarInput(chip8_key, true);
            } else if (e.type == SDL_KEYUP) {
                uint8_t chip8_key = key_map[e.key.keysym.scancode];
                if (chip8_key != 0) vm.VM_ProcessarInput(chip8_key, false);
            }
        }

        for (int i = 0; i < cycles_per_frame; ++i) {
            vm.VM_ExecutarInstrucao();
            #ifdef DEBUG
            vm.VM_ImprimirRegistradores();
            #endif
        }
        
        vm.VM_ProcessarTimers();

        desenhar_display(renderer, &vm);
 
        SDL_Delay(timer_delay_ms);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}