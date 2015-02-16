/*
 * funciones.h
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#ifndef FUNCIONES_H_
#define FUNCIONES_H_

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

#include "cpupanel.h"

#define BUFFERSIZE 1200

int flagTerminoPrograma;
int flagProcessSwitch;
int flagSegFault;

int kernel_socket;


//int flagLista = 0;
t_list *listaMalloc;

typedef struct{
	int pid;
	int direccion;
}structMalloc;

void ejecutarTarea();
char* traducirRegistro(char*);
void test();
void LeerConfigCPU();
int conexiones();
int enviarDatos(int, void*);
int recibirDatos(int, char*);
char* leerMemoria(int, t_registros_cpu*);
int pedirSegmentoAMemoria(uint32_t, int);
void escribirMemoria(int, int, int, int, char*);
char* tcbAString(t_hilo*);
void destruirSegmentoMemoria(int, int);

void load(t_registros_cpu*, int[]);

void getm(t_registros_cpu*, int[]);

void setm(t_registros_cpu*, int[]);

void movr(t_registros_cpu*, int[]);

void addr(t_registros_cpu*, int[]);

void subr(t_registros_cpu*, int[]);

void mulr(t_registros_cpu*, int[]);

void modr(t_registros_cpu*, int[]);

void divr(t_registros_cpu*, int[]);

void incr(t_registros_cpu*, int[]);

void decr(t_registros_cpu*, int[]);

void comp(t_registros_cpu*, int[]);

void cgeq(t_registros_cpu*, int[]);

void cleq(t_registros_cpu*, int[]);

void g0t0(t_registros_cpu*, int[]);//ya existe goto

void jmpz(t_registros_cpu*, int[]);

void jpnz(t_registros_cpu*, int[]);

void inte(t_registros_cpu*, int[]);

void shif(t_registros_cpu*, int[]);

void nopp(t_registros_cpu*, int[]);

void push(t_registros_cpu*, int[]);

void take(t_registros_cpu*, int[]);

void xxxx(t_registros_cpu*, int[]);



void malc(t_registros_cpu*, int[]);

void FREE(t_registros_cpu*, int[]); //ya existe free

void innn(t_registros_cpu*, int[]);

void innc(t_registros_cpu*, int[]);

void outn(t_registros_cpu*, int[]);

void outc(t_registros_cpu*, int[]);

void crea(t_registros_cpu*, int[]);

void join(t_registros_cpu*, int[]);

void blok(t_registros_cpu*, int[]);

void wake(t_registros_cpu*, int[]);


#endif /* FUNCIONES_H_ */
