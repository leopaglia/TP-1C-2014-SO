// Bibliotecas //
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <commons/config.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>

#if 1 // CONSTANTES //
//Ruta del config
#define PATH_CONFIG "config.cfg"
#define NOMBRE_ARCHIVO_CONSOLA     "Archivo_msp.txt"
#define NOMBRE_ARCHIVO_LOG 		   "msp.log"

//Memoria
#define TAMANIO_PAGINA 256
#define CANTIDAD_MAX_SEGMENTOS 4096

#endif

#if 1 // VARIABLES GLOBALES //
// - Base de la memoria principal de la msp
char* g_BaseMemoria;

// Tamaño de la memoria
int g_CantidadMemoria;

// Tamaño de la memoria
int g_CantidadSwap;
long memSwap;

// - Puerto por el cual escucha el programa
int g_Puerto;

// - Algoritmo utilizado para asignacion de memoria
char * g_SustPags;

// Archivo donde descargar info impresa por consola
FILE * g_ArchivoConsola;

// Logger del commons
t_log* logger;

//Mensaje de error global.
char* g_MensajeError;

// Definimos los hilos principales
	pthread_t hOrquestadorConexiones, hConsola;

#endif

/** Longitud del buffer  */
#define BUFFERSIZE 40000

// Retardo (en milisegundos) para contestar una solicitud a un cliente
int g_Retardo = 0;

//Mensajes aceptados
#define MSJ_LEER_MEMORIA          1
#define MSJ_ESCRIBIR_MEMORIA      2
#define MSJ_HANDSHAKE             3
#define MSJ_CAMBIO_PROCESO        4
#define MSJ_CREAR_SEGMENTO        5
#define MSJ_DESTRUIR_SEGMENTO     6


// METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...);
void Traza(const char* mensaje, ...);
void SetearErrorGlobal(const char* mensaje, ...);
void ErrorFatal(const char* mensaje, ...);

// METODOS CONFIGURACION //
void LevantarConfig();

//METODOS CONSOLA //
void operaciones_consola();
int corte_msp();
void comenzar_msp();
void mostrarListaSegmentos();
void mostrarListaSegmentosDe(int pid);
void mostrarListaSegmentosYPaginas(int pid);
void mostrarListaPaginas(t_list * listaPaginas);

//METODOS MANEJO SOCKETS
void HiloOrquestadorDeConexiones();

//MANEJO DE MEMORIA
void reservarMemoriaPrincipal();
int memoriaDisponible(int tamanio);

//METODOS DE INTERFAZ
int crearSegmento(int pid, int tamanio);
int destruirSegmento(int pid, int base);
int escribirMemoria(int pid, int base, int pagina, int offset,int tamanio, char*texto);
int leerMemoria(int pid, int base, int pagina, int offset, int tamanio, char * buffer);

//ESTRUCTURA DE TABLA DE SEGMENTO Y METODOS PARA SU MANEJO
typedef struct {
    int pid;
    int tamanio;
    int nroSegmento;
    t_list *lista_paginas;
} t_segmento;

typedef struct {
	int nroPagina;
	int marco;
	int swapping;
} t_pagina;


typedef struct {
	int marco;
	int nroPagina;
	int pid;
	int base;
	bool bu;
	bool bm;
} t_algoritmoSust;

typedef struct {
	int nroMarco;
} t_marco;

static t_pagina *pagina_create(int nroPagina, int marco, int swapping){
	t_pagina *new = malloc(sizeof(t_pagina));
	new->nroPagina = nroPagina;
	new->marco = marco;
	new->swapping = swapping;
	return new;
}

static t_marco *marco_create(int nroMarco){
	t_marco *new = malloc(sizeof(t_marco));
	new->nroMarco = nroMarco;
	return new;
}

static t_list *lista_marco_create(int tamanio){
	t_list *lista = list_create();
	int i;
	int cantMarcos = tamanio/TAMANIO_PAGINA;
	if(tamanio%TAMANIO_PAGINA > 0){
		cantMarcos++;
	}
	for(i=0;i<cantMarcos;i++){
		list_add(lista,marco_create(i));
	}
	return lista;
}


static t_list *lista_pagina_create(int tamanio){
	t_list *lista = list_create();
	int i;
	int cantPaginas = tamanio/TAMANIO_PAGINA;
	if(tamanio%TAMANIO_PAGINA > 0){
		cantPaginas++;
	}
	for(i=0;i<cantPaginas;i++){
		list_add(lista,pagina_create(i,-1,0));
	}
	return lista;
}

static t_segmento *segmento_create(int pid,int tamanio,int nroSegmento) {
    t_segmento *new = malloc(sizeof(t_segmento));
    new->pid = pid;
    new->tamanio = tamanio;
    new->nroSegmento = nroSegmento;
    new->lista_paginas = lista_pagina_create(tamanio);
    return new;
}


static t_algoritmoSust *create_elemento(int marco, int nroPagina, int pid, int base){
	t_algoritmoSust *new = malloc(sizeof(t_algoritmoSust));
	new->marco = marco;
	new->nroPagina = nroPagina;
	new->pid = pid;
	new->base = base;
	new->bm=0;
	new->bu=1;
	return new;
}

static void elemAlgoritmo_destroy(t_algoritmoSust *self){
	free(self);
}


static void pagina_destroy(t_pagina *self){
	free(self);
}

static void segmento_destroy(t_segmento *self) {
    free(self);
}

t_list *listaSegmentos;
t_list *listaMarcosLibres;
t_list *listaAlgoritmo;

int agregarAListaAlgoritmo(int marco, int nroPagina, int pid, int base,int modo);
int numSegmento = 0;
int agregarSegmento(int pid, int tamanio);
t_segmento* buscarListaPagina(int pid,int segmento);
t_pagina *buscarPagina(t_segmento* segmento, int pagina);
int cantidadPaginas(t_pagina *pagina,t_list *lista_paginas);
char* traerSwapping(t_pagina *pagina, int pid, int base);
char* generarNombreArchivo(int pagina, int pid, int base);
int swappear(t_pagina* tpagina,int pid,int base);
int asignarMarcoLibre(t_pagina* tpagina,int pid,int base,int modo);
int obtenerMarcoLibre();
int grabarEnMemoria(int nroMarco,int posicion, int tamanio, char * texto);
void mostrarListaMarcosLibres(t_list * lista);
int leerEnMemoria(int marco,int posicion, int tamanio,char* buffer);
void liberarYEliminarMarcosOSwapping(t_segmento *segmento);
int eliminarArchivoSwapping(int pid, int segmento, int pagina);
void grabarArchivoSwap(int nroMarco, FILE* archivoSwap);
t_algoritmoSust* sustitucionPag(char* algoritmo);
int puntero = 0; //Puntero Clock Modificado
int obtenerPaginayOffset(int desplazamiento,int * pagina,int *offset);

//OPERACIONES PRINCIPALES
void crearListaSegmentos();
void crearListaMarcosLibres();
void crearListaAlgoritmo();

// - Puerto por el cual escucha el programa
int g_Puerto;

// - Bandera que controla la ejecución o no del programa. Si está en 0 el programa se cierra.
int g_Ejecutando = 1;

//Semaforos
sem_t semaforoAccesoMemoria;
sem_t semaforoMarcosLibres;
