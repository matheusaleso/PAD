#include <stdio.h>
#include <stdlib.h>
#include <omp.h> // Biblioteca OpenMP
#include <time.h>
#include <sys/time.h>

/* Funcao que multiplica duas matrizes */
float* multiplicar_matrizes(float*, float*, unsigned int, unsigned int, unsigned int, unsigned int);

/* Funcao que soma os elementos de uma matriz */
double reducao_pela_soma(float*, unsigned int, unsigned int);

/* Funcao que gera um arquivo com numeros aleatorios entre -10 e 10 */
void gerar_valores_aleatorios (char*, unsigned int, unsigned int);

/* Funcao que le um arquivo e armazena seus valores em uma matriz */
float *gerar_matriz (char*, float*, unsigned int, unsigned int);

/* Funcao que grava a matriz em um arquivo */
void gravar_matriz(char*, float*, unsigned int, unsigned int);


int main(int argc, char **argv){
    if(argc==8){

        int y = atoi(argv[1]), w = atoi(argv[2]), v = atoi(argv[3]);
        char *nomeArqA = argv[4], *nomeArqB = argv[5], *nomeArqC = argv[6], *nomeArqD = argv[7];
        struct timeval inicio, fim;
        long segundos, microssegundos;
        double tempo, soma;

        float *matrizA, *matrizB, *matrizC, *matrizD, *matrizAB;

        /* Alocacao dinamica das matrizes */
        matrizA   = (float *) malloc(y * w * sizeof(float));
        matrizB   = (float *) malloc(w * v * sizeof(float));
        matrizC   = (float *) malloc(v * 1 * sizeof(float));
        matrizD   = (float *) malloc(y * 1 * sizeof(float));
        matrizAB  = (float *) malloc(y * v * sizeof(float)); /* Matriz resultado da multiplicacao da matrizA com a matrizB */

        /* Preenche os arquivos com valores aleatorios */
        srand(time(NULL)); // garante que os valores gerados sejam diferentes a cada execução sejam diferentes
        gerar_valores_aleatorios(nomeArqA,y,w);
        gerar_valores_aleatorios(nomeArqB,w,v);
        gerar_valores_aleatorios(nomeArqC,v,1);

        /* Preenche as matrizes com os valores dos respectivos arquivos */
        matrizA = gerar_matriz(nomeArqA,matrizA,y,w);
        matrizB = gerar_matriz(nomeArqB,matrizB,w,v);
        matrizC = gerar_matriz(nomeArqC,matrizC,v,1);

        gettimeofday(&inicio, 0);

        /* Multiplica as matrizes */
        matrizAB = multiplicar_matrizes(matrizA, matrizB, y, w, w, v);
        matrizD = multiplicar_matrizes(matrizAB, matrizC, y, v, y, 1);
        free(matrizAB);

        soma = reducao_pela_soma(matrizD,y,1);
        gettimeofday(&fim, 0);

        segundos = fim.tv_sec - inicio.tv_sec;
        microssegundos = fim.tv_usec - inicio.tv_usec;
        tempo = segundos + microssegundos*1e-6;
        /* printf("As operacoes com matrizes levaram %f segundos para executar\n", tempo); */

        /* Grava a matrizD em um arquivo */
        gravar_matriz(nomeArqD,matrizD,y,1);
        printf("%lf\n", soma);
    }

    else{
        printf("Quantidade de parametros invalida!\n");
    }

    return 0;
}


float* multiplicar_matrizes(float *matA, float *matB, unsigned int linA, unsigned int colA, unsigned int linB, unsigned int colB){
    float *matAux = (float *) malloc(linA * colB * sizeof(float));

    register unsigned int linha, coluna, i;
    register float soma=0;

   #pragma omp parallel num_threads(4) private(linha, coluna, i, soma) shared(matA, matB, linA, colB, linB)
   {
        #pragma omp for
        for(linha=0; linha<linA; linha++){
            for(coluna=0; coluna < colB; coluna++){
                soma=0;
                for(i=0; i < linB; i++){
                    soma += matA[linha*colA + i] * matB[i*colB + coluna];
                }
                matAux[linha*colB + coluna] = soma;
            }
        }
   }

    return matAux;
}

double reducao_pela_soma(float *matriz, unsigned int linM, unsigned int colM){
    register int linha, coluna;
    register double soma = 0;

    for(linha=0; linha < linM; linha++){
        #pragma omp parallel for num_threads(4) reduction(+:soma)
        for(coluna=0; coluna < colM; coluna++){
            soma += matriz[linha*colM + coluna];
        }
    }

    return soma;
}

void gerar_valores_aleatorios (char *nomeArquivo, unsigned int linM, unsigned int colM){
     register unsigned int linha, coluna;
     register float rand_float, rand_limite;

     FILE * arquivo;
     arquivo = fopen(nomeArquivo, "w");

     for (linha=0; linha<linM;linha++){
        for (coluna=0; coluna<colM;coluna++){
            rand_float = (float)rand() / ((float) RAND_MAX + 1);  // gera um número real entre [0, 1)
            rand_limite = (-10) + rand_float * (10 - (-10 + 1));  // dimensiona o número para o intervalo: [-10, 10]

            fprintf(arquivo, "%.2f\n", rand_limite);
        }
     }

     fclose(arquivo);
}

float *gerar_matriz (char *nomedoArquivo, float *matriz, unsigned int linM, unsigned int colM){
    register unsigned int linha, coluna;
    FILE  *arquivo;
    arquivo= fopen(nomedoArquivo,"r");

    for (linha=0; linha<linM;linha++){
        for (coluna=0; coluna<colM;coluna++){
            fscanf(arquivo,"%f",&matriz[linha*colM +coluna]);
        }
    }

    fclose(arquivo);
    return matriz;
}

void gravar_matriz(char *nomeArquivo, float *matriz, unsigned int linM, unsigned int colM){
    FILE *arquivo;
    arquivo = fopen(nomeArquivo, "w");

    register unsigned int linha, coluna;

    for (linha=0; linha<linM; linha++){
        for (coluna=0; coluna<colM;coluna++){
            fprintf(arquivo,"%.2f\n", matriz[linha*colM +coluna]);
        }
    }

     fclose(arquivo);
}
