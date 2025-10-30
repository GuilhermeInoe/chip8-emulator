# Como Usar o Emulador Chip-8

Este documento detalha como executar o emulador e quais opções de linha de comando estão disponíveis.

---

## Sintaxe Básica

A sintaxe básica para executar o programa é:

```sh
./chip8 [opções] caminho/para/a/rom.ch8
```

> **Observação:**  
> O argumento `caminho/para/a/rom.ch8` é **obrigatório** e deve ser o **último argumento** que não é uma flag.

---

## Opções de Linha de Comando

### `--hz <numero>`
Define a **velocidade (frequência)** da CPU em Hz. Este valor determina quantos ciclos da VM são executados por segundo.  

**Padrão:** `700`

```sh
Exemplo: ./chip8 --hz 1000 roms/tetris.ch8
```

---

### `--escala <numero>`
Define o **fator de escala da janela**.  
A tela original do Chip-8 é 64x32 pixels. Um fator de escala 10 resulta em uma janela de 640x320 pixels.  

**Padrão:** `10`

```sh
Exemplo: ./chip8 --escala 15 roms/pong.ch8
```

---

### `--pc <endereco>`
Define o **endereço de memória inicial (Program Counter)** onde a ROM será carregada.  
O padrão `0x200` é o endereço histórico para a maioria das ROMs de Chip-8.  
O endereço pode ser em decimal ou hexadecimal (ex: `512` ou `0x200`).  

**Padrão:** `0x200`

```sh
Exemplo: ./chip8 --pc 0x200 roms/outra.ch8
```

---

### Não utilizar nenhum argumento
Exibe a **mensagem de ajuda** no terminal e encerra o programa.

```sh
Exemplo: ./chip8 --ajuda
```
