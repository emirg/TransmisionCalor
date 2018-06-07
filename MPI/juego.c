#include<stdio.h>
#include<stdlib.h>
#include<time.h>  
#include<unistd.h>
#include<mpi.h>
#include<string.h>

typedef enum { false, true } bool;

const float Cx = 0.1;
const float Cy = 0.1;

// Prototipos: permiten poner las funciones debajo de main
double sampleTime(void);
void divisionOptima(int, int*, int*);

int main(int argc, char *argv[])
{
	MPI_Init(&argc,&argv);

	int rank, cantProcesos, namelen;
	char nombreNodo[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &cantProcesos);
	MPI_Get_processor_name(nombreNodo, &namelen);

	if (argc != 3)
	{
		if (rank == 0)
		{
			printf("ERROR: Cantidad de argumentos invalidos\n");
		}
		MPI_Finalize();
		exit(1);
	}


	int Tlado = atoi(argv[1]);  // Tamaño de la matriz cuadrada - N
	int pasos = atoi(argv[2]);  // Cantidad de pasos
	
	if (Tlado <= 0 || pasos < 0)
	{
		if (rank == 0)
		{
			printf("ERROR: Datos de entrada invalidos\n");
		}
		MPI_Finalize();
		exit(2);
	}

	if (cantProcesos < 2)
	{
		if (rank == 0)
		{
			printf("ERROR: Ejecute el algoritmo con al menos 2 procesos\n");
		}
		MPI_Finalize();
		exit(3);
	}

	int i, j;               // Posición de cada proceso en submatrices de una matriz imaginaria
	int alto, ancho;        // Ancho y alto de cada submatriz
	int m, n, p;            // Iteracion de bucles 
	int cantFilas, cantCol; // División más óptima de una matriz imaginaria en base a la cantidad total de procesos
	int cantFilasExtra, cantColExtra;

	divisionOptima(cantProcesos, &cantFilas, &cantCol);

	// Posiciones de cada submatriz de acuerdo al rank que le toque a cada proceso
	i = rank / cantCol;
	j = rank % cantCol;

	char nombre[30];
	sprintf(nombre, "subgrid_%d_%d.out", i, j);

	FILE *f = fopen(nombre, "w");
	if (f == NULL)
	{
		printf("ERROR: No se pudo abrir el archivo");
		exit(4);
	}

	alto  = Tlado / cantFilas; // Dividimos el alto en la cantidad de filas mas optima
	ancho = Tlado / cantCol;   // Dividimos el ancho en la cantidad de columnas mas optima
	cantFilasExtra = Tlado % cantFilas;  // Obtenemos las filas adicionales totales que tomarán los procesos 
	cantColExtra   = Tlado % cantCol;    // Obtenemos las columnas adicionales totales que tomarán los procesos

	if (j < cantColExtra)
	{
		ancho++;
	}

	if (i < cantFilasExtra)
	{
		alto++;
	}

	//Una vez que se tienen los datos, cada nodo deberia reservar memoria acorde a lo que recibio y calcular los iniciales
	//Antes de empezar a calcular, deberia solicitar los datos de sus vecinos que necesitara (no bloqueante asi puede
	//empezar a calcular y poner una barrera cuando vaya a calcular un valor del cual requiera un vecino)


	float **matrizOriginal; //Puntero a la matriz original
	float **matrizCopia;    //Puntero a la matriz auxiliar utilizada para copiar los nuevos valores
	float **aux;            //Puntero auxiliar


	//Reservo espacio para la matriz original
	//printf("RESERVANDO DE MEMORIA...\n");
	matrizOriginal  = (float **)malloc(sizeof(float *) * alto);
	*matrizOriginal = (float *)malloc(sizeof(float) * alto * ancho);

	//Reservo espacio para la matriz copia
	matrizCopia  = (float **)malloc(sizeof(float *) * alto);
	*matrizCopia = (float *)malloc(sizeof(float) * alto * ancho);

	if (matrizCopia == NULL || matrizOriginal == NULL)
	{
		fprintf(f, "ERROR: Memoria insuficiente\n");
		exit(5);
	}

	//Creo los punteros para cada valor de fila
	for (m = 0; m < alto; m++)
	{
		matrizOriginal[m] = (*matrizOriginal + ancho * m);
		matrizCopia[m]    = (*matrizCopia + ancho * m);
	}

	//Inicializo matrizOriginal
	int x = i*alto  + (i >= cantFilasExtra ? cantFilasExtra : 0);
	int y = j*ancho + (j >= cantColExtra ? cantColExtra : 0);
	for (m = 0; m < alto; m++)
	{
		for (n = 0; n < ancho; n++)
		{
			matrizOriginal[m][n] = (x+m) * (Tlado - (x+m) - 1) * (y+n) * (Tlado - (y+n) - 1);
			matrizCopia[m][n]    = 0;
			// Donde estan los i y j entre parentesis se deberia cambiar para que cada proceso
			// inicialice el valor como corresponde
		}
	}


	int rankVecinoArriba    = rank-cantCol;
	int rankVecinoAbajo     = rank+cantCol;
	int rankVecinoIzquierda = rank-1;
	int rankVecinoDerecha   = rank+1;

	MPI_Datatype columnaMPI;
	MPI_Type_vector(alto, 1, ancho, MPI_FLOAT, &columnaMPI);
	MPI_Type_commit(&columnaMPI);

	float *filaArriba;
	float *filaAbajo;
	float *colIzq;
	float *colDer;
	
	//Estas dos variables se podrian quitar y colocarlas directamente en los 'for'
	int comienzoIteracionColDer = 1; 
	int comienzoIteracionFilAr  = 1; 

	//Estas dos variables se podrian quitar y colocarlas directamente en los 'for'
	int finIteracionColDer = alto-1; 
	int finIteracionFilAr  = ancho-1;  

	MPI_Request requestArriba;
	MPI_Request requestAbajo;
	MPI_Request requestIzq;
	MPI_Request requestDer;

	bool hayAlguienArriba = i-1 >= 0;
	bool hayAlguienAbajo  = i+1 < cantFilas;
	bool hayAlguienIzq    = j-1 >= 0;
	bool hayAlguienDer    = j+1 < cantCol;

	if (hayAlguienArriba)
	{
		filaArriba = malloc(sizeof(float) * ancho);
	}

	if (hayAlguienAbajo)
	{
		filaAbajo = malloc(sizeof(float) * ancho);
	}

	if (hayAlguienIzq)
	{
		colIzq = malloc(sizeof(float) * alto);
	}

	if (hayAlguienDer)
	{
		colDer = malloc(sizeof(float) * alto);
	}


	double tiempo_trans = 0;
	//POR AHORA TODOS LOS TAGS SON 1. SI ENVIAS UN DATO CON UN TAG, EL QUE LO RECIBE TIENE QUE ESPERAR EL MISMO TAG (SINO SE TRABA)
	//fprintf(f, "COMENZANDO COMPUTO...\n");
	tiempo_trans = sampleTime();
	for (p = 0; p < pasos; p++)
	{
		if (hayAlguienArriba)
		{ //Envio de fila a arriba
			MPI_Isend(matrizOriginal[0], ancho, MPI_FLOAT, rankVecinoArriba, 1, MPI_COMM_WORLD, &requestArriba); //Arriba
			MPI_Irecv(filaArriba, ancho, MPI_FLOAT, rankVecinoArriba, 1, MPI_COMM_WORLD, &requestArriba); //Arriba
		}

		if (hayAlguienAbajo)
		{ //Envio de fila a abajo
			MPI_Isend(matrizOriginal[alto-1], ancho, MPI_FLOAT, rankVecinoAbajo, 1, MPI_COMM_WORLD, &requestAbajo);  //Abajo
			MPI_Irecv(filaAbajo, ancho, MPI_FLOAT, rankVecinoAbajo, 1, MPI_COMM_WORLD, &requestAbajo); //Abajo
		}

		if (hayAlguienIzq)
		{//Envio de columna a izquierda
			// printf("[%d] Tengo uno a mi izquierda\n",rank);
			MPI_Isend(matrizOriginal[0], 1, columnaMPI, rankVecinoIzquierda, 1, MPI_COMM_WORLD, &requestIzq);
			MPI_Irecv(colIzq, alto, MPI_FLOAT, rankVecinoIzquierda, 1, MPI_COMM_WORLD, &requestIzq);
		}
		
		if (hayAlguienDer)
		{ //Envio de columna a derecha   
			// printf("[%d] Tengo uno a mi derecha\n",rank);
			MPI_Isend(&matrizOriginal[0][ancho-1], 1, columnaMPI, rankVecinoDerecha, 1, MPI_COMM_WORLD, &requestDer);
			MPI_Irecv(colDer, alto, MPI_FLOAT, rankVecinoDerecha, 1, MPI_COMM_WORLD, &requestDer);
		}


		//Estas iteraciones se pueden hacer independientes de los buffers que recibimos, por lo que podriamos solaparlo con la comunicacion
		// printf("Voy a calcular los elementos internos de la submatriz\n");
		for (m = 1; m < alto-1; m++) 
		{
			for (n = 1; n < ancho-1; n++)
			{
				matrizCopia[m][n] = 
				matrizOriginal[m][n] + 
					Cx * (matrizOriginal[m+1][n] + 
					matrizOriginal[m-1][n] - 2 * matrizOriginal[m][n]) + 
					Cy * (matrizOriginal[m][n+1] + 
					matrizOriginal[m][n-1] - 2 * matrizOriginal[m][n]);
			}
		}
		//fprintf(f, "Termine calculando los elemnentos internos\n");


		// Luego, habria un for por cada lado que depende de un buffer vecino
		if (hayAlguienArriba)
		{  // Fila de arriba
			MPI_Wait(&requestArriba, MPI_STATUS_IGNORE);
			for (n = comienzoIteracionFilAr; n < finIteracionFilAr; n++)
			{
				matrizCopia[0][n] = 
					matrizOriginal[0][n] + 
					Cx * (matrizOriginal[1][n] + 
					filaArriba[n] - 2 * matrizOriginal[0][n]) + 
					Cy * (matrizOriginal[0][n+1] + 
					matrizOriginal[0][n-1] - 2 * matrizOriginal[0][n]); 
			}
			
			if(hayAlguienIzq){
				MPI_Wait(&requestIzq, MPI_STATUS_IGNORE);
				matrizCopia[0][0] = 
					matrizOriginal[0][0] + 
					Cx * (matrizOriginal[1][0] + 
					filaArriba[0] - 2 * matrizOriginal[0][0]) + 
					Cy * (matrizOriginal[0][1] + 
					colIzq[0] - 2 * matrizOriginal[0][0]); 
			
			}
			
			if(hayAlguienDer){
				MPI_Wait(&requestDer, MPI_STATUS_IGNORE);
				matrizCopia[0][ancho-1] = 
					matrizOriginal[0][ancho-1] + 
					Cx * (matrizOriginal[1][ancho-1] + 
					filaArriba[ancho-1] - 2 * matrizOriginal[0][ancho-1]) + 
					Cy * (colDer[0] + 
					matrizOriginal[0][ancho-2] - 2 * matrizOriginal[0][ancho-1]); 
			
			}
		}

		if (hayAlguienAbajo)
		{  // Fila de abajo
			MPI_Wait(&requestAbajo, MPI_STATUS_IGNORE);
			for (n = comienzoIteracionFilAr; n < finIteracionFilAr; n++)
			{
				matrizCopia[alto-1][n] = 
					matrizOriginal[alto-1][n] + 
					Cx * (filaAbajo[n] + 
					matrizOriginal[alto-2][n] - 2 * matrizOriginal[alto-1][n]) + //El alto-2 se rompe si se divide por filas y las filas son muy "finas"
					Cy * (matrizOriginal[alto-1][n+1] + 
					matrizOriginal[alto-1][n-1] - 2 * matrizOriginal[alto-1][n]);
			}
			
			if(hayAlguienIzq){
				MPI_Wait(&requestIzq, MPI_STATUS_IGNORE);
				matrizCopia[alto-1][0] = 
					matrizOriginal[alto-1][0] + 
					Cx * (filaAbajo[0] + 
					matrizOriginal[alto-2][0] - 2 * matrizOriginal[alto-1][0]) + 
					Cy * (matrizOriginal[alto-1][1] + 
					colIzq[alto-1] - 2 * matrizOriginal[alto-1][0]); 
			
			}	
			
			if(hayAlguienDer){
				MPI_Wait(&requestDer, MPI_STATUS_IGNORE);
				matrizCopia[alto-1][ancho-1] = 
					matrizOriginal[alto-1][ancho-1] + 
					Cx * (filaAbajo[ancho-1] + 
					matrizOriginal[alto-2][ancho-1] - 2 * matrizOriginal[alto-1][ancho-1]) + 
					Cy * (colDer[alto-1] + 
					matrizOriginal[alto-1][ancho-2] - 2 * matrizOriginal[alto-1][ancho-1]); 
			
			}
		}

		if (hayAlguienIzq)
		{  // Columna izquierda
			MPI_Wait(&requestIzq, MPI_STATUS_IGNORE);
			for (m = comienzoIteracionColDer; m < finIteracionColDer; m++)
			{
				matrizCopia[m][0] = 
					matrizOriginal[m][0] + 
					Cx * (matrizOriginal[m+1][0] + 
					matrizOriginal[m-1][0] - 2 * matrizOriginal[m][0]) + 
					Cy * (matrizOriginal[m][1] + 
					colIzq[m] - 2 * matrizOriginal[m][0]);
			}
		}
		
		if (hayAlguienDer)
		{  // Columna derecha
			MPI_Wait(&requestDer, MPI_STATUS_IGNORE);
			for (m = comienzoIteracionColDer; m < finIteracionColDer; m++)
			{
				matrizCopia[m][ancho-1] = 
					matrizOriginal[m][ancho-1] + 
					Cx * (matrizOriginal[m+1][ancho-1] + 
					matrizOriginal[m-1][ancho-1] - 2 * matrizOriginal[m][ancho-1]) + 
					Cy * (colDer[m] + 
					matrizOriginal[m][ancho-2] - 2 * matrizOriginal[m][ancho-1]);
			}
		}


		// Realizo el cambio de punteros para que matrizOriginal (P) apunte a la nueva matriz (P+1)
		aux            = matrizOriginal;
		matrizOriginal = matrizCopia;
		matrizCopia    = aux;

		// ANTES DE IR AL SIGUIENTE PASO SE DEBERIA HACER UN WAIT (O ALGO ASI) PARA ESPERAR A LOS OTROS PROCESOS
		MPI_Barrier(MPI_COMM_WORLD);
	}


	tiempo_trans = sampleTime() - tiempo_trans;
	//printf("COMPUTO FINALIZADO: OK\n");
	//printf("[%d] Tiempo transcurrido: %.21f\n",rank,tiempo_trans );
	if (hayAlguienArriba)
	{
		free(filaArriba);
	}

	if (hayAlguienAbajo)
	{
		free(filaAbajo);
	}

	if (hayAlguienIzq)
	{
		free(colIzq);
	}

	if (hayAlguienDer)
	{
		free(colDer);
	}

	// IMPRIMIR EN FICHERO LOS RESULTADOS DE LAS SUBMATRICES
	// NOTA: PARA EVALUAR TIEMPOS DE EJECUCION SE PODRIA COMENTAR ESTA SECCION 
	for (m = 0; m < alto; m++)
	{
		for (n = 0; n < ancho; n++)
		{
			fprintf(f, "%0.1f\t", matrizOriginal[m][n]);
		}
		fprintf(f, "\n");
	}

	free(matrizCopia);
	free(matrizOriginal);

	MPI_Finalize();

	return 0;
}

