/*
 * kernel.h
 *
 *  Created on: 17/09/2014
 *      Author: JnsCas
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <resolv.h>
#include <fcntl.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <semaphore.h>
//#include <panel/panel.h>
//#include <panel/kernel.h>

#if 1

#define PATH_CONFIG "kernel_config.cfg"
#define SOY_CONSOLA 'C'
#define SOY_CPU 'U'
#define SOY_CPU_INTE 'I'
#define SOY_CPU_ENTRADA 'E'
#define SOY_CPU_SALIDA 'S'
#define SOY_CPU_CREAR 'H'
#define SOY_CPU_JOIN 'J'
#define SOY_CPU_BLOCK 'B'
#define SOY_CPU_DESPERTAR 'D'
#define RECIBO_PROGRAMA 'P'
#define MSJ_NEGACION "N"
#define MSJ_CONFIRMACION "O"
#define MSJ_HANDSHAKE_DESCONOCIDO "D"
#define BUFFERSIZE 1200
#define MSJ_CREAR_SEGMENTO 5
#define MSJ_ESCRIBIR_MEMORIA 2
#define MSJ_DESTRUIR_SEGMENTO 6

#endif

#if 1

int PUERTO_KERNEL;
char* PUERTO_MSP;
char* IP_MSP;
char* SYSCALLS;
int QUANTUM;
int STACK;

int flag = 1; //si existe un tcb kernel

//para saber si termino o no la ejecucion del tcb a esperar en la sys join
sem_t sem_join;
//tid q se esta esperando que termine
int tid_esperado_join = 0;

uint32_t pid = 1;
uint32_t tid = 2;

t_list *listaNew,*listaReady,*listaExec,*listaBlock,*listaExit;
t_list *listaTcb;
t_list *listaCpuLibre;
t_list *listaLlamadas;
t_list *listaBlockXRecursos;

sem_t semaforoTCB;
sem_t semaforoCPU;

pthread_mutex_t mutexEnviarDatos;
pthread_mutex_t mutexRecibirDatos;

typedef enum { NEW, READY, EXEC, BLOCK, EXIT } t_cola;

typedef struct {
	uint32_t pid;
	uint32_t tid;
	bool kernel_mode;
	uint32_t segmento_codigo;
	uint32_t segmento_codigo_size;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registros[5];
	t_cola cola;
} t_hilo;

typedef struct{
	int libre;
	int socket_cpu;
}structParametros;

typedef struct{
	int pid;
	int tid;
	int socket_consola;
}structTcb;

typedef struct{
	t_hilo* hilo;
	int dir_memoria;
}structLlamadas;

typedef struct{
	int id_recurso;
	t_hilo* tcb;
}structRecursos;

t_log* loggerKernel;

pthread_t hRecibirConexiones, hAtenderCliente;

#endif


void leerConfigKernel();
void hiloRecibirConexiones();
void hiloAtenderCliente(void *);
void loader(int, int, char*);
int escribirMemoria(int,int,int,char*);
void confirmarConexionOK(int );
void planificador();
int conectarMSP();
int enviarDatos(int, void*);
int enviarDatosConTamanio(int, void*, int);
int recibirDatos(int, char*);
int cantidadDigitos(int);
int calcularCantDigitos(int);
int pedirSegmentoAMemoria(uint32_t,int);
void crearMultiplexor();
void atenderCPU(int, char*);
void entradaEstandar(int, char*);
void salidaEstandar(int, char*);
char* crearHilo(int, char*);
char* join(int, char*);
void bloquear(int, char*);
void despertar(int, char*);
t_hilo* stringATcb(char*);
char* tcbAString(t_hilo*);
char* tcbAStringSinQuantum(t_hilo*);
void eliminarTCB_Lista(t_list*,int);
int eliminarCPU_Lista(t_list*,int);
void interrupcion(int, char*);
void ejecutarLlamadaAlSist();
void* buscarTCB_Lista(t_list*, t_hilo*);
int buscarSocketConsolaPorTid(int);
int buscarSocketConsolaPorPid(int);
void eliminarTCB_ListaPorTid(t_list*, int);
void finalizarLlamadaAlSist(t_hilo*);
structRecursos* buscarRecurso(int);



