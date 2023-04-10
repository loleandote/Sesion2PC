
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
    pid_t pid = getpid();
	
    
    // Coge semáforos y memoria compartida


    
    // Se pone en estado de libre incrementando el número de teléfonos libres
    while(1){
        sem_t *telefonos=get_sem(TELEFONOS);
        sem_t *lineas=get_sem(LINEAS);
        // Mensaje de Espera
        printf("Teléfono [%d] en espera...\n",pid);
        

       // wait_sem(telefonos);
		//TODO: Aquí hay que realizar procesos
        int llamadasEspera= obtener_var(LLAMADASESPERA);
        sem_t *semaforo= get_sem(MUTEXESPERA);
        wait_sem(semaforo);

        // Mensaje de en conversacion
        int i= 0;
        consultar_var(llamadasEspera,&i);
        if(i>0){
        modificar_var(llamadasEspera, --i);
        
        printf("Teléfono [%d] en conversacion... Nº Llamadas en espera: %d\n",pid,i);
        
       
        // Espera en conversación
        sleep(rand() % 10 + 10);
        signal_sem(lineas);
        }
        else
         signal_sem(semaforo); 
    }

    return EXIT_SUCCESS;
}