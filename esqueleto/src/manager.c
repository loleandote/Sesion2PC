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

// variables globales que usaremos en toda la clase
int g_telefonosProcesses = 0;
int g_lineasProcesses = 0;
// estas variables globales son punteros con estructura TProcess_t ya que las utilizaremos para acceder a tablas de procesos
/*La estructura "TProcess_t" puede contener información relevante sobre los procesos en ejecución, como el identificador de proceso (PID),
el estado del proceso, el tiempo de ejecución, etc.*/
struct TProcess_t *g_process_telefonos_table;
struct TProcess_t *g_process_lineas_table;

int main(int argc, char *argv[])
{
    // Define variables locales
    if (argc != 3)
    {
        printf("Error: Los argumentos pasados por el terminal son incorrectos\n");
        printf("Ejemplo de ejecución: \n ./exec/manager <nTelefonos> <nLineas>\n");
        exit(EXIT_FAILURE);
    }
    int numTelefonos = atoi(argv[1]);
    int numLineas = atoi(argv[2]);

    // Procesa los argumentos y los guarda en las dos variables
    procesar_argumentos(argc, argv, &numTelefonos, &numLineas);

    // Creamos semáforos y memoria compartida
    crear_sem(MUTEXESPERA, 1);    // un semaforo que nos funcionara para manejar la espera
    crear_sem(TELEFONOS, 0);      // semaforo para contorlar los telefonos
    crear_sem(LINEAS, 0);         // semaforo para controlar las lineas
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
} // end main
//--------------------------------------------------------------------------------------

/**
 * La función establece variables globales para el número de procesos telefónicos y de línea en función
 * de los argumentos de entrada.
 *
 * @param argc El número de argumentos pasados al programa desde la línea de comandos, incluido el
 * nombre del propio programa.
 * @param argv Una matriz de cadenas que contienen los argumentos de la línea de comando pasados al
 * programa.
 * @param numTelefonos Un puntero de número entero que se actualizará con el número de procesos
 * telefónicos que se crearán en función de los argumentos de la línea de comandos pasados al programa.
 * @param numLineas numLineas es un puntero a una variable entera que almacenará el número de líneas a
 * procesar. La función procesar_argumentos toma este puntero como argumento y asigna el valor apuntado
 * por numLineas a la variable global g_lineasProcesses.
 */
void procesar_argumentos(int argc, char *argv[], int *numTelefonos, int *numLineas)
{
    g_telefonosProcesses = *numTelefonos;
    g_lineasProcesses = *numLineas;
} // end procesar_argumentos
//-------------------------------------------------------------------------------------

