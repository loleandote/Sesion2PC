// Semáforos
#define MUTEXESPERA "mutexesp"
#define TELEFONOS "contadortel"
#define LINEAS "contadorlin"

//Memoria Compartida
#define LLAMADASESPERA "llamadasespera"

// CLASES y PATHS
#define CLASE_TELEFONO "TELEFONO"
#define RUTA_TELEFONO "./exec/telefono"
#define CLASE_LINEA "LINEA"
#define RUTA_LINEA "./exec/linea"

struct TProcess_t
{
  pid_t pid;
  char *clase;
};
