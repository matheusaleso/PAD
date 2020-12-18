#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include <time.h>
#include <sys/time.h>
#define N 4
#define MASTER 0
#define TAG 1
/* Funcao que multiplica duas matrizes */
void multiplicar_matrizes(float*, float*, float*, unsigned int, unsigned int, unsigned int);

/* Funcao que soma os elementos de uma matriz */
double reducao_pela_soma(float*, unsigned int, unsigned int);

/* Funcao que le um arquivo e armazena seus valores em uma matriz */
void gerar_matriz (char*, float*, unsigned int, unsigned int);

/* Funcao que grava a matriz em um arquivo */
void gravar_matriz(char*, float*, unsigned int, unsigned int);

/* Funcao que retorna a quantidade de linhas da matriz A que serão processada pela respectiva worker  */
int get_numLinhas(int, int, int);

/* Funcao que envia as matrizes ("mensagens") para as workers  */
void enviar_para_workers(float*, float*, float*, int, int, int, int);

/* Funcao que recebe as matrizes ("mensagens") das workers  */
void receber_das_workers(float*, int, int, int, MPI_Status, double *);

int main(int argc, char **argv){
    if(argc==8){
        /* Variбveis relacionadas ao MPI */
        MPI_Status status;
        int numTasks, rank, numWorkers, tag;

        /* Variбveis relacionadas as matrizes */
        int y = atoi(argv[1]), w = atoi(argv[2]), v = atoi(argv[3]);
        float *matrizA, *matrizB, *matrizC, *matrizD, *matrizAB;

        /* Variбveis relacionadas aos nomes dos arquivos */
        char *nomeArqA = argv[4], *nomeArqB = argv[5], *nomeArqC = argv[6], *nomeArqD = argv[7];

        /* Variбveis relacionadas ao tempo de execuзгo */
        struct timeval inicio, fim;
        long segundos, microssegundos;
        double tempo, soma;

        /* Inicializa as matrizes */
        matrizA   = (float *) malloc(y * w * sizeof(float));
        matrizB   = (float *) malloc(w * v * sizeof(float));
        matrizC   = (float *) malloc(v * 1 * sizeof(float));
        matrizD   = (float *) malloc(y * 1 * sizeof(float));

        /* Preenche as matrizes com os valores dos respectivos arquivos */
        gerar_matriz(nomeArqA,matrizA,y,w);
        gerar_matriz(nomeArqB,matrizB,w,v);
        gerar_matriz(nomeArqC,matrizC,v,1);

        /* Inicializa o MPI */
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
        numWorkers = numTasks-1;

        if(numTasks == N) {
            /* ------ Processos na Master ------ */
            if(rank == 0){
               gettimeofday(&inicio, 0);
               enviar_para_workers(matrizA, matrizB, matrizC, y, w, v, numWorkers);
               receber_das_workers(matrizD, y, 1, numWorkers, status, &soma);

               gettimeofday(&fim, 0);

               segundos = fim.tv_sec - inicio.tv_sec;
               microssegundos = fim.tv_usec - inicio.tv_usec;
               tempo = segundos + microssegundos*1e-6;
               //printf("As operacoes com matrizes levaram %f segundos para executar\n", tempo);

              /* Grava a matrizD em um arquivo */
                gravar_matriz(nomeArqD,matrizD,y,1);a

                printf("%lf\n", soma);
            }

            /* ------ Processos nas Workers ------  */
            if(rank > 0){
                int numLinhasA;
                MPI_Recv(&numLinhasA, 1, MPI_INT, MASTER, TAG, MPI_COMM_WORLD, &status);

                float *matrizA = malloc(numLinhasA*w*sizeof(float*));
                float *matrizB = malloc(w*v*sizeof(float*));
                float *matrizC = malloc(v*1*sizeof(float*));
                float *matrizAB = malloc(numLinhasA*v*sizeof(int *));
                float *matrizD = malloc(numLinhasA*1*sizeof(int *));

                MPI_Recv(matrizA, numLinhasA*w, MPI_FLOAT, MASTER, TAG, MPI_COMM_WORLD, &status);
                MPI_Recv(matrizB, w*v, MPI_FLOAT, MASTER, TAG, MPI_COMM_WORLD, &status);
                MPI_Recv(matrizC, v*1, MPI_FLOAT, MASTER, TAG, MPI_COMM_WORLD, &status);

                /* --- Multiplicaзгo das Matrizes --- */
                multiplicar_matrizes(matrizA, matrizB, matrizAB, numLinhasA, w, v);
                multiplicar_matrizes(matrizAB, matrizC, matrizD, numLinhasA, v, 1);

                /* --- Reduзгo pela Soma --- */
                double somaParcial = reducao_pela_soma(matrizD, numLinhasA, 1);

                MPI_Send(matrizD, numLinhasA*1, MPI_FLOAT, MASTER, TAG, MPI_COMM_WORLD);
                MPI_Send(&somaParcial, 1, MPI_DOUBLE, MASTER, TAG, MPI_COMM_WORLD);

                free(matrizA);
                free(matrizB);
                free(matrizAB);
                free(matrizC);
                free(matrizD);
            }
        }
        else {
             printf("O programa requer %d processadores.\n", N);
        }
        MPI_Finalize();
    }
    else{
        printf("Quantidade de parametros invalida!\n");
    }

    return 0;
}


