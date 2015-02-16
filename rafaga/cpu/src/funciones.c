/*
 * funciones.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include "funciones.h"

int calculameCantDigitos(int num) {
	//patch v2.32, ahora soporta numeros negativos! llame ya
	int contador = 1;

	if(num < 0){
		contador++;
		num = num * -1;
	}

	while (num / 10 > 0) {
		num = num / 10;
		contador++;
	}

	return contador;
}

char* handshakeInterrupcion(int interrupcion){

	char* respuesta;

	switch(interrupcion){
		case 0: respuesta = "I"; break;
		case 28: respuesta = "I"; break;
		case 48: respuesta = "E"; break;
		case 64: respuesta = "E"; break;
		case 76: respuesta = "S"; break;
		case 108: respuesta = "S"; break;
		case 132: respuesta = "H"; break;
		case 164: respuesta = "J"; break;
	};

	return respuesta;
};

void load(t_registros_cpu* registros, int parametros[]){

	//perdon
	if(parametros[1] == 11111111){
		registros->registros_programacion[parametros[0]] = registros->M;
		return;
	}
	if(parametros[1] == 22222222){
		registros->registros_programacion[parametros[0]] = registros->X;
		return;
	}

	registros->registros_programacion[parametros[0]] = parametros[1];

	return;
};

void getm(t_registros_cpu* registros, int parametros[]){

	t_registros_cpu* aux = malloc(sizeof(t_registros_cpu));
	aux->I = registros->I;
	aux->M = registros->M;
	aux->P = registros->registros_programacion[parametros[1]];
	aux->K = false;

	char* respuestaDeMemoria = leerMemoria(1, aux);

	if(strcmp(respuestaDeMemoria, "-1") == 0){
		flagSegFault = 1;
		free(aux);
		return;
	}

	//TODO: en bigstack, levanta "HHola"
	char letra = respuestaDeMemoria[0];

	registros->registros_programacion[parametros[0]] = letra;


	free(aux);

	return;
};

void setm(t_registros_cpu* registros, int parametros[]){

	char* datos = string_new();
	int cuantosBytes = parametros[0];
	int donde;

	if(parametros[1] == 33333333){
		donde = registros->S;
	}else{
		donde = registros->registros_programacion[parametros[1]];
	}

	int data = registros->registros_programacion[parametros[2]];

	int i;
	int byte;

	for(i = 0; i < cuantosBytes; i++){
		byte = ((data >> (8*i)) & 0xFF); // dicen que esta mugre lee byte por byte
		string_append(&datos,string_itoa(byte));
	};

	escribirMemoria(registros->I, registros->M, cuantosBytes, donde, datos);

	return;
};

void movr(t_registros_cpu* registros, int parametros[]){

	registros->registros_programacion[parametros[0]] = registros->registros_programacion[parametros[1]];

	return;
};

void addr(t_registros_cpu* registros, int parametros[]){


	//perdon
	if(parametros[0] == 11111111){
		registros->registros_programacion[0] = registros->registros_programacion[parametros[1]] + registros->M;
		return;
	}
	if(parametros[0] == 22222222){
		registros->registros_programacion[0] = registros->registros_programacion[parametros[1]] + registros->X;
		return;
	}

	//perdonx2
	if(parametros[1] == 11111111){
		registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] + registros->M;
		return;
	}
	if(parametros[1] == 22222222){
		registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] + registros->X;
		return;
	}

	registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] + registros->registros_programacion[parametros[1]];

	return;
};

void subr(t_registros_cpu* registros, int parametros[]){

	registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] - registros->registros_programacion[parametros[1]];

	return;
};

void mulr(t_registros_cpu* registros, int parametros[]){

	registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] * registros->registros_programacion[parametros[1]];

	return;
};

void modr(t_registros_cpu* registros, int parametros[]){

	registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] % registros->registros_programacion[parametros[1]];

	return;
};

void divr(t_registros_cpu* registros, int parametros[]){

	if (registros->registros_programacion[parametros[1]] == 0){
		return; //TODO : Exception div 0
	}else{
		registros->registros_programacion[0] = registros->registros_programacion[parametros[0]] / registros->registros_programacion[parametros[1]];
	}

	return;
};

void incr(t_registros_cpu* registros, int parametros[]){

	if(parametros[0] == 33333333){
		registros->S ++;
		return;
	}

	registros->registros_programacion[parametros[0]]++;

	return;
};

void decr(t_registros_cpu* registros, int parametros[]){

	registros->registros_programacion[parametros[0]]--;

	return;
};

void comp(t_registros_cpu* registros, int parametros[]){

	int cmp1;
	int cmp2;

		switch (parametros[0]){
		case 22222222:
			cmp1 = registros->X;
			break;
		case 33333333:
			cmp1 = registros->S;
			break;
		default:
			cmp1 = registros->registros_programacion[parametros[0]];
			break;
		};

		switch (parametros[1]){
		case 22222222:
			cmp2 = registros->X;
			break;
		case 33333333:
			cmp2 = registros->S;
			break;
		default:
			cmp2 = registros->registros_programacion[parametros[1]];
			break;
		};


	if (cmp1 == cmp2){
		registros->registros_programacion[0] = 1;
	}
	else{
		registros->registros_programacion[0] = 0;
	}

	return;
};

void cgeq(t_registros_cpu* registros, int parametros[]){

	if (registros->registros_programacion[parametros[0]] >= registros->registros_programacion[parametros[1]]){
		registros->registros_programacion[0] = 1;
	}
	else{
		registros->registros_programacion[0] = 0;
	}

	return;
};

void cleq(t_registros_cpu* registros, int parametros[]){

	if (registros->registros_programacion[parametros[0]] <= registros->registros_programacion[parametros[1]]){
		registros->registros_programacion[0] = 1;
	}
	else{
		registros->registros_programacion[0] = 0;
	}

	return;
};

void g0t0(t_registros_cpu* registros, int parametros[]){ //ya existe goto

	registros->P = registros->registros_programacion[parametros[0]];

	return;
};

void jmpz(t_registros_cpu* registros, int parametros[]){

	if (registros->registros_programacion[0] == 0){
		registros->P = parametros[0];
	}

	return;
};

void jpnz(t_registros_cpu* registros, int parametros[]){

	if (registros->registros_programacion[0] != 0){
		registros->P = parametros[0];
	}

	return;
};

void nopp(t_registros_cpu* registros, int parametros[]){
	return;
};

void shif(t_registros_cpu* registros, int parametros[]){

	int cantidad = parametros[0];

	if (cantidad > 0){
		registros->registros_programacion[parametros[1]] = registros->registros_programacion[parametros[1]] >> cantidad;
	}

	if (cantidad  < 0){

		cantidad = cantidad * -1; //magic

		registros->registros_programacion[parametros[1]] = registros->registros_programacion[parametros[1]] << cantidad;
	}

	return;
};

void inte(t_registros_cpu* registros, int parametros[]){

	kernel_socket = parametros[2];

	t_hilo* tcbAux = malloc(sizeof(t_hilo));
	char* buffer = string_new();
	char* msj = string_new();

	tcbAux->pid = registros->I;
	tcbAux->tid = parametros[1];
	tcbAux->kernel_mode = registros->K;
	tcbAux->base_stack = registros->X;
	tcbAux->cursor_stack = registros->S;
	tcbAux->puntero_instruccion = registros->P;
	tcbAux->segmento_codigo = registros->M;
	tcbAux->segmento_codigo_size = parametros[3];
	tcbAux->registros[0] = registros->registros_programacion[0];
	tcbAux->registros[1] = registros->registros_programacion[1];
	tcbAux->registros[2] = registros->registros_programacion[2];
	tcbAux->registros[3] = registros->registros_programacion[3];
	tcbAux->registros[4] = registros->registros_programacion[4];

	string_append(&buffer, tcbAString(tcbAux));

//	strcpy(msj, handshakeInterrupcion(parametros[0]));
	string_append(&msj, "I");

	string_append(&msj, buffer);
	string_append(&msj, string_itoa(parametros[0]));
	string_append(&msj, "\0");

	enviarDatos(kernel_socket, msj);

	free(tcbAux);

	flagProcessSwitch = 1; //para que corte ejecucion y vuelva a estar libre

	return;
};

void push(t_registros_cpu* registros, int parametros[]){

	char* datos = string_new();
	int aux = registros->registros_programacion[parametros[1]];
	int cant_digitos_aux = calculameCantDigitos(aux);
	int byte;
	int i;

	for(i = 0; i < parametros[0]; i++){
		byte = ((aux >> (8*i)) & 0xFF); // en teoria lee byte por byte
		string_append(&datos,string_itoa(byte));
	};

	//TODO: algo esta feo aca, tiene que grabar en binario?
	escribirMemoria(registros->I, registros->X, cant_digitos_aux, registros->S, datos);
	registros->S = registros->S + cant_digitos_aux;


	return;
};

void take(t_registros_cpu* registros, int parametros[]){

	t_registros_cpu* aux = malloc(sizeof(t_registros_cpu));
	aux->I = registros->I;
	aux->M = registros->X;
	aux->P = registros->S-1;

	//creo que esta leyendo del codigo, arreglar
	registros->registros_programacion[parametros[1]] = atoi(leerMemoria(1, aux));

//	registros->S = registros->S - parametros[0];
	registros->S = registros->S - 1;

	free(aux);

	return;
};

void xxxx(t_registros_cpu* registros, int parametros[]){
	flagTerminoPrograma = 1;
	return ;
};


//Instrucciones protegidas


void malc(t_registros_cpu* registros, int parametros[]){

	if(listaMalloc == NULL){
		listaMalloc = list_create();
	}

	int a = registros->registros_programacion[0];
	registros->registros_programacion[0] = pedirSegmentoAMemoria(registros->I, a);

	structMalloc* nuevoSegmento = malloc(sizeof(structMalloc));
	nuevoSegmento->pid = registros->I;
	nuevoSegmento->direccion = registros->registros_programacion[0];

	list_add(listaMalloc, nuevoSegmento);

	free(nuevoSegmento);

	return;
};

structMalloc* list_find_jjjlsCustom(t_list* lista, structMalloc* data){

	t_link_element* nodo;

	nodo = lista->head;

	while(nodo){

		if(data == (structMalloc*)nodo->data) return nodo->data;
		nodo = nodo->next;

	};

	return NULL;
};


void FREE(t_registros_cpu* registros, int parametros[]){

	structMalloc* elementoABuscar = malloc(sizeof(structMalloc));
	elementoABuscar->pid = registros->I;
	elementoABuscar->direccion = registros->registros_programacion[0];

	structMalloc* elementoEncontrado = malloc(sizeof(structMalloc));;

	elementoEncontrado = list_find_jjjlsCustom(listaMalloc, elementoABuscar);

	if(elementoEncontrado != NULL){
		destruirSegmentoMemoria(elementoEncontrado->pid, elementoEncontrado->direccion);
	};

	free(elementoABuscar);
	free(elementoEncontrado);

	return;
};

void innn(t_registros_cpu* registros, int parametros[]){

	char buffer[BUFFERSIZE];

	char* mensaje = string_new();

	string_append(&mensaje, "EN");
	string_append(&mensaje, string_itoa(registros->I));

	enviarDatos(kernel_socket, mensaje);

	recibirDatos(kernel_socket, buffer);
	registros->registros_programacion[0] = atoi(buffer);

	return;
};

void innc(t_registros_cpu* registros, int parametros[]){

	char buffer[BUFFERSIZE];
	int size = registros->registros_programacion[1] * sizeof(char);
	int a = registros->registros_programacion[0];

	char* mensaje = string_new();

	string_append(&mensaje, "EC");
	string_append(&mensaje, string_itoa(registros->I));

	enviarDatos(kernel_socket, mensaje);
	recibirDatos(kernel_socket, buffer);
	escribirMemoria(registros->I, registros->M, size, a, buffer);

	return;
};

void outn(t_registros_cpu* registros, int parametros[]){

	char* pid = string_itoa(registros->I);
	char* pidSize = string_itoa(string_length(pid));
	char* buffer = string_new();
	char buffer2 [BUFFERSIZE];
	char* numero = string_itoa(registros->registros_programacion[0]);

	string_append(&buffer, "SN");
	string_append(&buffer, pidSize);
	string_append(&buffer, pid);
	string_append(&buffer, numero);

	enviarDatos(kernel_socket, buffer);
	recibirDatos(kernel_socket, buffer2);

	if(strcmp(buffer2,"1") != 0){
		printf("El kernel no mandó el ok para seguir (salidaEstandar)");
		exit(1);
	}

	return;
};

void outc(t_registros_cpu* registros, int parametros[]){

	char* pid = string_itoa(registros->I);
	char* pidSize = string_itoa(string_length(pid));
	char* buffer = string_new();
	char buffer2 [BUFFERSIZE];
	char* mensaje = string_new();

	t_registros_cpu* aux = malloc(sizeof(t_registros_cpu));
	aux->I = registros->I;
	aux->M = registros->M;
	aux->P = registros->registros_programacion[0];
	aux->K = !registros->K;

	mensaje = leerMemoria(registros->registros_programacion[1], aux);

	string_append(&buffer, "SC");
	string_append(&buffer, pidSize);
	string_append(&buffer, pid);
	string_append(&buffer, mensaje);

	enviarDatos(kernel_socket, buffer);
	recibirDatos(kernel_socket, buffer2);

	if(strcmp(buffer2,"1") != 0){
		printf("El kernel no mandó el ok para seguir (salidaEstandar)");
		exit(1);
	}

	free(aux);

	return;
};

void crea(t_registros_cpu* registros, int parametros[]){

	char* msj = string_new();
	char* buffer = string_new();
	int socket_kernel = parametros[2];

	t_hilo* hijo = malloc(sizeof(t_hilo));

	hijo->pid = registros->I;
	hijo->puntero_instruccion = registros->registros_programacion[1];
	hijo->kernel_mode = 0;

	//datos que faltan, los carga el kernel
	hijo->tid = 0;
	hijo->segmento_codigo_size = 0;
	hijo->segmento_codigo = 0;
	hijo->base_stack = 0;
	hijo->cursor_stack = 0;
	hijo->registros[0] = 0;
	hijo->registros[1] = 0;
	hijo->registros[2] = 0;
	hijo->registros[3] = 0;
	hijo->registros[4] = 0;
//
	string_append(&buffer, tcbAString(hijo));
	string_append(&msj, "H");
	string_append(&msj, buffer);

	enviarDatos(socket_kernel, msj);

	char buffer2[BUFFERSIZE];
	recibirDatos(socket_kernel, buffer2);

	registros->registros_programacion[0] = atoi(buffer2);

	free(hijo);

	return;
};

void join(t_registros_cpu* registros, int parametros[]){

	char* buffer = string_new();
	char* tidA = string_itoa(registros->registros_programacion[0]);
	char* tidL = string_itoa(parametros[1]);
	char* sizeTidL = string_itoa(string_length(tidL));
	char* sizeTidA = string_itoa(string_length(tidA));

	string_append(&buffer, sizeTidL);
	string_append(&buffer, tidL);
	string_append(&buffer, sizeTidA);
	string_append(&buffer, tidA);

	int socket_kernel = parametros[2];
	enviarDatos(socket_kernel, buffer);

	return;
};

void blok(t_registros_cpu* registros, int parametros[]){

	char* buffer = string_new();
	char* id_recurso = string_itoa(registros->registros_programacion[1]);
	char* tid = string_itoa(parametros[1]);

	string_append(&buffer, id_recurso);
	string_append(&buffer, tid);

	int socket_kernel = parametros[2];
	enviarDatos(socket_kernel, buffer);

	return;
};

void wake(t_registros_cpu* registros, int parametros[]){

	char* buffer = string_itoa(registros->registros_programacion[1]);

	int socket_kernel = parametros[2];
	enviarDatos(socket_kernel, buffer);
};

