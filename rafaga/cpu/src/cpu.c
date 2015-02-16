/*
 * cpu.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include "cpu.h"
#include "funciones.h"


int calcularCantDigitos(int num) {
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

char* RecibirDatos(int socket, char *buffer, int *bytesRecibidos) {
	*bytesRecibidos = 0;
	if (buffer != NULL ) {
		free(buffer);
	}

	char* bufferAux = malloc(BUFFERSIZE * sizeof(char));
	memset(bufferAux, 0, BUFFERSIZE * sizeof(char)); //-> llenamos el bufferAux con barras ceros.

	if ((*bytesRecibidos = *bytesRecibidos
			+ recv(socket, bufferAux, BUFFERSIZE, 0)) == -1) {
		//Error(
			//	"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",
				//socket);
	}

//	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s", socket,
			//(char*) bufferAux);
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}

//#include <panel/cpu.h>
	//variables globales

	t_hilo* ptr_hilo;
	t_registros_cpu* ptr_registros;
//	t_hilo* hiloAux;





//Cambia la letra que representa al registro por su indice en el array de punteros a registros
//El atoi lo convierte en int
char* traducirRegistro(char* registro){
	char* num = malloc(9*sizeof(char));

	char reg = (char)registro[0];

	switch (reg){
		case 'A':{
			*num = '0';
			break;
		}
		case 'B':{
			*num = '1';
			break;
		}
		case 'C':{
			*num = '2';
			break;
		}
		case 'D':{
			*num = '3';
			break;
		}
		case 'E':{
			*num = '4';
			break;
		}

		//0.0000003% chances de que esto falle
		case 'M':{
			num = "11111111";
			break;
		}
		case 'X':{
			num = "22222222";
			break;
		}
		case 'S':{
			num = "33333333";
			break;
		}
		default:{
			num = registro; //devuelve lo que venia
		}
	}

	return num;
};

int enviarDatos(int socket, void *buffer){
	int bytecount;

		if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1){
			printf("ERROR: no se pudo enviar información. \n");
		}
		return (bytecount);
}



int recibirDatos(int socket,char *buffer) {
	int bytecount;
	// memset se usa para llenar el buffer con 0s
	memset(buffer, 0, BUFFERSIZE);
	bytecount = recv(socket, buffer, BUFFERSIZE, 0);
	return (bytecount);
}


void LeerConfigCPU() {
	t_config* config = config_create(PATH_CONFIG);

	if(config->properties->table_current_size !=0){
		
		if(config_has_property(config, "IP_KERNEL")){
			ip_kernel = config_get_string_value(config, "IP_KERNEL");
		} else printf("ERROR: No se pudo leer el parametro IP_KERNEL \n");

		if(config_has_property(config, "PUERTO_KERNEL")){
			puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
		} else printf("ERROR: No se pudo leer el parametro PUERTO_KERNEL \n");
		
		if(config_has_property(config, "IP_MSP")){
			ip_msp = config_get_string_value(config, "IP_MSP");
		} else printf("ERROR: No se pudo leer el parametro IP_MSP \n");

		if(config_has_property(config, "PUERTO_MSP")){
			puerto_msp = config_get_string_value(config, "PUERTO_MSP");
		} else printf("ERROR: No se pudo leer el parametro PUERTO_MSP \n");
		
		if(config_has_property(config, "RETARDO")){
			retardo = config_get_int_value(config, "RETARDO");
		} else printf("ERROR: No se pudo leer el parametro RETARDO \n");

		
	} else {

		printf("FATAL ERROR: No se pudo abrir el archivo de configuracion \n");
		
		exit(1);
	}

	if(config != NULL){
		free(config);
	}
}


int conexionKernel(){
	
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if(getaddrinfo(ip_kernel, puerto_kernel, &hints, &serverInfo) != 0 ){	// Carga en serverInfo los datos de la conexion
		printf("ERROR: cargando datos de conexion socket kernel \n");
		exit(1);
	}

	if ((kernel_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0){
		printf("ERROR: crear socket_kernel\n");
		exit(1);
	}

	if(connect(kernel_socket, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0 ){
		printf("ERROR: conectar socket_kernel\n");
		exit(1);
	}
	
	freeaddrinfo(serverInfo);

		
	return EXIT_SUCCESS;
};

int conexionMSP(){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// India que usaremos el protocolo TCP

	if(getaddrinfo(ip_msp, puerto_msp, &hints, &serverInfo) != 0 ){	// Carga en serverInfo los datos de la conexion
		printf("ERROR: cargando datos de conexion socket msp \n");
		exit(1);
	}

	if ((msp_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0){
		printf("ERROR: crear socket_msp\n");
		exit(1);
	}

	if(connect(msp_socket, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0 ){
		printf("ERROR: conectar socket_msp\n");
		exit(1);
	}

	freeaddrinfo(serverInfo);

	return EXIT_SUCCESS;

}

int charToInt(char x)
{
	int numero = 0;
	char * aux = string_new();
	string_append_with_format(&aux, "%c", x);
	//char* aux = malloc(1 * sizeof(char));
	//sprintf(aux, "%c", x);
	numero = strtol(aux, (char **) NULL, 10);

	if (aux != NULL )
		free(aux);
	return numero;
}

void escribirMemoria(int pid, int base_segmento, int prog_size, int offset, char* datos ){
	// Graba en la memoria
	// Formato del mensaje: CABBBBCDDDDHIIIJOOOOOOOOO....
	// C = Codigo de mensaje ( = 2)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el segmento
	// DDDD = El numero del segmento (hasta 4096)
	// H = Cantidad de digitos que tiene el desplazamiento
	// I = Desplazamiento (hasta 256)
	// J = Cantidad de digitos que tiene el tamaño del programa
	// OOOOOOOO = El tamaño del programa(hasta 9999999)

	int cant_digitos_pid;
	int cant_digitos_base_segmento;
	int cant_digitos_prog_size;
	int cant_digitos_offset;
	char* buffer = string_new();
	char buffer2 [BUFFERSIZE];

	cant_digitos_pid = calcularCantDigitos(pid);
	cant_digitos_base_segmento = calcularCantDigitos(base_segmento);
	cant_digitos_prog_size = calcularCantDigitos(prog_size);
	cant_digitos_offset = calcularCantDigitos(offset);

	string_append(&buffer,string_itoa(2));
	string_append(&buffer,string_itoa(cant_digitos_pid));
	string_append(&buffer,string_itoa(pid));
	string_append(&buffer,string_itoa(cant_digitos_base_segmento));
	string_append(&buffer,string_itoa(base_segmento));
	string_append(&buffer,string_itoa(cant_digitos_offset));
	string_append(&buffer,string_itoa(offset));
	string_append(&buffer,string_itoa(cant_digitos_prog_size));
	string_append(&buffer,string_itoa(prog_size));

	//metadata
	if(enviarDatos(msp_socket, buffer) == -1){
		printf("Error en enviar datos al escribir memoria");
		exit(1);
	};
	//data
	if(enviarDatos(msp_socket, datos) == -1){
		printf("Error en enviar datos al escribir memoria");
		exit(1);
	};


	recibirDatos(msp_socket, buffer2);

}


char* leerMemoria(int cuantosBytesQuieroLeer, t_registros_cpu* ptr_registros){ //que nombre de mierda

	//esta funcion arma el string concatenado para la msp, se lo manda y devuelve la respuesta

	char* respuesta = string_new();
	char* mensaje = string_new();

	//Leer memoria
	char* codMensaje = malloc(sizeof(char));
	codMensaje = "1";
	string_append(&mensaje, codMensaje);

//	free(codMensaje);

	if(ptr_registros->K){
		string_append(&mensaje, "1");
		string_append(&mensaje, "1");
	}
	else{
		string_append(&mensaje, string_itoa(calcularCantDigitos(ptr_registros->I)));
		string_append(&mensaje, string_itoa(ptr_registros->I));
	}

	string_append(&mensaje, string_itoa(calcularCantDigitos(ptr_registros->M)));
	string_append(&mensaje, string_itoa(ptr_registros->M));
	string_append(&mensaje, string_itoa(calcularCantDigitos(ptr_registros->P)));
	string_append(&mensaje, string_itoa(ptr_registros->P));
	string_append(&mensaje, string_itoa(calcularCantDigitos(cuantosBytesQuieroLeer)));
	string_append(&mensaje, string_itoa(cuantosBytesQuieroLeer));

	send(msp_socket, mensaje, strlen(mensaje), 0);
	respuesta = RecibirDatos(msp_socket, respuesta,&cuantosBytesQuieroLeer);

	return respuesta;
};

int stringInArray(char* string, const char* array[], int cantidadElementos){
	//devuelve 1 si el string esta en el array
	//0 si no esta
	//no usar en otro lado :D

	int i;
	int flag = 0;

	for (i = 0; i < cantidadElementos; i++){
		if (strcmp(string, array[i]) == 0 ){
			flag = 1;
			break;
		}
	};

	return flag;
};

int estructuraParametros(char* instruccion){

//	printf("\naca esta:%s\n", instruccion);
//	leerBinario(instruccion);
	if(stringInArray(instruccion, NRR, CANTIDADELEMENTOS(NRR))) return 1;
	if(stringInArray(instruccion, RN, CANTIDADELEMENTOS(RN))) return 2;
	if(stringInArray(instruccion, RR, CANTIDADELEMENTOS(RR))) return 3;
	if(stringInArray(instruccion, NR, CANTIDADELEMENTOS(NR))) return 4;
	if(stringInArray(instruccion, R, CANTIDADELEMENTOS(R))) return 5;
	if(stringInArray(instruccion, N, CANTIDADELEMENTOS(N))) return 6;
	if(stringInArray(instruccion, NADA, CANTIDADELEMENTOS(NADA))) return 7;

	return -1;
};

void leerNumero(t_list *lista_parametros, int* parametros, int posicionEnArray){

	parametroString = leerMemoria(4, ptr_registros);

	int *t;
	char numero[4];
	numero[0] = parametroString[0];
	numero[1] = parametroString[1];
	numero[2] = parametroString[2];
	numero[3] = parametroString[3];
	t = numero;
//		printf("\nEL GRAN NUMERO:%d\n",*t);

	list_add(lista_parametros, string_substring(parametroString, 0, 1));
	parametros[posicionEnArray] = *t;

};

void leerRegistro(t_list *lista_parametros, int* parametros, int posicionEnArray){
	char* parametroString = string_new();
	parametroString = leerMemoria(1, ptr_registros);
	char* reg = traducirRegistro(parametroString);

	list_add(lista_parametros, string_substring(parametroString, 0, 1));
	int traducido = atoi(reg);

	if(traducido == 22) traducido = 2; //feo

	parametros[posicionEnArray] = traducido;
};


void ejecutarTarea(){

	instruccion = leerMemoria(4, ptr_registros);

	if(strcmp(instruccion, "INTE") == 0){
		parametros[1] = ptr_hilo->tid;
		parametros[2] = kernel_socket;
		parametros[3] = ptr_hilo->segmento_codigo_size;
	};

	if(strcmp(instruccion, "JOIN") == 0){
		parametros[1] = ptr_hilo->tid;
		parametros[2] = kernel_socket;
	};

	if(strcmp(instruccion, "BLOK") == 0){
			parametros[1] = ptr_hilo->tid;
			parametros[2] = kernel_socket;
		};

	if(strcmp(instruccion, "WAKE") == 0){
			parametros[1] = ptr_hilo->tid;
			parametros[2] = kernel_socket;
		};

	ptr_registros->P += 4;


	switch (estructuraParametros(instruccion)){
	//hermoso

		//1 numero, registro, registro
		//2 registro, numero
		//3 registro, registro
		//4 numero, registro
		//5 registro
		//6 numero
		//7 nada

		case 1:{

			leerNumero(lista_parametros, parametros, 0);
			ptr_registros->P += 4;

			leerRegistro(lista_parametros, parametros, 1);
			ptr_registros->P += 1;

			leerRegistro(lista_parametros, parametros, 2);
			ptr_registros->P += 1;
			break;
		}
		case 2:{
			leerRegistro(lista_parametros, parametros, 0);
			ptr_registros->P += 1;

			leerNumero(lista_parametros, parametros, 1);
			ptr_registros->P += 4;
			break;
		}
		case 3:{
			leerRegistro(lista_parametros, parametros, 0);
			ptr_registros->P += 1;

			leerRegistro(lista_parametros, parametros, 1);
			ptr_registros->P += 1;
			break;
		}
		case 4:{
			leerNumero(lista_parametros, parametros, 0);
			ptr_registros->P += 4;

			leerRegistro(lista_parametros, parametros, 1);
			ptr_registros->P += 1;
			break;
		}
		case 5:{
			leerRegistro(lista_parametros, parametros, 0);
			ptr_registros->P += 1;
			break;
		}
		case 6:{
			leerNumero(lista_parametros, parametros, 0);
			ptr_registros->P += 4;
			break;
		}

		case 7:{
			break;
		}
		default:{
			printf("Error en lectura de instruccion. \n");
			exit(1);
			break;
		}
	};

	ejecucion_instruccion(instruccion, lista_parametros); //logger

	instrucciones(instruccion, ptr_registros, parametros); //la magia

	cambio_registros(*ptr_registros); //logger

	list_clean(lista_parametros);

}

int posicionDeBufferAInt(char* buffer, int posicion) {
	int logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return charToInt(buffer[posicion]);
}

int subCadenaAInt(char* text, int start, int length) {
	int retorno = 0;
	int logitudBuffer = 0;
	logitudBuffer = strlen(text);

	if (logitudBuffer >= (start + length)) {
		char* aux;
		aux = string_substring(text, start, length);
		retorno = atoi(aux);
		if (aux != NULL )
			free(aux);
	}
	return retorno;
}



char* tcbAString(t_hilo* tcbAEnviar){
	//FORMATO A ENVIAR: ABBBBCDDDDEF
	//A: CANTIDAD DE DIGITOS DE PID
	//BBBB: PID (MAXIMO 9999)
	//C: CANTIDAD DE DIGITOS DE TID
	//DDDD: TID (MAXIMO 9999)
	//E: KERNEL MODE (1 o 0)
	//F: CANTIDAD DE DIGITOS DE BASE SEGMENTO
	//GGGGGGGGG: BASE SEGMENTO
	//H: CANTIDAD DE DIGITOS DE BASE SEGMENTO SIZE
	//IIII: BASE SEGMENTO SIZE (MAXIMO 9999)
	//J: CANTIDAD DE DIGITOS PUNTERO INSTRUCCION
	//KKKK : PUNTERO INSTRUCCION
	//L: CANTIDAD DE DIGITOS BASE STACK
	//MMMMMMMMM : BASE STACK (MAXIMO 999999999)
	//N: CANTIDAD DE DIGITOS PUNTERO STACK
	//OOOO : PUNTERO STACK
	//N: CANTIDAD DE DIGITOS A
	//OOOO : A
	//N: CANTIDAD DE DIGITOS B
	//OOOO : B
	//N: CANTIDAD DE DIGITOS C
	//OOOO : C
	//N: CANTIDAD DE DIGITOS D
	//OOOO : D
	//N: CANTIDAD DE DIGITOS E
	//OOOO : E

	int cant_digitos_pid = calcularCantDigitos(tcbAEnviar->pid);
	int cant_digitos_tid = calcularCantDigitos(tcbAEnviar->tid);
	int cant_digitos_base_segmento = calcularCantDigitos(tcbAEnviar->segmento_codigo);
	int cant_digitos_base_segmento_size = calcularCantDigitos(tcbAEnviar->segmento_codigo_size);
	int cant_digitos_puntero_instruccion = calcularCantDigitos(tcbAEnviar->puntero_instruccion);
	int cant_digitos_base_stack = calcularCantDigitos(tcbAEnviar->base_stack);
	int cant_digitos_puntero_stack = calcularCantDigitos(tcbAEnviar->cursor_stack);

	int cant_digitos_a = calcularCantDigitos(tcbAEnviar->registros[0]);
	int cant_digitos_b = calcularCantDigitos(tcbAEnviar->registros[1]);
	int cant_digitos_c = calcularCantDigitos(tcbAEnviar->registros[2]);
	int cant_digitos_d = calcularCantDigitos(tcbAEnviar->registros[3]);
	int cant_digitos_e = calcularCantDigitos(tcbAEnviar->registros[4]);

	char* buffer = string_new();

	string_append(&buffer,string_itoa(cant_digitos_pid));
	string_append(&buffer,string_itoa(tcbAEnviar->pid));
	string_append(&buffer,string_itoa(cant_digitos_tid));
	string_append(&buffer,string_itoa(tcbAEnviar->tid));
	string_append(&buffer,string_itoa(tcbAEnviar->kernel_mode));
	string_append(&buffer,string_itoa(cant_digitos_base_segmento));
	string_append(&buffer,string_itoa(tcbAEnviar->segmento_codigo));
	string_append(&buffer,string_itoa(cant_digitos_base_segmento_size));
	string_append(&buffer,string_itoa(tcbAEnviar->segmento_codigo_size));
	string_append(&buffer,string_itoa(cant_digitos_puntero_instruccion));
	string_append(&buffer,string_itoa(tcbAEnviar->puntero_instruccion));
	string_append(&buffer,string_itoa(cant_digitos_base_stack));
	string_append(&buffer,string_itoa(tcbAEnviar->base_stack));
	string_append(&buffer,string_itoa(cant_digitos_puntero_stack));
	string_append(&buffer,string_itoa(tcbAEnviar->cursor_stack));


	string_append(&buffer, string_itoa(cant_digitos_a));
	string_append(&buffer, string_itoa(tcbAEnviar->registros[0]));
	string_append(&buffer, string_itoa(cant_digitos_b));
	string_append(&buffer, string_itoa(tcbAEnviar->registros[1]));
	string_append(&buffer, string_itoa(cant_digitos_c));
	string_append(&buffer, string_itoa(tcbAEnviar->registros[2]));
	string_append(&buffer, string_itoa(cant_digitos_d));
	string_append(&buffer, string_itoa(tcbAEnviar->registros[3]));
	string_append(&buffer, string_itoa(cant_digitos_e));
	string_append(&buffer, string_itoa(tcbAEnviar->registros[4]));

	return (buffer);
}

int pedirSegmentoAMemoria(uint32_t pid, int prog_size) {
	// Crear Segmento en la memoria
	// Formato del mensaje: CABBBBCDDDDDDD
	// C = Codigo de mensaje ( = 5)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el tamaño del segmento a crear
	// DDDDDDDD = El tamaño maximo de un segmento (hasta 9999999)

	int cant_digitos_pid;
	int cant_digitos_prog_size;
	cant_digitos_pid = calcularCantDigitos(pid);
	cant_digitos_prog_size = calcularCantDigitos(prog_size);

	char* buffer = string_new();
	string_append(&buffer, string_itoa(5));
	string_append(&buffer, string_itoa(cant_digitos_pid));
	string_append(&buffer, string_itoa(pid));
	string_append(&buffer, string_itoa(cant_digitos_prog_size));
	string_append(&buffer, string_itoa(prog_size));

	if(enviarDatos(msp_socket, buffer) == -1){
		printf("Error en enviar datos al pedir segmento a memoria");
		exit(1);
	};
	char respuestaMSP[BUFFERSIZE];
	recibirDatos(msp_socket, respuestaMSP);

	int direccion;
	direccion = atoi(respuestaMSP);

	return direccion;
}


void destruirSegmentoMemoria(int pid, int base_segmento){

	// Destruir segmento de la memoria
	// Formato del mensaje: CABBBBCDDDD....
	// C = Codigo de mensaje ( = 6)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el segmento
	// DDDD = El numero del segmento (hasta 4096)

	// Retorna: 1 + si se ejecuto correctamente
	//			0 + mensaje error si no se pudo leer

		int cant_digitos_pid;
		int cant_digitos_base_segmento;
		cant_digitos_pid = calcularCantDigitos(pid);
		cant_digitos_base_segmento = calcularCantDigitos(base_segmento);

		char* buffer = string_new();
		string_append(&buffer, string_itoa(6));
		string_append(&buffer, string_itoa(cant_digitos_pid));
		string_append(&buffer, string_itoa(pid));
		string_append(&buffer, string_itoa(cant_digitos_base_segmento));
		string_append(&buffer, string_itoa(base_segmento));

		if(enviarDatos(msp_socket, buffer) == -1){
			printf("Error en enviar datos al destruir segmento memoria");
			exit(1);
		};

};

void recibirTcbStringdeKernel(t_hilo* ptr_hilo){

	//FORMATO A ENVIAR: ABBBBCDDDDEF
	//A: CANTIDAD DE DIGITOS DE PID
	//BBBB: PID (MAXIMO 9999)
	//C: CANTIDAD DE DIGITOS DE TID
	//DDDD: TID (MAXIMO 9999)
	//E: KERNEL MODE (1 o 0)
	//F: CANTIDAD DE DIGITOS DE BASE SEGMENTO
	//GGGGGGGGG: BASE SEGMENTO
	//H: CANTIDAD DE DIGITOS DE BASE SEGMENTO SIZE
	//IIII: BASE SEGMENTO SIZE (MAXIMO 9999)
	//J: CANTIDAD DE DIGITOS PUNTERO INSTRUCCION
	//KKKK : PUNTERO INSTRUCCION
	//L: CANTIDAD DE DIGITOS BASE STACK
	//MMMMMMMMM : BASE STACK (MAXIMO 999999999)
	//N: CANTIDAD DE DIGITOS PUNTERO STACK
	//OOOO : PUNTERO STACK
	//P: cantidad digitos quantum
	//QQ: quantum

//		t_hilo* hiloAux = malloc(sizeof(t_hilo));

		char buffer[BUFFERSIZE];
		recibirDatos(kernel_socket, buffer);

//		t_hilo* hilo = malloc(56);


		int posicion = 0;
		int cantidadDigitosPID = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->pid = subCadenaAInt(buffer, posicion, cantidadDigitosPID);

		posicion = posicion + cantidadDigitosPID;
		int cantidadDigitosTID = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->tid = subCadenaAInt(buffer, posicion, cantidadDigitosTID);

		posicion = posicion + cantidadDigitosTID;
		ptr_hilo->kernel_mode = subCadenaAInt(buffer, posicion, 1);

		posicion++;
		int cantidadDigitosBase = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->segmento_codigo = subCadenaAInt(buffer, posicion, cantidadDigitosBase);

		posicion = posicion + cantidadDigitosBase;
		int cantidadDigitosBaseSize = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->segmento_codigo_size = subCadenaAInt(buffer, posicion, cantidadDigitosBaseSize);

		posicion = posicion + cantidadDigitosBaseSize;
		int cantidadDigitosIP = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->puntero_instruccion = subCadenaAInt(buffer, posicion, cantidadDigitosIP);

		posicion = posicion + cantidadDigitosIP;
		int cantidadDigitosBaseStack = posicionDeBufferAInt(buffer, posicion);
		posicion ++;
		ptr_hilo->base_stack = subCadenaAInt(buffer, posicion, cantidadDigitosBaseStack);

		posicion = posicion +cantidadDigitosBaseStack;
		int cantidadDigitosPtrStack= posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->cursor_stack = subCadenaAInt(buffer, posicion, cantidadDigitosPtrStack);

		posicion = posicion + cantidadDigitosPtrStack;
		int cantidadDigitosQuantum = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		quantum = subCadenaAInt(buffer, posicion, cantidadDigitosQuantum);

		posicion = posicion + cantidadDigitosQuantum;
		int cantidadDigitosA = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->registros[0] = subCadenaAInt(buffer, posicion, cantidadDigitosA);

		posicion = posicion + cantidadDigitosA;
		int cantidadDigitosB = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->registros[1] = subCadenaAInt(buffer, posicion, cantidadDigitosB);

		posicion = posicion + cantidadDigitosB;
		int cantidadDigitosC = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->registros[2] = subCadenaAInt(buffer, posicion, cantidadDigitosC);

		posicion = posicion + cantidadDigitosC;
		int cantidadDigitosD = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->registros[3] = subCadenaAInt(buffer, posicion, cantidadDigitosD);

		posicion = posicion + cantidadDigitosD;
		int cantidadDigitosE = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		ptr_hilo->registros[4] = subCadenaAInt(buffer, posicion, cantidadDigitosE);


//	return (hiloAux);
}

int main(){

	ptr_hilo = malloc(sizeof(t_hilo));
//	hiloAux = malloc(sizeof(t_hilo));
	ptr_registros = malloc(sizeof(t_registros_cpu));
	instruccion = string_new();
	char* buffer = string_new();

	//logger
	lista_parametros = list_create();
	inicializar_panel(CPU, PANEL_PATH);

	LeerConfigCPU();

	conexionKernel();
	conexionMSP();

	while(1){

		flagTerminoPrograma = 0;
		flagProcessSwitch = 0;
		flagSegFault = 0;

		//SOY CPU LIBRE
		if(enviarDatos(kernel_socket, "U1") == -1){
			printf("Error en enviar datos al mandar handshake kernel");
			exit(1);
		};

//		mensajeRecibido = string_new();
//
//		recibirDatos(kernel_socket, mensajeRecibido);
//
//		if(strcmp(mensajeRecibido,"1") != 0){
//			printf("El kernel no recibio el handshake correctamente");
//			exit(1);
//		}

		recibirTcbStringdeKernel(ptr_hilo);

		ptr_registros->I = ptr_hilo->pid;
		ptr_registros->M = ptr_hilo->segmento_codigo;
		ptr_registros->P = ptr_hilo->puntero_instruccion;
		ptr_registros->S = ptr_hilo->cursor_stack;
		ptr_registros->K = ptr_hilo->kernel_mode;
		ptr_registros->X = ptr_hilo->base_stack;
		ptr_registros->registros_programacion[0] = ptr_hilo->registros[0];
		ptr_registros->registros_programacion[1] = ptr_hilo->registros[1];
		ptr_registros->registros_programacion[2] = ptr_hilo->registros[2];
		ptr_registros->registros_programacion[3] = ptr_hilo->registros[3];
		ptr_registros->registros_programacion[4] = ptr_hilo->registros[4];

		comienzo_ejecucion(ptr_hilo, quantum);	//logger

		if(ptr_hilo->kernel_mode){

			while(!flagTerminoPrograma){
				ejecutarTarea();
				usleep(retardo*1000);
			}

		}else{

			int k;
			for(k=0; k<quantum; k++){
				if(!flagTerminoPrograma && !flagProcessSwitch && !flagSegFault){
					ejecutarTarea();
					usleep(retardo*1000);
				}
				else break;
			}

		};

		//actualizo el hilo con los valores de los registros de cpu
		ptr_hilo->cursor_stack = ptr_registros->S;
		ptr_hilo->puntero_instruccion = ptr_registros->P;
		ptr_hilo->registros[0] = ptr_registros->registros_programacion[0];
		ptr_hilo->registros[1] = ptr_registros->registros_programacion[1];
		ptr_hilo->registros[2] = ptr_registros->registros_programacion[2];
		ptr_hilo->registros[3] = ptr_registros->registros_programacion[3];
		ptr_hilo->registros[4] = ptr_registros->registros_programacion[4];


		buffer = tcbAString(ptr_hilo);
		//devuelvo el hilo al kernel

		char* mensaje = string_new();
		char mensajeRecibido [BUFFERSIZE];

		if(flagProcessSwitch){
			continue;
		}
		if(flagSegFault){
			string_append(&mensaje, "U3");
		}else{
			if(flagTerminoPrograma){
				string_append(&mensaje,"U2");
			}
			if(!flagTerminoPrograma){
				string_append(&mensaje,"U0");
			}
		}

		if(enviarDatos(kernel_socket, mensaje) == -1){
			printf("Error en enviar datos al devolver tcb a kernel");
			exit(1);
		};

		recibirDatos(kernel_socket, mensajeRecibido);

		if(strcmp(mensajeRecibido,"1") != 0){
			printf("El kernel no mandó el ok para seguir");
			exit(1);
		}

		if(enviarDatos(kernel_socket, buffer) == -1){
			printf("Error en enviar datos al devolver tcb a kernel");
			exit(1);
		};

		fin_ejecucion(); //logger
	}

};


