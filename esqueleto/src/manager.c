#include <errno.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include <definitions.h>
#include <memoriaI.h>
#include <semaforoI.h>

void procesar_argumentos(int argc, char *argv[], int *numTelefonos, int *numLineas);
void instalar_manejador_senhal();
void manejador_senhal(int sign);
void iniciar_tabla_procesos(int n_procesos_telefono, int n_procesos_linea);
void crear_procesos(int numTelefonos, int numLineas);
void lanzar_proceso_telefono(const int indice_tabla);
void lanzar_proceso_linea(const int indice_tabla);
void esperar_procesos();
void terminar_procesos(void);
void terminar_procesos_especificos(struct TProcess_t *process_table, int process_num);
void liberar_recursos();

int g_telefonosProcesses = 0;
int g_lineasProcesses = 0;
struct TProcess_t *g_process_telefonos_table;
struct TProcess_t *g_process_lineas_table;

int main(int argc, char *argv[])
{
    // Define variables locales
    int numTelefonos;
    int numLineas;

    // Procesa los argumentos y los guarda en las dos variables
    procesar_argumentos(argc, argv, &numTelefonos, &numLineas);

    // Creamos semáforos y memoria compartida
    crear_sem(MUTEXESPERA, 1);
    crear_sem(TELEFONOS, 0);
    crear_sem(LINEAS, 0);
    crear_var(LLAMADASESPERA, 0); // No hay llamadas en espera

    // Manejador de Ctrl-C
    instalar_manejador_senhal();

    // Crea Tabla para almacenar los pids de los procesos
    iniciar_tabla_procesos(numTelefonos, numLineas);

    // Tenemos todo
    // Lanzamos los procesos
    crear_procesos(numTelefonos, numLineas);

    // Esperamos a que finalicen las lineas
    esperar_procesos();

    // Matamos los telefonos y cualquier otro proceso restante
    terminar_procesos();

    // Finalizamos Manager
    printf("\n[MANAGER] Terminacion del programa (todos los procesos terminados).\n");
    liberar_recursos();

    return EXIT_SUCCESS;
}

// TODO: Realizar todas las funciones necesarias.
void procesar_argumentos(int argc, char *argv[], int *numTelefonos, int *numLineas)
{
}

