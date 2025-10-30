# Instruções de Compilação do Emulador Chip-8

Este documento explica como compilar e executar o emulador Chip-8 em um sistema Linux (Ubuntu/Debian), macOS ou Arch Linux.

## 1. Dependências

Para compilar o projeto, você precisará dos seguintes componentes:

1.  **Compilador C++:** `g++` (GNU).
2.  **Biblioteca SDL2:** A "Simple DirectMedia Layer" (versão 2) é usada para criar a janela, renderizar os gráficos, processar o teclado e reproduzir o áudio.
    * **Nome do pacote (Linux/Debian):** `libsdl2-dev`
3.  **Ferramenta Make:** `make` (para processar o Makefile).

## 2. Instalação das Dependências para Linux (baseado em Debian/Ubuntu)

```sh
sudo apt-get update
sudo apt-get install g++ make libsdl2-dev
```

### 3. Como compilar o projeto
1. Abra seu terminal na pasta raiz do projeto (onde o arquivo Makefile está localizado).
2. Execute o comando: 
```sh 
make
```
3. O executável gerado se chamará chip8
