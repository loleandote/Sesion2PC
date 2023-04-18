
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>
void telefono();
// Modulo principal
int main(int argc, char *argv[])
{
    telefono();
    return EXIT_SUCCESS;
}
/**
 * Esta función simula una conversación telefónica obteniendo semáforos y memoria compartida, poniendo
 * el teléfono en un estado libre y luego esperando las llamadas entrantes mientras se reduce el número
 * de llamadas en la cola de espera.
 */
void telefono()
{
    pid_t pid = getpid();
    int valorEspera = obtener_var(LLAMADASESPERA);
    int i = 0;
    // Coge semáforos y memoria compartida
    sem_t *telefono = get_sem(TELEFONOS);
    sem_t *mutex = get_sem(MUTEXESPERA);
    sem_t *linea = get_sem(LINEAS);
    // Se pone en estado de libre incrementando el número de teléfonos libres
    while (1)
    {
        // Mensaje de Espera
        printf("Teléfono [%d] en espera...\n", pid);
        signal_sem(telefono);
        // printf("estoy atrapado");
        wait_sem(linea);
        // Obtenemos el número de llamadas en espera
        // decrementamos el numero de llamadas en espera
        wait_sem(mutex);
        consultar_var(valorEspera, &i);
        modificar_var(valorEspera, --i);
        signal_sem(mutex);
        // Mensaje de en conversacion
        printf("Teléfono [%d] en conversacion... Nº Llamadas en espera: %d\n", pid, i);
        // Espera en conversación
        sleep(rand() % 10 + 10);
    }
}