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
	char processor_name[MPI_MAX_PROCESSOR_NAME];

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &cantProcesos);
	MPI_Get_processor_name(processor_name, &namelen);  // nombre del procesador (nodo)


	//ACA PONER LOS PRINTS DE LOS RANKS
	//printf("RANK %d\n",rank );

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

	// int extremo = Tlado-1; // Temporalmente inutilizada

	int q;
	int i, j; // Variables utilizadas para indicar la posicion de cada submatriz en una matriz imaginaria que permite observar que procesos necesitan comunicarse entre si
	int alto, ancho; // Variables que determinaran el ancho y alto de cada submatriz
	int m, n, p; //Variables auxiliares para la iteracion de bucles 
	int cantFilas, cantCol; // Variables usadas para determinar en cuantas filas y columnas esta dividida la matriz original (es decir, cantFilas*cantCol = cantProcesos)

	divisionOptima(cantProcesos, &cantFilas, &cantCol);  //Calculamos la division mas optima

	//printf("Filas: %d\nColumnas: %d\n",cantFilas,cantCol); 
	//(rank,cantCol,cantFoilas) --> (i,j)

	// Posiciones de cada submatriz de acuerdo al rank que le toque a cada proceso
	i = rank / cantCol;
	j = rank % cantCol;

	char nombre[15];
	sprintf(nombre, "subgrid_%d_%d.out", i, j);

	FILE *f = fopen(nombre, "w");
	if (f == NULL)
	{
		printf("ERROR: No se pudo abrir el archivo");
		exit(4);
	}

	bool hayAlguienArriba = i-1 >= 0;
	bool hayAlguienAbajo  = i+1 < cantFilas;
	bool hayAlguienIzq    = j-1 >= 0;
	bool hayAlguienDer    = j+1 < cantCol;

	alto  = Tlado / cantFilas; //Dividimos el alto en la cantidad de filas mas optima
	ancho = Tlado / cantCol;   //Dividimos el ancho en la cantidad de columnas mas optima

	if (j < Tlado % cantCol)
	{
		ancho++;
	}

	if (i < Tlado % cantFilas)
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
	//printf("RESERVA DE MEMORIA: OK\n");
	//Inicializo matrizOriginal
	//printf("INICIALIZANDO...\n");
	for (m = 0; m < alto; m++)
	{
		for (n = 0; n < ancho; n++)
		{
			//matrizOriginal[i][j] = i * (alto - i -1) * j * (ancho - j - 1);  
			matrizOriginal[m][n] = (i*alto + m) * (Tlado - (i*alto + m) -1) * (j*ancho + n) * (Tlado - (j*ancho + n) - 1);
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


	int comienzoIteracionColIzq = 0;
	int comienzoIteracionColDer = 0;
	int comienzoIteracionFilAr  = 0;
	int comienzoIteracionFilAb  = 0;

	int finIteracionColIzq = alto;
	int finIteracionColDer = alto;
	int finIteracionFilAr  = ancho;
	int finIteracionFilAb  = ancho;

	//ESTO CAPAZ QUE FUNCIONA PARA CALCULAR TENIENDO EN CUENTA SI ES O NO UN BORDE (Si se habilita eso, cambiar los valores de arriba a 0 y sacar "-1")
	if(i==0){
		comienzoIteracionColIzq=1;
		comienzoIteracionColDer=1;
	}

	if(j==0){
		comienzoIteracionFilAr=1;
		comienzoIteracionFilAb=1;
	}
	if(i==cantFilas-1){
		finIteracionColIzq=alto-1;
		finIteracionColDer=alto-1;
	}

	if(j==cantCol-1){
		finIteracionFilAr=ancho-1;
		finIteracionFilAb=ancho-1;
	}

	//fprintf(f, "RESERVANDO BUFFERS ADICIONALES...\n");
	if (hayAlguienArriba)
	{
		filaArriba = malloc(sizeof(float) * ancho);
		MPI_Request requestArriba;
	}

	if (hayAlguienAbajo)
	{
		filaAbajo = malloc(sizeof(float) * ancho);
		MPI_Request requestAbajo;
	}

	if (hayAlguienIzq)
	{
		colIzq = malloc(sizeof(float) * alto);
		MPI_Request requestIzq;
	}

	if (hayAlguienDer)
	{
		colDer = malloc(sizeof(float) * alto);
		MPI_Request requestDer;
	}
	//printf("RESERVA BUFFERS ADICIONALES: OK\n");

	//printf("[%d] Voy a empezar a hacer los send\n",rank);

	
	double tiempo_trans = 0;
	//POR AHORA TODOS LOS TAGS SON 1. SI ENVIAS UN DATO CON UN TAG, EL QUE LO RECIBE TIENE QUE ESPERAR EL MISMO TAG (SINO SE TRABA)
	//fprintf(f, "COMENZANDO COMPUTO...\n");
	tiempo_trans = sampleTime();
	for (p = 0; p < pasos; p++)
	{
		//LOS SEND Y RECV YA ESTARIAN, HABRIA QUE COMPROBARLO EN EL CLUSTER (LOS PROBE CON UNOS TEST LOCALES - pruebaSend.c)
		if (hayAlguienArriba)
		{ //Envio de fila a arriba
			MPI_Isend(matrizOriginal[0], ancho, MPI_FLOAT, rankVecinoArriba, 1, MPI_COMM_WORLD, &request); //Arriba
			MPI_Irecv(filaArriba, ancho, MPI_FLOAT, rankVecinoArriba, 1, MPI_COMM_WORLD, &requestArriba); //Arriba
		}
		// printf("[%d] Ya verifique si tengo a alguien arriba y mande lo corespondiente\n",rank);

		if (hayAlguienAbajo)
		{ //Envio de fila a abajo
			MPI_Isend(matrizOriginal[alto-1], ancho, MPI_FLOAT, rankVecinoAbajo, 1, MPI_COMM_WORLD, &request);  //Abajo
			MPI_Irecv(filaAbajo, ancho, MPI_FLOAT, rankVecinoAbajo, 1, MPI_COMM_WORLD, &requestAbajo); //Abajo
		}
		// printf("[%d] Ya verifique si tengo a alguien abajo y mande lo corespondiente\n",rank);

		//ERROR CON EL USO DE VECTORES
		if (hayAlguienDer)
		{ //Envio de columna a derecha   
			// printf("[%d] Tengo uno a mi derecha\n",rank);
			MPI_Isend(&matrizOriginal[0][ancho-1], 1, columnaMPI, rankVecinoDerecha, 1, MPI_COMM_WORLD, &request);
			MPI_Irecv(colDer, alto, MPI_FLOAT, rankVecinoDerecha, 1, MPI_COMM_WORLD, &requestDer);
		}

		if (hayAlguienIzq)
		{//Envio de columna a izquierda
			// printf("[%d] Tengo uno a mi izquierda\n",rank);
			MPI_Isend(matrizOriginal[0], 1, columnaMPI, rankVecinoIzquierda, 1, MPI_COMM_WORLD, &request);
			MPI_Irecv(colIzq, alto, MPI_FLOAT, rankVecinoIzquierda, 1, MPI_COMM_WORLD, &requestIzq);
		}

		// <!> Temporal <!> //
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
			MPI_Wait(&requestArriba,MPI_STATUS_IGNORE);
			for (n = comienzoIteracionFilAr; n < finIteracionFilAr; n++)
			{
				matrizCopia[0][n] = 
					matrizOriginal[0][n] + 
					Cx * (matrizOriginal[1][n] + 
					filaArriba[n] - 2 * matrizOriginal[0][n]) + 
					Cy * (matrizOriginal[0][n+1] + 
					matrizOriginal[0][n-1] - 2 * matrizOriginal[0][n]); 
			}
		}

		if (hayAlguienAbajo)
		{  // Fila de abajo
			MPI_Wait(&requestAbajo,MPI_STATUS_IGNORE);
			for (n = comienzoIteracionFilAb; n < finIteracionFilAb; n++)
			{
				matrizCopia[alto-1][n] = 
					matrizOriginal[alto-1][n] + 
					Cx * (filaAbajo[n] + 
					matrizOriginal[alto-2][n] - 2 * matrizOriginal[alto-1][n]) + //El alto-2 se rompe si se divide por filas y las filas son muy "finas"
					Cy * (matrizOriginal[alto-1][n+1] + 
					matrizOriginal[alto-1][n-1] - 2 * matrizOriginal[alto-1][n]);
			}
		}

		if (hayAlguienDer)
		{  // Columna derecha
			MPI_Wait(&requestDer,MPI_STATUS_IGNORE);
			for (m = comienzoIteracionColDer; m < finIteracionColDer; m++)
			{
				matrizCopia[m][ancho-1] = 
					matrizOriginal[m][ancho-1] + 
					Cx * (matrizOriginal[m+1][ancho-1] + 
					matrizOriginal[m-1][ancho-1] - 2 * matrizOriginal[m][ancho-1]) + 
					Cy * (colDer[n] + 
					matrizOriginal[m][ancho-2] - 2 * matrizOriginal[m][ancho-1]);
			}
		}

		if (hayAlguienIzq)
		{  // Columna izquierda
			MPI_Wait(&requestIzq,MPI_STATUS_IGNORE);
			for (m = comienzoIteracionColIzq; m < finIteracionColIzq; m++)
			{
				matrizCopia[m][0] = 
					matrizOriginal[m][0] + 
					Cx * (matrizOriginal[m+1][0] + 
					matrizOriginal[m-1][0] - 2 * matrizOriginal[m][0]) + 
					Cy * (matrizOriginal[m][1] + 
					colIzq[m] - 2 * matrizOriginal[m][0]);
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

	//PRINT PARA VER LOS RESULTADOS DE LAS SUBMATRICES
	//if(rank == 0){
	for (m = 0; m < alto; m++)
	{
		for (n = 0; n < ancho; n++)
		{
			fprintf(f, "%0.1f\t", matrizOriginal[m][n]);
		}
		fprintf(f, "\n");
	}
	/*}else{
		sleep(1);
		for ( m = 0; m < alto; m++)
		{
			for ( n = 0; n < ancho; n++)
			{
				printf("%0.1f\t",matrizOriginal[m][n]);
			}
			printf("\n");
		}
		printf("\n");
	}*/

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
void dividirMatriz(int cantProcesos) // ESTE METODO YA NO SE USA, TODO EL CODIGO SE ENCUENTRA EN EL MAIN
{
	int cantFilas, cantCol;
	divisionOptima(cantProcesos,&cantFilas,&cantCol); //Calculamos la division mas optima
	printf("Filas: %d\nColumnas: %d\n",cantFilas,cantCol ); 

	//(rank,cantCol,cantFilas) --> (i,j)
	int i = rank%(cantCol-1);
	int j = rank/(cantFilas-1);

	int alto  = N/cantFilas; //Dividimos el alto en la cantidad de filas mas optima
	int ancho = N/cantCol;   //Dividimos el ancho en la cantidad de columnas mas optima
	if ( N % cantCol < j)
	{
		ancho++;
	}

	if (N % cantFilas < i)
	{
		alto++;
	}

	//int acumuladorAlto  = alto-1;
	//int acumuladorAncho = ancho-1;


	// struct Rango rng;
	// rng.ranki = i;
	// rng.rankj = j;
	// rng.inicioFila = i * alto;
	// rng.inicioCol = j * ancho;
}
*/

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
