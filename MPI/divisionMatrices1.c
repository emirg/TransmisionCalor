#include <stdio.h>
#include <math.h>

const int  N = 85000;

struct Rango{
	int rank;
	int inicioFila;
	int inicioCol;
	int finFila;
	int finCol;
	//int esCuadrado; //1 si es cuadrado, 0 si no es cuadrado
};

int dividirMatriz(int cantProcesos){

	// 1) DIVIDIR EL ALTO DE LA MATRIZ EN 2
	// 2) DIVIDIR LA CANTIDAD DE PROCESOS EN 2, Y DIVIDIR CADA MITAD POR LA CANTIDAD DE PROCESOS DIVIDIDOS 
	// 2.5) SI LA CANTIDAD TOTAL NO ES DIVISIBLE POR 2, UNA MITAD DIVIDIRLA EN UNA PARTE MAS
	// 3) SI EL ANCHO DE LA MATRIZ NO ES DIVISIBLE POR LA CANTIDAD DE PROCESOS, QUE LA ULTIMA SUBMATRIZ SEA MAS ANCHA PARA ABARCAR LA TOTALIDAD

	//CORRECIONES POR HACER
	// Tratar de dividir en cantidades mas grandes el ancho (fijarse el incremento)
	// Comprobar que las comprobaciones para el ancho son correctas
	// PROBLEMAS CON 11x11 con 14 procesos

	int alto = 0, altoAux = 0, mitadProcesos = 0, mitadProcesosAux= 0;
	
	// (1)
	if(N % 2 == 0){
		alto = N/2;
		altoAux = alto;
	}else{
		alto = N/2;
		altoAux = alto + 1;
	}

	// (2) y (2.5)
	if(cantProcesos % 2 == 0){
		mitadProcesos = cantProcesos/2;
		mitadProcesosAux = mitadProcesos;
	}else{
		mitadProcesos = cantProcesos/2;
		mitadProcesosAux = mitadProcesos + 1; //Tambien se podria hacer cantidadProcesos - mitadProcesos;
	}

	// (3)
	int i,j;
	int anchoAcumulador = 0;

	struct Rango procesos[cantProcesos];

	if(mitadProcesos != 0 && mitadProcesosAux != 0){ //Si es una cantidad de procesos distinta de 1
		if(fmod(sqrt(cantProcesos),1)!=0){ //Si es una cantidad que no se puede dividir en cantidades iguales
			int incrementoAnchoSup = N/mitadProcesos;//round((double)N/(double)mitadProcesos); //
			int incrementoAnchoInf = N/mitadProcesosAux; //round((double)N/(double)mitadProcesosAux); //
	 
			//for para la parte superior
			for (i = 0; i < mitadProcesos; i++)
			{
				struct Rango rng;
				rng.rank = i;
				rng.inicioFila = 0;
			  	rng.inicioCol = anchoAcumulador;
			  	rng.finFila = alto-1;

			  	int anchoSig = ((anchoAcumulador + incrementoAnchoSup*2) - 1) < N ? ((anchoAcumulador + incrementoAnchoSup*2) - 1) - (anchoAcumulador + incrementoAnchoSup): N - (anchoAcumulador + incrementoAnchoSup);
			  	//printf("Siguiente: (%d - %d)\n",(anchoAcumulador + incrementoAnchoSup),((anchoAcumulador + incrementoAnchoSup*2) - 1));
			  	//printf("Ancho Siguiente: %d\n",anchoSig);
			  	if((anchoAcumulador + incrementoAnchoSup) - 1 < N && anchoSig < incrementoAnchoSup && i+1 < mitadProcesos){
			  		rng.finCol = (anchoAcumulador + incrementoAnchoSup) - 1;
			  	}else{
			  		rng.finCol = N-1;
			  	}
			  	anchoAcumulador = anchoAcumulador + incrementoAnchoSup;
				printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
				procesos[i] = rng;
					//printf("Proceso: %d\n",i);
			}

			anchoAcumulador = 0;
			//for para la parte inferior
			// ((anchoAcumulador + incrementoAnchoSup*2)) - ((anchoAcumulador + incrementoAnchoSup))  < incrementoAnchoSup
			for (j = 0; j < mitadProcesosAux; j++)
			{
				struct Rango rng;
				rng.rank = i + j ;
				rng.inicioFila = alto;
			  	rng.inicioCol = anchoAcumulador;
			  	rng.finFila = N-1;

			  	int anchoSig = ((anchoAcumulador + incrementoAnchoInf*2) - 1) < N ? ((anchoAcumulador + incrementoAnchoInf*2) - 1) - (anchoAcumulador + incrementoAnchoInf):N- (anchoAcumulador + incrementoAnchoInf) ;
			  	//printf("Siguiente ancho: %d\n",anchoSig );

			  	if((anchoAcumulador + incrementoAnchoInf) - 1 < N && anchoSig < incrementoAnchoInf && j+1 < mitadProcesosAux){
			  		rng.finCol = (anchoAcumulador + incrementoAnchoInf) - 1;
			  	}else{
			  		rng.finCol = N-1;
			  	}
			  	anchoAcumulador = anchoAcumulador + incrementoAnchoInf;
				printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
				procesos[i+j] = rng;
				//printf("Proceso: %d\n",i+j );
			}

		}else{ //Si es una cantidad de procesos que puede ser dividida en partes iguales
			int raizProcesos = (int) sqrt(cantProcesos); //Con 9 procesos es 3 
			int longBloque = (int) round((float)N/(float)raizProcesos); // Bloques de 3x3 con 9 procesos
			int anchoAcumulador=longBloque-1, altoAcumulador=longBloque-1;

			for (int i = 0; i < raizProcesos; i++) //Fila
			{
				for (int j = 0; j < raizProcesos; j++) //Columna
				{
				

				  	if(j < raizProcesos -1 && i<raizProcesos-1){
						struct Rango rng;
						rng.rank = i + j ;
						rng.inicioFila = i*longBloque;
					  	rng.inicioCol = j*longBloque;
					  	rng.finFila=altoAcumulador;
					  	rng.finCol=anchoAcumulador;
  					  	printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);

				  	}else if (j==(raizProcesos-1) && i<raizProcesos-1){

				  		struct Rango rng;
						rng.rank = i + j ;
						rng.inicioFila = i*longBloque;
					  	rng.inicioCol = j*longBloque;
					  	rng.finFila=altoAcumulador;
					  	rng.finCol=N-1;
  					  	printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);

				  	}else if(j < raizProcesos -1 && i==(raizProcesos-1)){
				  		struct Rango rng;
						rng.rank = i + j ;
						rng.inicioFila = i*longBloque;
					  	rng.inicioCol = j*longBloque;
					  	rng.finFila=N-1;
					  	rng.finCol=anchoAcumulador;
  					  	printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);

				  	}else if(j==(raizProcesos-1) && i==(raizProcesos-1)){
				  		struct Rango rng;
						rng.rank = i + j ;
						rng.inicioFila = i*longBloque;
					  	rng.inicioCol = j*longBloque;
					  	rng.finFila=N-1;
					  	rng.finCol=N-1;
  					  	printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);

				  	}

				  	anchoAcumulador+=longBloque;
				}

				anchoAcumulador=longBloque-1;
				altoAcumulador+=longBloque;
			}
		}
	}else{ //Si es un solo proceso
		struct Rango rng;

		rng.finFila=N-1;
		rng.finCol=N-1;
		rng.inicioCol=0;
		rng.inicioFila=0;

		procesos[0]=rng;

		printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);


	}

	return 0;
}

