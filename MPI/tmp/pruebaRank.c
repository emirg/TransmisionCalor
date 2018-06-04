#include <stdio.h>
#include <math.h>


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

int main(){
	int cantProcesos = 11;
	int cantFilas ;
	int cantCol;

	divisionOptima(cantProcesos,&cantFilas,&cantCol);

	printf("Cantidad Filas: %d\nCantidad Col: %d\n",cantFilas,cantCol );
	int i,j;
	for (int rank = 0; rank < cantProcesos; rank++)
	{
                i = rank/(cantCol); 
                j = rank%(cantCol); 


            printf("Proceso %d: (%d,%d)\n",rank,i,j );
	}



}