#include<stdio.h>
#include<mpi.h>
#include<stdlib.h>

int main(int argc, char *argv[]){
// Find out rank, size
	MPI_Init(&argc, &argv);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int alto = 4;
	int ancho = 4;

	MPI_Datatype columnaMPI;
	//MPI_Datatype type2;

    // MPI_Type_contiguous(3, MPI_INT, &type2);
    // MPI_Type_commit(&type2);
	MPI_Type_vector(alto,1,ancho, MPI_FLOAT,&columnaMPI);
	MPI_Type_commit(&columnaMPI);
	//float **matrizOriginal;
	float matrix[4][4]={{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};;

	if(world_rank == 0){	
			

/*	    matrizOriginal  = (float **)malloc(sizeof(float *) * alto);
	    *matrizOriginal = (float *)malloc(sizeof(float) * alto * ancho);
	 	
	 	int m,n;
	 	for(m = 0; m < alto; m++)
	    {
	        matrizOriginal[m] = (*matrizOriginal + ancho * m);
	    }

	    int acum=0;
	    for (int i = 0; i < alto; i++)
	    {
	    	for (int j = 0; j < ancho; j++)
	    	{
	    		matrizOriginal[i][j] = acum;
	    		acum++;
	    	}
	    }

	    for (int i = 0; i < alto; i++)
	    {
	    	for (int j = 0; j < ancho; j++)
	    	{
	    		printf("[0] (%d,%d) -> Numero: %0.1f  -  Memoria: %p\n",i,j,matrizOriginal[i][j],&matrizOriginal[i][j] );
	    	}
	    }*/
	}

	// float fila[3];

	int number;
	if (world_rank == 0) {
	    //number = -1;
	    //MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD)
        MPI_Send(&matrix[0][3],1,columnaMPI,1,1,MPI_COMM_WORLD); 
	    //MPI_Recv(columnaEnvio,alto,MPI_FLOAT,rankVecinoDerecha,-2,MPI_COMM_WORLD,&status); //Derecho
	} else if (world_rank == 1) {
            //MPI_Send(matrizOriginal,1,columnasMPI,0,-1,MPI_COMM_WORLD); //Derecho
	   // columnasMPI col[3];
		float *col = malloc(sizeof(float) * alto);
		int i;
		for (i = 0; i < alto; i++)
		{
		
			printf("[1] Memoria col: %p\n",&col[i]);
		}
	    MPI_Recv(col,alto,MPI_FLOAT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Derecho
		int j;

	    for(j= 0;j < alto; j++){
			printf("[1] Numero recibido: %0.1f - Memoria: %p\n",col[j],&col[j]);
		}
	}


	MPI_Type_free(&columnaMPI);
	MPI_Finalize();
}