int main(){
	
	dividirMatriz(11);

}


//MAIN
	//int res = sqrt(100/12) ;
	//printf("%d\n", res);
	//int cantProcesos = 7;
	// float tamanoSubmatriz = ((float)(N*N))/(cantProcesos);
	// float anchoSubmatriz = sqrt(tamanoSubmatriz); //Y alto cuando puede ser cuadrada
	// float decimalanchoSubmatriz = fmod(anchoSubmatriz,1);
	// printf("Tamaño: %.21f \n Ancho: %.21f\n Decimal: %.21f\n",tamanoSubmatriz, anchoSubmatriz,decimalanchoSubmatriz);
	// if(decimalanchoSubmatriz > 0.5){
	// 	anchoSubmatriz = (anchoSubmatriz - decimalanchoSubmatriz) + 1;
	// }else{
	// 	anchoSubmatriz = (anchoSubmatriz - decimalanchoSubmatriz);
	// }
	// printf("%.21f\n", anchoSubmatriz);

	// int alto = 0, altoAux = 0, mitadProcesos = 0, mitadProcesosAux= 0;
	// // (1)
	// if(N % 2 == 0){
	// 	alto = N/2;
	// }else{
	// 	alto = N/2;
	// 	altoAux = alto + 1;
	// }

	// // (2)
	// if(cantProcesos % 2 == 0){
	// 	mitadProcesos = cantProcesos/2;
	// }else{
	// 	mitadProcesos = cantProcesos/2;
	// 	mitadProcesosAux = mitadProcesos + 1;
	// }


	// printf("Alto: %d\nAltoAux: %d\n",alto,altoAux);
	// printf("mitadProcesos: %d\nmitadProcesosAux: %d\n",mitadProcesos,mitadProcesosAux);



	// float tamanoSubmatriz = ((float)(N*N))/(cantProcesos);
	// float anchoSubmatriz = sqrt(tamanoSubmatriz); //Y alto cuando puede ser cuadrada
	// float decimalanchoSubmatriz = fmod(anchoSubmatriz,1);
	// if(decimalanchoSubmatriz > 0.5){
	// 	anchoSubmatriz = (anchoSubmatriz - decimalanchoSubmatriz) + 1;
	// }else{
	// 	anchoSubmatriz = (anchoSubmatriz - decimalanchoSubmatriz);
	// }
	// //float altoSubmatriz = sqrt(tamanoSubmatriz);
	// for (int i = 0; i < cantProcesos; i++)
	// {
	// 	struct Rango rng;
	// 	// if(){ //Si puedo crear una matriz cuadrada
	// 	// 	rng.inicioFila = altoSub;
	//  // 		rng.inicioCol = anchoSub;
	//  // 		rng.finFila = altoSub + pedazoAlto - 1;
	//  // 		rng.finCol = anchoSub + pedazoAncho - 1;
 // 	// 	}else{ //Si no puedo crear una matriz cuadrada
	// 	// 	rng.inicioFila = altoSub;
	//  // 		rng.inicioCol = anchoSub;
	//  // 		rng.finFila = altoSub + pedazoAlto - 1;
	//  // 		rng.finCol = anchoSub + pedazoAncho - 1;
 // 	// 	}	
 //        printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
	// }



 //    int ancho = cantProcesos/2; 
 //    int alto = 2;
 //    struct Rango procesoRango[cantProcesos];
 //    int pedazoAncho=N/ancho;
 //    int pedazoAlto = N/alto;
	// int h = 0,j,i,anchoSub=0,altoSub=0;
	// for (int i = 0; i < cantProcesos; i++)
	// {
	// 	struct Rango rng;
 // 		rng.inicioFila = altoSub;
 // 		rng.inicioCol = anchoSub;
 // 		rng.finFila = altoSub + pedazoAlto - 1;
 // 		rng.finCol = anchoSub + pedazoAncho - 1;
 //        printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
 //        if(rng.finCol == N-1){
 //        	altoSub = altoSub + pedazoAlto;
 //        	anchoSub = 0;
 //        }else{
 //        	anchoSub = anchoSub +pedazoAncho;
 //    	}
	// }


	// for(i=0; i<N; i+=pedazo){
	// 	for(j=0; j<N; j+=pedazo){  
	// 		struct Rango rng;
	// 		rng.inicioFila = i;
	// 		rng.inicioCol = j;
	// 		rng.finFila = i + pedazo - 1;
	// 		rng.finCol = j + pedazo - 1;
 //            printf("(%d - %d) / (%d - %d)\n", rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
	// 		//procesoRango[h] = rng;
	// 		//h++;
	// 	}
	// }