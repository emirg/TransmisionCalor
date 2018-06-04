#include<stdio.h>
#include <stdlib.h>

int main(){

	float *matrizOriginal = malloc(sizeof(float) * 9);
	matrizOriginal[0] = 1;
	matrizOriginal[1] = 2;
	matrizOriginal[2] = 3;
	matrizOriginal[3] = 4;
	matrizOriginal[4] = 5;
	matrizOriginal[5] = 6;
	matrizOriginal[6] = 7;
	matrizOriginal[7] = 8;
	matrizOriginal[8] = 9;

	for(int i = 0; i<9;i++){
		printf("%0.8f",matrizOriginal[i]);
	}
}