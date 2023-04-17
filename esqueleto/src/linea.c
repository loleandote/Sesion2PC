#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>

int main(int argc,char *argv[]){

	//TODO: Esquema especificado en la práctica.
	pid_t pid = getpid();
    int valorEspera =  obtener_var (LLAMADASESPERA);
    int i =0;
    // Coge semáforos y memoria compartida
    sem_t *telefono= get_sem(TELEFONOS);
    sem_t  *mutex = get_sem(MUTEXESPERA);
    sem_t *linea = get_sem(LINEAS);
   // No hay llamadas en espera

    // Realiza una espera entre 1..60 segundos
    printf("Linea [%d] esperando llamada...\n",pid);
    sleep(rand() % 30 + 1);
    
    //Aumenta las llamadas en espera
    wait_sem(mutex);
    consultar_var(valorEspera, &i);
    modificar_var(valorEspera,++i);
    signal_sem(mutex);
    printf("Linea [%d] esperando telefono libre...Nº Llamadas en espera: %d\n",pid,i);
    // Espera telefono libre
    signal_sem(linea);
   // printf("quedo libre");
   
    // Lanza la llamada
    printf("Linea [%d] desviando llamada a un telefono...\n",pid);

    wait_sem(telefono);    
    
    
    return EXIT_SUCCESS;
}
