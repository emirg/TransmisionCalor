#include <stdio.h>
#include <math.h>

const int  N = 100000;

struct Rango {
	int ranki;
	int rankj;
	int inicioFila;
	int inicioCol;
	int finFila;
	int finCol;
	//int esCuadrado; //1 si es cuadrado, 0 si no es cuadrado
};

void divisionOptima(int cantProcesos, int *filas, int *columnas)
{
	int i;
	int optimo = cantProcesos;
	int fila_tmp, col_tmp, distancia;
	for (i = cantProcesos; i > 0; i--)
	{
		if ( !(cantProcesos % i) ) // Si i es un divisor
		{
			fila_tmp = i;
			col_tmp = cantProcesos / i;
			distancia = fila_tmp - col_tmp;
			// termina para evitar repetidos (3x2 == 2x3)
			if (distancia < 0)
			{
				break;
			}

			if (distancia < optimo)
			{
				*filas = fila_tmp;
				*columnas = col_tmp;
				optimo = distancia;
			}
		}
	}
}

void dividirMatriz(int cantProcesos, struct Rango *procesos)
{
	int cantFilas, cantCol;
	divisionOptima(cantProcesos,&cantFilas,&cantCol); //Calculamos la division mas optima
	printf("Filas: %d\nColumnas: %d\n",cantFilas,cantCol ); 


	int i, j, h = 0;
	int alto  = N/cantFilas; //Dividimos el alto en la cantidad de filas mas optima
	int ancho = N/cantCol;   //Dividimos el ancho en la cantidad de columnas mas optima
	int acumuladorAlto  = alto-1;
	int acumuladorAncho = ancho-1;
	//struct Rango procesos[cantProcesos];

	for (i = 0; i < cantFilas; i++)
	{
		for (j = 0; j < cantCol; j++)
		{
			struct Rango rng;
			rng.ranki = i;
			rng.rankj = j;
			rng.inicioFila = i * alto;
			rng.inicioCol = j * ancho;
			if (i+1 == cantFilas && j+1 != cantCol) { //Si es el ultimo proceso de las filas pero no de las columnas
			  	rng.finFila = N-1;
			  	rng.finCol = acumuladorAncho;
			  	//printf("Proceso (%d,%d): (%d - %d) / (%d - %d)\n", rng.ranki,rng.rankj,rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
			  	procesos[h] = rng;
			} else if (i+1 != cantFilas && j+1 == cantCol) { //Si es el ultimo proceso de las columnas pero no de las filas
			  	rng.finFila = acumuladorAlto;
			  	rng.finCol = N-1;
			  	//printf("Proceso (%d,%d): (%d - %d) / (%d - %d)\n", rng.ranki,rng.rankj,rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
			  	procesos[h] = rng;
			} else if (i+1 == cantFilas && j+1 == cantCol) { //Si es el ultimo proceso de las filas y de las columnas
			  	rng.finFila = N-1;
			  	rng.finCol = N-1;
			  	//printf("Proceso (%d,%d): (%d - %d) / (%d - %d)\n", rng.ranki,rng.rankj,rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
			  	procesos[h] = rng;
			} else { //Si no es un proceso de la ultima fila ni columna
			  	rng.finFila = acumuladorAlto;
			  	rng.finCol = acumuladorAncho;
			  	//printf("Proceso (%d,%d): (%d - %d) / (%d - %d)\n", rng.ranki,rng.rankj,rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
			  	procesos[h] = rng;
			}
			acumuladorAncho += ancho;
			h++;
		}
		acumuladorAncho = ancho-1;
		acumuladorAlto += alto;
	}
}

int main(void)
{
	int i = 0, cantProcesos = 21;
	struct Rango proc[cantProcesos];
	dividirMatriz(cantProcesos,proc); 

	//Imprimo los valores desde el arreglo "procesos" para asegurarme que se guardaron correctamente
	for (i = 0; i < cantProcesos; i++)
	{
		//struct Rango rng = *(proc+i);
		struct Rango rng = proc[i];
	  	printf("Proceso (%d,%d): (%d - %d) / (%d - %d)\n", rng.ranki,rng.rankj,rng.inicioFila,rng.finFila,rng.inicioCol,rng.finCol);
	}

	return 0;
}
