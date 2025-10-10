# Emulador Chip-8 em C++

![Linguagem](https://img.shields.io/badge/linguagem-C%2B%2B-blue.svg)
![Build](https://img.shields.io/badge/build-CMake%20%7C%20Make-brightgreen.svg)
![Plataforma](https://img.shields.io/badge/plataforma-Linux%20%7C%20Windows-orange.svg)
![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-lightgrey.svg)

Este projeto é uma implementação da máquina virtual **Chip-8**, desenvolvida em C++ com a biblioteca SDL2, como parte do trabalho da disciplina de Aspectos De Linguagens De Programação  ministrada pelo **Prof. Dr. Juliano H. Foleis**.

## 📖 Tabela de Conteúdos

- [O que é Chip-8?](#-o-que-é-chip-8)
- [✨ Funcionalidades](#-funcionalidades)
- [📸 Screenshots](#-screenshots)
- [🚀 Como Executar](#-como-executar)
- [⌨️ Mapeamento do Teclado](#️-mapeamento-do-teclado)
- [🎯 Status do Projeto](#-status-do-projeto)

## ❔ O que é Chip-8?

O Chip-8 é uma linguagem de programação interpretada, desenvolvida nos anos 70 por Joseph Weisbecker. Por sua simplicidade e arquitetura bem definida, a implementação de um interpretador Chip-8 tornou-se um rito de passagem popular para entusiastas interessados em emulação e o funcionamento de máquinas virtuais.

O objetivo deste trabalho é criar um interpretador funcional, capaz de carregar e executar programas e jogos clássicos desenvolvidos para a plataforma Chip-8, compreendendo na prática o ciclo de *fetch-decode-execute*, gerenciamento de memória, registradores e periféricos virtuais.

## ✨ Funcionalidades

- **CPU Chip-8 Completa**: Implementação de todas as 36 instruções (opcodes) da especificação original.
- **Gerenciamento de Memória**: 4KB de RAM, com espaço para ROMs a partir do endereço `0x200`.
- **Registradores**: 16 registradores de 8 bits (V0-VF), registrador de endereço (I), Program Counter (PC) e Stack Pointer (SP).
- **Renderização Gráfica**: Tela monocromática de 64x32 pixels renderizada com a biblioteca **SDL2**.
- **Entrada de Teclado**: Suporte ao teclado hexadecimal de 16 teclas, mapeado para o teclado físico.
- **Temporizadores**: Implementação do *Delay Timer* e *Sound Timer*, decrementando a 60Hz.
- **Áudio**: Reprodução de um "beep" quando o *Sound Timer* está ativo.
- **Configuração via Linha de Comando**:
  - Velocidade da CPU (clock em Hz).
  - Fator de escala da janela (zoom).
  - Endereço de carregamento da ROM.
- **Multiplataforma**: Código portável para compilar e rodar em Linux e Windows.

## 📸 Screenshots

*Adicionar futuramente*

| algo | algo |
| :---: | :---: |
| ![Executando algo](https://i.imgur.com/3u32Cf7.png) | ![Executando algo](https://i.imgur.com/uXEAS3j.png) |
| *Exemplo de Screenshot de tal coisa.* | *Exemplo de Screenshot de tal coisa.* |

## 🛠️ Tecnologias e Dependências

Para compilar e executar este projeto, você precisará das seguintes ferramentas:

- **C++ Compiler**: `g++` ou `clang++` com suporte a C++17 ou superior.
- **Build System**: `CMake` (versão 3.10 ou superior) ou `Make`.
- **Biblioteca Gráfica**: `SDL2` (Simple DirectMedia Layer).


## 🚀 Como Executar
Primeiro, compile o projeto
```sh
make all
```
Após, execute:
```sh
./chip8 1-chip8-logo.ch8
```



## ⌨️ Mapeamento do Teclado

O teclado hexadecimal do Chip-8 foi mapeado para as seguintes teclas do teclado físico:

| Tecla Chip-8 | Tecla Física |   | Tecla Chip-8 | Tecla Física |
| :----------: | :----------: | - | :----------: | :----------: |
|      `1`     |      `1`     |   |      `2`     |      `2`     |
|      `3`     |      `3`     |   |      `C`     |      `4`     |
|      `4`     |      `Q`     |   |      `5`     |      `W`     |
|      `6`     |      `E`     |   |      `D`     |      `R`     |
|      `7`     |      `A`     |   |      `8`     |      `S`     |
|      `9`     |      `D`     |   |      `E`     |      `F`     |
|      `A`     |      `Z`     |   |      `0`     |      `X`     |
|      `B`     |      `C`     |   |      `F`     |      `V`     |

## 🎯 Status do Projeto

🚧 Em Desenvolvimento 🚧

---
*Projeto desenvolvido por [Guilherme Inoe] e [Winicius Abilio] para a disciplina de [Aspectos De Linguagens De Programação] - [2025/2].*
