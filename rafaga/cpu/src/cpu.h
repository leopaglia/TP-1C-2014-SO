/*
 * cpu.h
 *
 *  Created on: 03/10/2014
 *      Author: utnso
 */

#ifndef CPU2_H_
#define CPU2_H_

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
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
#include <commons/collections/list.h>
#include <commons/log.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

//#include <panel/cpu.h>

#include "instrucciones.h"

#define PATH_CONFIG "cpu_config.cfg"
#define PANEL_PATH "."

#define BUFFERSIZE 1200

#define INSTRUCCION 1;
#define PARAMETROS 2;

#define CANTIDADELEMENTOS(x) (sizeof(x) / sizeof(x[0]))

//	t_list *lista_parametros;
	t_list *lista_parametros;

	char* puerto_kernel;
	char* ip_kernel;
	char* puerto_msp;
	char* ip_msp;
	int retardo;
	int quantum;

	int	cantidadDeParametros = -1;
	int parametros[4] = {-1,-1,-1,-1};

	char* parametroString;
	char* instruccion;

	int msp_socket;
	int kernel_socket;

//Arrays de parametros
	//numero = 4bytes
	//registro = 1byte

	//numero, registro, registro
	const char* NRR[] = {"SETM"};

	//registro, numero
	const char* RN[] = {"LOAD"};

	//registro, registro
	const char* RR[] = {"GETM", "MOVR", "ADDR", "SUBR",
					    "MULR", "MODR", "DIVR", "COMP",
					    "CGEQ", "CLEQ"};

	//numero, registro
	const char* NR[] = {"SHIF", "PUSH", "TAKE"};

	//registro
	const char* R[] = {"INCR", "DECR", "GOTO"};

	//numero
	const char* N[] = {"JMPZ", "JPNZ", "INTE"};

	//nada
	const char* NADA[] = {"NOPP", "XXXX", "MALC", "FREE",
						  "INNN", "INNC", "OUTN", "OUTC",
						  "CREA", "JOIN", "BLOK", "WAKE"};

int calcularCantDigitos(int);
void escribirMemoria(int, int, int, int, char*);
void ejecutarTarea();
char* traducirRegistro(char*);
void test();
void LeerConfigCPU();
int conexiones();
int enviarDatos(int, void*);
int recibirDatos(int, char*);
char* leerMemoria(int, t_registros_cpu*);



#endif /* CPU_H_ */