void divisionOptima(int cantProcesos, int *filas, int *columnas)
{
	int i;
	int optimo = cantProcesos;
	int fila_tmp, col_tmp, distancia;

	for (i = cantProcesos; i > 0; i--)
	{
		if ( !(cantProcesos % i) ) // Si i es un divisor
		{
			fila_tmp  = i;
			col_tmp   = cantProcesos / i;
			distancia = fila_tmp - col_tmp;
			// termina para evitar repetidos (3x2 == 2x3)
			if (distancia < 0)
			{
				break;
			}

			if (distancia < optimo)
			{
				*filas    = fila_tmp;
				*columnas = col_tmp;
				optimo    = distancia;
			}
		}
	}
}

double sampleTime(void)
{
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);

	return ((double)tv.tv_sec+((double)tv.tv_nsec)/1000000000.0);
}

/*
void copiarAArchivo() // NO USAR, SOLO PARA REFERENCIA DE COMO ESCRIBIR EN UN ARCHIVO
{
	FILE *f = fopen("output.txt", "w");
	if(f == NULL){
		printf("ERROR: No se pudo abrir el archivo");
		exit(1);
	}

	fprintf(f,"Matriz Final - %d\n", pasos);
	for (i = 0; i <  Tlado; i++) {
		for (j = 0; j < Tlado; j++){
			fprintf(f, "%8.3f \t", matrizOriginal[i][j]);
		}
		fprintf(f,"\n");
	}
}
*/
