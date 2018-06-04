#include <stdio.h>
#include <stdlib.h>



int main(){
	
	// int r = 3, c = 6, i, j, count;
	//  float **arr = (float **)malloc(r * sizeof(float *));
	// // for (i=0; i<r; i++)
	// // 	arr[i] = (int *)malloc(c * sizeof(int));	
	 

 //    arr  = (float **)malloc(sizeof(float *) * r);
 //    *arr = (float *)malloc(sizeof(float) * r * c);

 //    for(i = 0; i < r; i++)
 //    {
 //        arr[i] = (*arr + c * i);
 //    }

 //    count = 0;
 //    for (i = 0; i <  r; i++)
 //      for (j = 0; j < c; j++)
 //         arr[i][j] = ++count;  // OR *(*(arr+i)+j) = ++countk
 
 //    for (i = 0; i <  r; i++)
 //    {
 //      for (j = 0; j < c; j++)
 //      {
 //         printf("%p ", &arr[i][j]);
 //      }
 //      printf("\n");
 // 	}
   /* Code for further processing and free the 
      dynamically allocated memory */
  float **matrizOriginal;
  int alto = 3;
  int ancho = 3;
        matrizOriginal  = (float **)malloc(sizeof(float *) * alto);
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
          printf("(%d,%d) -> Numero: %0.8f  -  Memoria: %p\n",i,j,matrizOriginal[i][j],&matrizOriginal[i][j] );
        }
      }
 
   return 0;

}