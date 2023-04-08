
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>

// Modulo principal
int main(int argc,char *argv[]){

	//TODO: Esquema especificado en la práctica.
	pid_t pid = getpid();
    
	
    // Coge semáforos y memoria compartida
    sem_t *semaforo= get_sem(MUTEXESPERA);
    sem_t *telefonos= get_sem(TELEFONOS);
    //sem_t *lineas= get_sem(LINEAS);
    // Realiza una espera entre 1..60 segundos
   // printf("Linea [%d] esperando llamada...\n",pid);
    //sleep(rand() % 30 + 1);

    //Aumenta las llamadas en espera
    wait_sem(semaforo);
    int i=0;
    int valorEspera= obtener_var(LLAMADASESPERA);
    consultar_var(valorEspera,&i);
    modificar_var(valorEspera, ++i);
   

    // Espera telefono libre
    printf("Linea [%d] esperando telefono libre...Nº Llamadas en espera: %d\n",pid,i);
    signal_sem(semaforo);
    // Lanza la llamada
    //printf("Linea [%d] desviando llamada a un telefono...\n",pid);
    signal_sem(telefonos);
    return EXIT_SUCCESS;
}