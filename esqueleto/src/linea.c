
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>

// Modulo principal
int main(int argc,char *argv[]){

	//TODO: Esquema especificado en la práctica.
	
	
    // Coge semáforos y memoria compartida


    // Realiza una espera entre 1..60 segundos
    printf("Linea [%d] esperando llamada...\n",pid);
    sleep(rand() % 30 + 1);

    //Aumenta las llamadas en espera


    // Espera telefono libre
    printf("Linea [%d] esperando telefono libre...Nº Llamadas en espera: %d\n",pid,valorEspera);

    // Lanza la llamada
    printf("Linea [%d] desviando llamada a un telefono...\n",pid);


    return EXIT_SUCCESS;
}