
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>

// Modulo principal
int main(int argc,char *argv[]){


	//TODO: Esquema según especificado en la práctica.

    // Define variables locales



    // Coge semáforos y memoria compartida


    
    // Se pone en estado de libre incrementando el número de teléfonos libres
    while(1){

        // Mensaje de Espera
        printf("Teléfono [%d] en espera...\n",pid);
        

		//TODO: Aquí hay que realizar procesos


        // Mensaje de en conversacion
        printf("Teléfono [%d] en conversacion... Nº Llamadas en espera: %d\n",pid,valorEspera);

        // Espera en conversación
        sleep(rand() % 10 + 10);
        
    }

    return EXIT_SUCCESS;
}