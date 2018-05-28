#include<stdio.h>
#include<stdlib.h>
#include<time.h>  
#include<mpi.h>

const float Cx = 0.1;
const float Cy = 0.1;

struct Rango{
	int inicioFila;
	int inicioCol;
	int finFila;
	int finCol;
	int esCuadrado; //1 si es cuadrado, 0 si no es cuadrado

}

double sampleTime()
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return ((double)tv.tv_sec+((double)tv.tv_nsec)/1000000000.0);
}








int main(int argc, char *argv[])
{
    
    if(argc == 3){

        int Tlado =  atoi(argv[1]); // Tamaño de la matriz cuadrada
        int pasos = atoi(argv[2]); // Cantidad de pasos

        int q;
        float tiempopromedio = 0;

        if(Tlado > 0 && pasos > 0){

            char processor_name[MPI_MAX_PROCESSOR_NAME]
            //for(q = 0; q < 3; q++)
            //{
                float **matrizOriginal; //Puntero a la matriz original
                float **matrizCopia; //Puntero a la matriz auxiliar utilizada para copiar los nuevos valores
                float **aux; //Puntero auxiliar 
                register int i,j,p; //Variables auxiliares para la iteracion de bucles
                //float Cx = 0.1, Cy = 0.1; //Constantes para la transferencia de calor
              
                //Reservo espacio para la matriz original
                matrizOriginal  = (float **)malloc(sizeof(float *) * Tlado);
                *matrizOriginal = (float *)malloc(sizeof(float) * Tlado * Tlado);
             
                //Reservo espacio para la matriz copia
                matrizCopia  = (float **)malloc(sizeof(float *) * Tlado);
                *matrizCopia = (float *)malloc(sizeof(float) * Tlado * Tlado);

                //Creo los punteros para cada valor de fila
                for(i = 0; i < Tlado; i++)
                {
                    matrizOriginal[i] = (*matrizOriginal + Tlado * i);
                    matrizCopia[i] = (*matrizCopia + Tlado * i);
                }

                //Inicializo matrizOriginal
                for (i = 0; i < Tlado; i++)
                {
                    for (j = 0; j < Tlado; j++){
                        matrizOriginal[i][j] = i * (Tlado - i -1) * j * (Tlado - j - 1);  
                    }
                }

            


                //Aplico la cantidad de pasos solicitada 
                int extremo = Tlado-1; 
	    
                double tiempo_trans;
                const char* s = getenv("OMP_NUM_THREADS");

                int cantHilos,N;
                cantHilos = atoi(s);
                //printf("Cantidad hilos: %d\n",cantHilos);
                N = (extremo-1)/(cantHilos*2);
                printf("N = %d\n",N);
                tiempo_trans= sampleTime();
		

        		MPI_Init(&argc,&argv);	
        		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        		MPI_Comm_size(MPI_COMM_WORLD, &cantProcesos);
        		MPI_Get_processor_name(processor_name, &namelen); // QUE ES? Es el nombre del procesador (nodo)

                //dividirMatriz();

                for ( p = 0; p < pasos; p++)
                {
                	for (i = 1; i < extremo; i++)
                    	{
                        	for (j = 1; j < extremo; j++)
                       		{	
                            		matrizCopia[i][j] = matrizOriginal[i][j] + Cx * (matrizOriginal[i+1][j] + matrizOriginal[i-1][j] - 2 * matrizOriginal[i][j]) + Cy * (matrizOriginal[i][j+1] + matrizOriginal[i][j-1] - 2 * matrizOriginal[i][j]);
                       		}
                    	}

                    //Realizo el cambio de punteros para que matrizOriginal (P) apunte a la nueva matriz (P+1)
                    aux = matrizOriginal;
                    matrizOriginal = matrizCopia;
                    matrizCopia = aux;  
                }
	
                //tiempo_trans = sampleTime() - tiempo_trans;
                //printf("Tiempo de ejecucion: %.21f segundos.\n",tiempo_trans);
                //tiempopromedio = tiempopromedio + tiempo_trans;

                free(matrizCopia);
                free(matrizOriginal);
            //}

            //tiempopromedio = tiempopromedio/3;
            //printf("Tiempo promedio: %.21f\n",tiempopromedio);
            
            MPI_Finalize();
            return 0;

        }else{
            printf("Cantidad o Tipo de argumentos no valida/o \n");
        }
    }else{
        printf("Cantidad o Tipo de argumentos no valida/o \n");
    }
}
