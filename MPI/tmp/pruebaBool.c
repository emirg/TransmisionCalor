#include<stdio.h>

typedef enum { false, true } bool;

int main(){
	

	int j = 10;
	bool t = j<20;
	if(t){
		printf("Menor a 20\n");
	}else{
		printf("Mayor o iguala 20\n");
	}


}