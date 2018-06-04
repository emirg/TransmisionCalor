#include <stdio.h>

void algoritmo(int cantProcesos, int *filas, int *columnas)
{
	int i;
	int optimo = cantProcesos;
	int fila_tmp, col_tmp, distancia;
	for (i = cantProcesos; i > 0; i--)
	{
		if ( !(cantProcesos % i) ) // es un divisor
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

int main(void)
{
	int x;
	printf("Ingrese cantidad de procesos: ");
	scanf("%d", &x);
	int f, c;
	algoritmo(x, &f, &c);
	printf("Filas optimas: %d\n", f);
	printf("Columnas optimas: %d\n", c);
	return 0;
}