void instalar_manejador_senhal()
{
    if (signal(SIGINT, manejador_senhal) == SIG_ERR)
    {
        fprintf(stderr, "[MANAGER] Error al instalar el manejador se senhal: %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void manejador_senhal(int sign)
{
    printf("\n[MANAGER] Terminacion del programa (Ctrl + C).\n");
    terminar_procesos();
    liberar_recursos();
    exit(EXIT_SUCCESS);
}

void iniciar_tabla_procesos(int n_procesos_telefono, int n_procesos_linea)
{
    g_process_lineas_table = malloc(n_procesos_linea * sizeof(TProcess_t));
    g_process_telefonos_table = malloc(n_procesos_telefono * sizeof(TProcess_t));
    for (int i = 0; i < n_procesos_telefono; i++)
    {
        g_process_telefonos_table[i] = 0;
    }
    for (int i = 0; i < n_procesos_linea; i++)
    {
        g_process_lineas_table[i] = 0;
    }
}

void crear_procesos(int numTelefonos, int numLineas)
{
    int indice_tabla = 0;
    for (int i = 0; i < numTelefonos; i++)
    {
        lanzar_proceso_telefono(indice_tabla++);
    }
    printf("[MANAGER] %d teléfonos creadas.\n", numTelefonos);
    for (int i = 0; i < numLineas; i++)
    {
        lanzar_proceso_linea(indice_tabla++);
    }
    printf("[MANAGER] %d lineas creadas.\n", numLineas);
}

void lanzar_proceso_telefono(const int indice_tabla)
{
    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        fprintf(stderr, "[MANAGER] Error al lanzar proceso teléfono: %s.\n", strerror(errno));
        terminar_procesos();
        liberar_recursos();
        exit(EXIT_FAILURE);
    case 0:
        if (execl(RUTA_TELEFONO, CLASE_TELEFONO, NULL) == -1)
        {
            fprintf(stderr, "[MANAGER] Error usando execl() en el poceso %s: %s.\n", CLASE_TELEFONO, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    g_process_telefonos_table[indice_tabla].pid = pid;
    g_process_telefonos_table[indice_tabla].clase = CLASE_TELEFONO;
}

void lanzar_proceso_linea(const int indice_tabla)
{
    pid_t pid;
    switch (pid = fork())
    {
    case -1:
        fprintf(stderr, "[MANAGER] Error al lanzar proceso línea: %s.\n", strerror(errno));
        terminar_procesos();
        liberar_recursos();
        exit(EXIT_FAILURE);
    case 0:
        if (execl(RUTA_LINEA, CLASE_LINEA, NULL) == -1)
        {
            fprintf(stderr, "[MANAGER] Error usando execl() en el poceso %s: %s.\n", CLASE_LINEA, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    g_process_lineas_table[indice_tabla].pid = pid;
    g_process_lineas_table[indice_tabla].clase = CLASE_LINEA;
}

void esperar_procesos()
{
    int i, n_processes = g_lineasProcesses;
    pid_t pid;

    while (n_processes > 0)
    {
        pid = wait(NULL);
        for (i = 0; i < g_lineasProcesses; i++)
        {
            if (pid == g_process_lineas_table[i].pid)
            {
                printf("[MANAGER] Proceso línea terminado [%d]...\n", g_process_lineas_table[i].pid);
                g_process_lineas_table[i].pid = 0;
                n_processes--;
                break;
            }
        }
    }
    n_processes = g_telefonosProcesses;
    while (n_processes > 0)
    {
        pid = wait(NULL);
        for (i = 0; i < g_telefonosProcesses; i++)
        {
            if (pid == g_telefonosProcesses[i].pid)
            {
                printf("[MANAGER] Proceso teléfono terminado [%d]...\n", g_process_telefonos_table[i].pid);
                g_process_telefonos_table[i].pid = 0;
                n_processes--;
                break;
            }
        }
    }
}

void terminar_procesos_especificos(struct TProcess_t *process_table, int process_num)
{
    printf("[MANAGER] Terminando proceso %s [%d]...\n", process_table[i].clase, process_table[i].pid);
    if (kill(process_table[i].pid, SIGINT) == -1)
    {
        fprintf(stderr, "[MANAGER] Error al usar kill() en proceso %d: %s.\n", process_table[i].pid, strerror(errno));
    }
}

void terminar_procesos()
{
    int i;
    printf("\n--- Finalizando procesos ---\n");
    for (i = 0; i < g_process_lineas_table; i++)
    {
        if (g_process_lineas_table[i].pid != 0)
        {
            printf("[MANAGER] Terminando proceso %s [%d]...\n", g_process_lineas_table[i].clase, g_process_lineas_table[i].pid);
            if (kill(g_process_lineas_table[i].pid, SIGINT) == -1)
            {
                fprintf(stderr, "[MANAGER] Error al usar kill() en proceso %d: %s.\n", g_process_lineas_table[i].pid, strerror(errno));
            }
        }
    }
     for (i = 0; i < g_telefonosProcesses; i++)
    {
        if (g_process_telefonos_table[i].pid != 0)
        {
            printf("[MANAGER] Terminando proceso %s [%d]...\n", g_process_telefonos_table[i].clase, g_process_telefonos_table[i].pid);
            if (kill(g_process_telefonos_table[i].pid, SIGINT) == -1)
            {
                fprintf(stderr, "[MANAGER] Error al usar kill() en proceso %d: %s.\n", g_process_telefonos_table[i].pid, strerror(errno));
            }
        }
    }
}

void liberar_recursos()
{
    free(g_process_telefonos_table);
    free(g_process_lineas_table);
}
