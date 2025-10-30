#include "c8vm.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

// Valores padrão
const int DEFAULT_CHIP8_HZ = 700;
const int DEFAULT_SCALE = 10;
const uint16_t DEFAULT_PC_START = 0x200;
const int TIMER_FREQUENCY = 60; // 60Hz fixo

// Mapeamento de teclas SDL para CHIP-8 (0-F)
uint8_t key_map[SDL_NUM_SCANCODES] = {0};

void inicializar_key_map() {
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


void audio_callback(void* userdata, Uint8* stream, int len) {
    VM* vm = (VM*)userdata;
    Sint16* buffer = (Sint16*)stream;
    int samples = len / (sizeof(Sint16)); // 2 bytes por amostra (Sint16)

    // Período da onda (quantas amostras por ciclo de onda)
    static const int TONE_PERIOD = 44100 / 440; 
    static const Sint16 TONE_VOLUME = 3000;
    static int wave_pos = 0; // Posição atual na onda

    for (int i = 0; i < samples; i++) {
        if (vm->sound_timer > 0) {
            // Gera uma onda quadrada simples
            buffer[i] = ((wave_pos++ / (TONE_PERIOD / 2)) % 2) ? TONE_VOLUME : -TONE_VOLUME;
        } else {
            buffer[i] = 0; // Silêncio
        }
    }
}


void desenhar_display(SDL_Renderer* renderer, VM* vm, int pixel_size) {
    //Define a cor de fundo (preto) e limpa a tela
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Define a cor do pixel (branco)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Itera por cada pixel do display 64x32
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            
            // Se o pixel (na VM) estiver aceso
            if (vm->DISPLAY[y * 64 + x]) {
                
                // Define um retângulo escalado na posição correta
                SDL_Rect pixel;
                pixel.x = x * pixel_size;
                pixel.y = y * pixel_size;
                pixel.w = pixel_size;
                pixel.h = pixel_size;
                
                // Desenha o retângulo (pixel)
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
    
    // Apresenta o quadro renderizado na janela
    SDL_RenderPresent(renderer);
}


void imprimir_ajuda(char* prog_name) {
    fprintf(stderr, "Uso: %s <caminho_rom> [OPÇÕES]\n\n", prog_name);
    fprintf(stderr, "Opções:\n");
    fprintf(stderr, "  --hz <numero>      Define a velocidade da CPU em Hz (padrão: %d)\n", DEFAULT_CHIP8_HZ);
    fprintf(stderr, "  --escala <numero>  Define o fator de escala da janela (padrão: %d)\n", DEFAULT_SCALE);
    fprintf(stderr, "  --pc <endereco>    Define o endereço de carga (padrão: 0x%X)\n", DEFAULT_PC_START);
    fprintf(stderr, "  --ajuda            Mostra esta mensagem de ajuda\n");
}

int main(int argc, char** argv){
    if (argc < 2) {
        imprimir_ajuda(argv[0]);
        exit(EXIT_FAILURE);
    }

    int chip8_frequency = DEFAULT_CHIP8_HZ;
    int pixel_size = DEFAULT_SCALE;
    uint16_t pc_start = DEFAULT_PC_START;
    char* rom_path = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--ajuda") == 0) {
            imprimir_ajuda(argv[0]);
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(argv[i], "--hz") == 0 && i + 1 < argc) {
            chip8_frequency = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--escala") == 0 && i + 1 < argc) {
            pixel_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--pc") == 0 && i + 1 < argc) { 
            pc_start = (uint16_t)strtol(argv[++i], NULL, 0);
        }
        else if (rom_path == NULL) {
            // O primeiro argumento que não é flag é a ROM
            rom_path = argv[i];
        }
        else {
            fprintf(stderr, "Argumento desconhecido: %s\n", argv[i]);
            imprimir_ajuda(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (rom_path == NULL) {
        fprintf(stderr, "Erro: Caminho da ROM não fornecido.\n");
        imprimir_ajuda(argv[0]);
        exit(EXIT_FAILURE);
    }
    
    if (chip8_frequency <= 0 || pixel_size <= 0) {
        fprintf(stderr, "Erro: Frequência e escala devem ser > 0\n");
        exit(EXIT_FAILURE);
    }

    // Calcula tamanho da janela dinamicamente
    int screen_width = 64 * pixel_size;
    int screen_height = 32 * pixel_size;

    VM vm;
    vm.VM_Inicializar(pc_start);
    vm.VM_CarregarROM(rom_path, pc_start);
    inicializar_key_map();

    // Inicializa Vídeo e Áudio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Erro ao inicializar SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Configuração do Áudio
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want)); // Limpa a struct
    want.freq = 44100;                 // Frequência (samples por seg)
    want.format = AUDIO_S16SYS;        // Formato (Signed 16-bit)
    want.channels = 1;                 // Mono
    want.samples = 2048;               // Tamanho do buffer
    want.callback = audio_callback;    // Função a ser chamada
    want.userdata = &vm;               // Passa a VM para o callback

    if (SDL_OpenAudio(&want, &have) < 0) {
        fprintf(stderr, "Erro ao abrir áudio SDL: %s\n", SDL_GetError());
        // Continua o código se o áudio falhar
    } else {
        SDL_PauseAudio(0); // Inicia a reprodução
    }

    SDL_Window* window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screen_width, screen_height,
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

    // Ciclos de CPU por quadro de 60Hz
    int cycles_per_frame = chip8_frequency / TIMER_FREQUENCY;

    // Tempo por quadro (em ms) para atingir 60Hz
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
                // Ignora '0' que é o valor padrão
                if (chip8_key != 0 || e.key.keysym.scancode == SDL_SCANCODE_X) { 
                    vm.VM_ProcessarInput(chip8_key, true);
                }
            } else if (e.type == SDL_KEYUP) {
                uint8_t chip8_key = key_map[e.key.keysym.scancode];
                if (chip8_key != 0 || e.key.keysym.scancode == SDL_SCANCODE_X) {
                    vm.VM_ProcessarInput(chip8_key, false);
                }
            }
        }

        //Executa 'N' ciclos da CPU
        for (int i = 0; i < cycles_per_frame; ++i) {
            vm.VM_ExecutarInstrucao();
            #ifdef DEBUG
            vm.VM_ImprimirRegistradores();
            #endif
        }
        
        //Processa timers a 60Hz
        vm.VM_ProcessarTimers();

        //Desenha a 60Hz
        desenhar_display(renderer, &vm, pixel_size);
 
        // Trava o loop em 60Hz
        SDL_Delay(timer_delay_ms);
    }
    
    // Limpeza
    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}