int get_numLinhas(int idWorker, int resto, int mediaLinhas){
  if(idWorker <= resto )
    return mediaLinhas+1;
  else
    return mediaLinhas;
}

void enviar_para_workers(float *matrizA, float *matrizB, float *matrizC, int y, int w, int v, int numWorkers){
    /* Transforma a matriz B em um array */
    float *bufferB = malloc(w*v*sizeof(float *));
    for(int i=0; i<w; i++){
        for(int j=0; j<v; j++){
            bufferB[i*v+j] = matrizB[i*v+j];
        }
    }

    /* Transforma a matriz C em um array */
    float *bufferC = malloc(v*1*sizeof(float *));
    for(int i=0; i < v; i++){
      for(int j=0; j < 1; j++){
        bufferC[i*1+j] = matrizC[i*1+j];
      }
    }

    int mediaLinhas = y/numWorkers;
    int resto = y%numWorkers;
    int offset = 0;

    /* Divide a matriz A entre as workers */
    for(int idWorker = 1; idWorker <= numWorkers; idWorker++){
        int numLinhas = get_numLinhas(idWorker, resto, mediaLinhas);

        /* Transforma a matriz A em um array */
        float *bufferA = malloc(numLinhas*w*sizeof(float *));
        int index = 0;
        for(int linha=offset; linha < offset + numLinhas; linha++){
          for(int coluna=0; coluna < w; coluna++){
            bufferA[index++] = matrizA[linha*w+coluna];
          }
        }
        offset += numLinhas;

        MPI_Send(&numLinhas, 1, MPI_INT, idWorker, TAG, MPI_COMM_WORLD);
        MPI_Send(bufferA, numLinhas*w, MPI_FLOAT, idWorker, TAG, MPI_COMM_WORLD);
        MPI_Send(bufferB, w*v, MPI_FLOAT, idWorker, TAG, MPI_COMM_WORLD);
        MPI_Send(bufferC, v*1, MPI_FLOAT, idWorker, TAG, MPI_COMM_WORLD);

        free(bufferA);
    }

    free(bufferB);
    free(bufferC);
}

void receber_das_workers(float *matrizResultado, int y, int v, int numWorkers, MPI_Status status, double *soma){
  int mediaLinhas = y/numWorkers;
  int resto = y%numWorkers;
  int offset = 0;
  double somaParcial = 0;

  for(int idWorker=1; idWorker<=numWorkers; idWorker++){
        int numLinhas = get_numLinhas(idWorker, resto, mediaLinhas);

        float *bufferResultado = malloc(numLinhas*v*sizeof(float *));

        MPI_Recv(bufferResultado, numLinhas*v, MPI_FLOAT, idWorker, TAG, MPI_COMM_WORLD, &status);
        MPI_Recv(&somaParcial,1, MPI_DOUBLE, idWorker, TAG, MPI_COMM_WORLD, &status);
        *soma +=somaParcial;

        int index = 0;
        for(int linha=offset; linha < offset + numLinhas; linha++){
          for(int coluna=0; coluna < v; coluna++){
            matrizResultado[linha*v+coluna] = bufferResultado[index++];
          }
        }
        offset += numLinhas;

        free(bufferResultado);
  }
}

void multiplicar_matrizes(float *mat1, float *mat2, float *matResult, unsigned int y, unsigned int w, unsigned int v){
    register unsigned int linha, coluna, i;
    register float soma=0;

    for(linha=0; linha<y; linha++){
        for(coluna=0; coluna < v; coluna++){
            soma=0;
            for(i=0; i < w; i++){
                soma += mat1[linha*w + i] * mat2[i*v + coluna];
            }
            matResult[linha*v + coluna] = soma;
        }
    }
}

double reducao_pela_soma(float *matriz, unsigned int linM, unsigned int colM){
    register int linha, coluna;
    register double soma = 0;

    for(linha=0; linha < linM; linha++){
        for(coluna=0; coluna < colM; coluna++){
            soma += matriz[linha*colM + coluna];
        }
    }

    return soma;
}

void gerar_matriz (char *nomedoArquivo, float *matriz, unsigned int linM, unsigned int colM){
    FILE  *arquivo = fopen(nomedoArquivo,"r");

    if(arquivo == NULL){
        printf("O arquivo %s nao existe!\n", nomedoArquivo);
        fclose(arquivo);
        exit(0);
    }

    register unsigned int linha, coluna;

    for (linha=0; linha<linM;linha++){
        for (coluna=0; coluna<colM;coluna++){
            fscanf(arquivo,"%f",&matriz[linha*colM +coluna]);
        }
    }

    fclose(arquivo);
}

void gravar_matriz(char *nomeArquivo, float *matriz, unsigned int linM, unsigned int colM){
    FILE *arquivo = fopen(nomeArquivo, "w");

    if(arquivo == NULL){
        printf("Erro ao criar arquivo: %s\n", nomeArquivo);
        fclose(arquivo);
        exit(0);
    }

    register unsigned int linha, coluna;

    for (linha=0; linha<linM; linha++){
        for (coluna=0; coluna<colM;coluna++){
            fprintf(arquivo,"%.2f\n", matriz[linha*colM +coluna]);
        }
    }

     fclose(arquivo);
}