// Instala un manejador de señales en el programa y muestra un mensaje de error si la operación falla.
void instalar_manejador_senhal()
{
    // utilizaos la llamada al sistema signal para asociar el metodo manejador_senhal a la señal SIGNIT
    // De esta manera podamos posteriormente realizar un apagado de emergencia con el comando Ctrl-C
    if (signal(SIGINT, manejador_senhal) == SIG_ERR)
    {
        fprintf(stderr, "[MANAGER] Error al instalar el manejador se senhal: %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    } // end if
} // end instalar_manejador_senhal
//---------------------------------------------------------------------------------------

// Apagado de Emergencia (Ctrl + C)
void manejador_senhal(int sign)
{
    printf("\n[MANAGER] Apagado de emergencia (Ctrl + C).\n");
    // al apagar todo tenemos que forzar la finalizacion de todos los procesos y luego liberar los recursos por ello llamamos a esos metodos
    terminar_procesos();
    liberar_recursos();
    exit(EXIT_SUCCESS);
} // end manejador_senhal
//-------------------------------------------------------------------------------------------

// inicializamos dos tablas de procesos una para las llamadas y otra para las lineas
void iniciar_tabla_procesos(int n_procesos_telefono, int n_procesos_linea)
{
    // Lo primero que hacemos es asignar memoria dinamica a esta tabla por medio de malloc
    // lo que le asignamos a cada tabla la calculamos por el Nº de procesos de lineas que tengamos por el tamaño a estructura "TProcess_t"
    g_process_telefonos_table = malloc(n_procesos_telefono * sizeof(struct TProcess_t));
    g_process_lineas_table = malloc(n_procesos_linea * sizeof(struct TProcess_t));

    // Ahora que ya hemos asigando la memoria a las tablas que de procesos que hemos creado, recorremos dichas tablas por un for
    // tabla lineas
    for (int i = 0; i < n_procesos_linea; i++)
    {
        // inicalizamos cada entrada a la tabla con el pid a cero ya que esta esta vacia de momento porque no hemos creado ningun proceso
        g_process_lineas_table[i].pid = 0;
    } // end for
    // tablas telefonos
    for (int i = 0; i < n_procesos_telefono; i++)
    {
        // inicalizamos cada entrada a la tabla con el pid a cero ya que esta esta vacia de momento porque no hemos creado ningun proceso
        g_process_telefonos_table[i].pid = 0;
    } // end for

} // end iniciar_tabla_procesos
//----------------------------------------------------------------------------------------------

// Este metodo se encarga de crear los procesos necesarios para las lineas y los telefonos
void crear_procesos(int numTelefonos, int numLineas)
{
    // para crear estos procesos lo que hacemos es crear un bucle for para asi llamar a la funcion lanzar proceso ya sea para linea o
    //  telefonos, la creacion de estos procesos dependera de el numero de lineas y telefonos que tengamos disponibles.
    printf("[MANAGER] %d telefonos creados.\n", numTelefonos);
    for (int i = 0; i < numTelefonos; i++)
    {
        lanzar_proceso_telefono(i);
    } // end for
    // realizamos una impresion con los telefonos totales que han sido creados
    printf("[MANAGER] %d lineas creadas.\n", numLineas);
    for (int i = 0; i < numLineas; i++)
    {
        lanzar_proceso_linea(i);
    } // end for
    // realizamos una impresion con las lineas totales que han sido creados
} // end crear_procesos
//-------------------------------------------------------------------------------------------------

void lanzar_proceso_telefono(const int indice_tabla)
{
    // creamos la varaible pid de tipo pid_t para poder alamcenar el PID correspondiente a cada proceso hijo
    pid_t pid;
    // Creamos un switch para controlar los difrenets casos cuando el fork nos de difernetes valores
    // declaramos que el pid sera igual al valor devuelto de la primitiva fork(), la cual es la que usamos para crear procesos hijos
    switch (pid = fork())
    {
    // en caso que sea -1 es que ha sucedido un error
    case -1:
        fprintf(stderr, "[MANAGER] Hemos encontrado un error al lanzar el proceso telefono: %s.\n", strerror(errno));
        // ya que hemos encontrado un error necesitamso termianr los procesos y liberar los recursos
        terminar_procesos();
        liberar_recursos();
        break;
    // Si devuelve 0, significa que se está ejecutando en el proceso hijo.
    case 0:
        // realizamos la primitiva execl para que el hijo ejecute otro proceso dirferente al del padre, en el caso de que de -1 se marca un eror
        // y nos imprime lo que ha sucedido.
        if (execl(RUTA_TELEFONO, CLASE_TELEFONO, NULL) == -1)
        {
            fprintf(stderr, "[MANAGER] Hemos encontrado un error usando execl () en el proceso %s: %s. \n", CLASE_TELEFONO, strerror(errno));
            exit(EXIT_FAILURE);
        } // end if
    }     // end switch-case
    // lo que hacemos posteriormente es actualizar la tabla de procesos con el PID del nuevo proceso lanzado y su clase en este caso clase telefono.
    g_process_telefonos_table[indice_tabla].pid = pid;
    g_process_telefonos_table[indice_tabla].clase = CLASE_TELEFONO;
} // end lanzar_proceso_telefono
//--------------------------------------------------------------------------------------------------

// hacemos exactamente lo mismo que lanzar_proceso_telefono solo que en este caso con las lineas
void lanzar_proceso_linea(const int indice_tabla)
{
    pid_t pid;

    switch (pid = fork())
    {
    case -1:
        fprintf(stderr, "[MANAGER] Hemos encontrado un error al lanzar proceso línea: %s.\n", strerror(errno));
        terminar_procesos();
        liberar_recursos();
        exit(EXIT_FAILURE);
    case 0:
        if (execl(RUTA_LINEA, CLASE_LINEA, NULL) == -1)
        {
            fprintf(stderr, "[MANAGER] Hemos encontrado un error usando execl() en el poceso %s: %s.\n", CLASE_LINEA, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    g_process_lineas_table[indice_tabla].pid = pid;
    g_process_lineas_table[indice_tabla].clase = CLASE_LINEA;
} // end lanzar_proceso_linea
//-----------------------------------------------------------------------------------------------------

// Este metodo se utiliza para esperar que los procesos creados por el programa terminen.
void esperar_procesos()
{
    // inicializmaos una variable llamada n_procesos la cual le dareos el valor de g_lineasProcesses, colocando el Nº de procesos existentes
    int n_procesos = g_lineasProcesses;
    pid_t pid;

    // creamos un bucle while el cual terminara hasta que todos los proceso hayan terminado
    while (n_procesos > 0)
    {
        // llamamos a la función "wait" para esperar a que un proceso hijo termine. El valor devuelto por wait es el PID del proceso hijo que acaba de terminar.
        pid = wait(NULL);
        // realizamos para que asi de esta manera encontrar el proceso hijo que acaba de terminar
        for (int i = 0; i <= g_lineasProcesses; i++)
        {
            // cuando enocntrempos dicho hijo que temrino, sera igual su PID con el del la tabla que hemos recorrido por lo notificaremos que ese proceso ya temrino.
            if (pid == g_process_lineas_table[i].pid)
            {
                printf("[MANAGER] Proceso linea terminado [%d]...\n", g_process_lineas_table[i].pid);
                // actualizamos la tabla de procesos estableciendo el valor del PID en 0  en esa posicion y se decrementamos el valor de n_processes
                g_process_lineas_table[i].pid = 0;
                n_procesos--;
                // rompemos el bucle for con el break
                break;
            } // end if
        }     // end for
    }         // end while
    // creamos otro bucle for para terminar cualquier proceso telefono que aun este en ejecucion.
    for (int i = 0; i < g_telefonosProcesses; i++)
    {
        // llamamos al metodo termianr procesos especificos el cual estara en cada elemento de la tabla de procesos "g_process_telefonos_table" y
        // establecemos el segundo parámetro como 0 para indicar que no vamos a imprimir un mensaje.
        terminar_procesos_especificos(&g_process_telefonos_table[i], 0);
    } /// End for
} // end esperar_procesos
//-------------------------------------------------------------------------------------------------------

// Este metodo se encarga de terminar los procesos enviadno una señal de SIGINT y asi terminarlo de manera ordenada
void terminar_procesos(void)
{
    // declaramos la variable que utilizaremos para los For's
    int i;
    printf("\n--- Finalizando procesos ---\n");
    // recorremos tosos los procesos lineas con el siguinte for
    for (i = 0; i < g_lineasProcesses; i++)
    {
        // si el pid que se encuentra en la posicion i es distinto de cero significa que ese codigo se esta ejecutando
        if (g_process_lineas_table[i].pid != 0)
        {
            // printf("Estoy aqui lines");
            // imprimimos que el manager esta terminando el proceso
            printf("[MANAGER] Terminando proceso %s [%d]...\n", g_process_lineas_table[i].clase, g_process_lineas_table[i].pid);
            // utlizamos la primitiva kill para ir matando a cada proceso, y si el el resultado de dicha primitiva sea -1 significa que hubo un error
            if (kill(g_process_lineas_table[i].pid, SIGINT) == -1)
            {
                // notificamos el error
                fprintf(stderr, "[MANAGER] Error al usar kill() en proceso %d: %s.\n", g_process_lineas_table[i].pid, strerror(errno));
            } // end if
        }     // end if
    }         // end for
    // realizamos exactamente lo mismo solamente que para los procesos telefonos
    for (i = 0; i < g_telefonosProcesses; i++)
    {
        if (g_process_telefonos_table[i].pid != 0)
        {
            printf("[MANAGER] Terminando proceso %s [%d]...\n", g_process_telefonos_table[i].clase, g_process_telefonos_table[i].pid);
            if (kill(g_process_telefonos_table[i].pid, SIGINT) == -1)
            {
                fprintf(stderr, "[MANAGER] Error al usar kill() en proceso %d: %s.\n", g_process_telefonos_table[i].pid, strerror(errno));
            } // end if
        }     // end if
    }         // end for
} // end terminar_procesos
//--------------------------------------------------------------------------------------------------------

// al igual lo que temrianr procesos este envia una señal de SIGNIT para finalizar un proceso solo que este caso sera un proceso en particular.
void terminar_procesos_especificos(struct TProcess_t *process_table, int process_num)
{
    printf("[MANAGER] Terminando proceso %s [%d]...\n", process_table[process_num].clase, process_table[process_num].pid);
    // Utilizamos la primitiva kill para matar al proceso especifico, ya que especificamos elindice de la tabla y el numero del proceso.
    // si en el caso de que kill sea -1, se amrca como un error y se notifica
    if (kill(process_table[process_num].pid, SIGINT) == -1)
    {
        fprintf(stderr, "[MANAGER] Hemos encontrado un error al usar kill() en proceso %d: %s.\n", process_table[process_num].pid, strerror(errno));
    } // end if
    else
    {
        process_table[process_num].pid = 0;
    }

} // end terminar_procesos_especificos
//--------------------------------------------------------------------------------------------------

// es imporatnte siempre liberar los recursos entonces este metodo se encarga de ello
void liberar_recursos()
{
    // utilizmaos la funcion free para liberar la memoria que antes hemos asignado con malloc
    free(g_process_telefonos_table);
    free(g_process_lineas_table);
    destruir_sem(TELEFONOS);
    destruir_sem(LINEAS);
    destruir_sem(MUTEXESPERA);
    destruir_var(LLAMADASESPERA);
} // end liberar_recursos
  //--------------------------------------------------------------------------------------------------