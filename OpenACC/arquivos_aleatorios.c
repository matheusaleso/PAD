#include <stdio.h>
#include <stdlib.h>

/* Funcao que gera um arquivo com numeros aleatorios entre -10 e 10 */
void gerar_valores_aleatorios (char*, unsigned int, unsigned int);

int main(int argc, char **argv){
    if(argc == 7){

        int y = atoi(argv[1]), w = atoi(argv[2]), v = atoi(argv[3]);
        char *nomeArqA = argv[4], *nomeArqB = argv[5], *nomeArqC = argv[6];

        float *matrizA, *matrizB, *matrizC, *matrizD, *matrizAB;

        /* Preenche os arquivos com valores aleatorios */
        gerar_valores_aleatorios(nomeArqA,y,w);
        gerar_valores_aleatorios(nomeArqB,w,v);
        gerar_valores_aleatorios(nomeArqC,v,1);
    }

    else{
        printf("Quantidade de parametros invalida!\n");
    }

    return 0;
}

void gerar_valores_aleatorios (char *nomeArquivo, unsigned int linM, unsigned int colM){
     FILE * arquivo = fopen(nomeArquivo, "w");
     if(arquivo != NULL){
        register unsigned int linha, coluna;
        register float rand_float, rand_limite;

        for (linha=0; linha<linM;linha++){
            for (coluna=0; coluna<colM;coluna++){
                rand_float = (float)rand() / ((float) RAND_MAX + 1);  // gera um nъmero real entre [0, 1)
                rand_limite = (-10) + rand_float * (10 - (-10 + 1));  // dimensiona o nъmero para o intervalo: [-10, 10]

                fprintf(arquivo, "%.2f\n", rand_limite);
            }
        }
     }
     else {
        printf("Erro ao criar arquivo: %s\n", nomeArquivo);
     }
     fclose(arquivo);
}
