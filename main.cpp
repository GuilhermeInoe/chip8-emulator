#include "c8vm.h"
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    if (argc < 2) {
        fprintf(stderr, "EXecute o comando seguido do arquivo ROM Ex.: %s <arquivo_rom>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    VM vm;
    vm.VM_Inicializar(&vm, 0x200);
    
    vm.VM_CarregarROM(&vm, argv[1], 0x200);

    #ifdef DEBUG
    vm.VM_ImprimirRegistradores(&vm);
    #endif

    while(1){
        vm.VM_ExecutarInstrucao(&vm);
        #ifdef DEBUG
        vm.VM_ImprimirRegistradores(&vm);
        #endif
    }
}
