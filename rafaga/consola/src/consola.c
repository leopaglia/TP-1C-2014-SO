/*
 * consola.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 *
 *  Test:
 *  	  -setear en consola_config.cfg el ip_kernel.
 *  	  -ejecutar primero el kernel, y despues la consola.
 *  	  -pasar el path al archivo .bc (/home/utnso/git/tp-2014-2c-jjjls/rafaga/consola/stack.bc),
 */

#include "consola.h"

#define PATH_CONFIG "consola_config.cfg"
char* puerto_kernel;
char* ip_kernel;
size_t prog_tamanio;

int enviarPrograma(int socket_kernel,char* programa,size_t prog_tamanio) {
	int bytecount;
			if ((bytecount = send(socket_kernel, programa, prog_tamanio, 0)) == -1)
				log_info(loggerConsola, "No puedo enviar información al clientes. Socket: %d. Config: %s", socket, PATH_CONFIG);

			log_trace(loggerConsola, "ENVIO datos. socket: %d. buffer: %s", socket_kernel,
					(char*) programa);

			return (bytecount);
}

int enviarDatos(int socket, void *buffer){
	int bytecount;
		if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1){
			log_info(loggerConsola, "No puedo enviar información al clientes. Socket: %d. Config: %s", socket, PATH_CONFIG);
		}
		log_info(loggerConsola, "ENVIO datos. socket: %d. buffer: %s", socket,
				(char*) buffer);

		return (bytecount);
}

int recibirDatos(int socket,char *buffer) {
	int bytecount;
	// memset se usa para llenar el buffer con 0s
	memset(buffer, 0, BUFFERSIZE);

	if ((bytecount = recv(socket, buffer, BUFFERSIZE, 0)) == -1){
//		log_info(loggerConsola, "ERROR: error al intentar recibir datos del kernel");
		log_info(loggerConsola, "ERROR al intentar recibir datos. socket: %d. buffer: %s\n", socket,
					(char*) buffer);
		exit(1);
	}
	log_info(loggerConsola, "RECIBO datos. socket: %d. buffer: %s\n", socket,
			(char*) buffer);
	return (bytecount);
}

int calcularCantDigitos(int num) {
	int contador = 1;

	while (num / 10 > 0) {
		num = num / 10;
		contador++;
	}

	return contador;
}

void enviarCodigoAlKernel(char* codigo){
	log_info(loggerConsola, "Intentando conectar a kernel\n");

	//conectar con kernel
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if(getaddrinfo(ip_kernel, puerto_kernel, &hints, &serverInfo) != 0 ){	// Carga en serverInfo los datos de la conexion
		log_info(loggerConsola, "ERROR: cargando datos de conexion socket_consola");
	}

	if ((socket_kernel = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0){
		log_info(loggerConsola, "ERROR: crear socket_kernel");
	}

	if(connect(socket_kernel, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0 ){
		log_info(loggerConsola, "ERROR: conectar socket_kernel");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	//INSERTAR HANDSHAKE AL PRINCIPIO DEL CODIGO
	int cantDigitosTamanio = calcularCantDigitos(prog_tamanio);
	char* handshake = string_new();
	string_append(&handshake, HANDSHAKE_CONSOLA);
	string_append(&handshake,string_itoa(cantDigitosTamanio));
	string_append(&handshake,string_itoa(prog_tamanio));

	//ENVIO HANDSHAKE
	if (enviarDatos(socket_kernel,handshake) == -1){
		log_info(loggerConsola, "ERROR FATAL: no se pudo enviar handshake al kernel");
		exit(1);
	}
	log_info(loggerConsola, "Se envió el handshake al kernel");

	//RECIBO CONFIRMACION
	char respuestaKERNEL[BUFFERSIZE];
	if(recibirDatos(socket_kernel,respuestaKERNEL) == 0){
		log_info(loggerConsola, "ERROR FATAL: se desconectó el kernel esperando confirmación.");
		exit(1);
	}

	if(respuestaKERNEL[0] == 'N'){
		log_info(loggerConsola, "El kernel rechazó la conexión");
		exit(1);
	}
	log_info(loggerConsola, "Conexión con kernel aceptada");

	//ENVIA PROGRAMA
	if (enviarPrograma(socket_kernel,codigo,prog_tamanio) == -1){
		log_info(loggerConsola, "ERROR: no se puedo enviar programa al kernel");
	}
	log_info(loggerConsola, "Programa enviado");

	if(recibirDatos(socket_kernel,respuestaKERNEL) == 0 ){
		log_info(loggerConsola, "ERROR FATAL: se desconectó el kernel esperando confirmación de recepción de programa.");
		exit(1);
	}
	//RECIBO CONFIRMACION
	if(respuestaKERNEL[0] == 'N'){
		log_info(loggerConsola, "El kernel rechazó el programa.");
		exit(1);
	}
	log_info(loggerConsola, "El kernel recibió el programa correctamente.");

}

void leerProgramaCompilado(){

//	system("clear");
	printf("\nIngrese ubicación del archivo a ejecutar:");

	char* ubicacion_programa;
	ubicacion_programa = (char*) malloc (128*sizeof(char));	//(char*) malloc(strlen(argv[1]));
	scanf("%s",ubicacion_programa);

	FILE * programa;
	programa = fopen(ubicacion_programa,"rb");

	if(programa == NULL){
			log_info(loggerConsola, "ERROR: al abrir el archivo : %s . Ubicación inválida o archivo inexistente.", ubicacion_programa);
			exit(1);
	}

	log_info(loggerConsola, "Se abrió el archivo : %s.", ubicacion_programa);

//	size_t prog_tamanio;
	size_t BytesLeidos;
	fseek(programa,0, SEEK_END);	//nos situa en el final del archivo
	prog_tamanio=ftell(programa);	//devuelve el tamaño del archivo en bytes
	rewind(programa);//nos situa en el comienzo del archivo

	char* buffer;
	buffer = (char*) malloc(prog_tamanio*sizeof(char));
	if (buffer == NULL ) {
			log_info(loggerConsola, "ERROR: no hay memoria disponible\n");
			exit(1);
		}

	BytesLeidos=fread(buffer,sizeof(char),prog_tamanio,programa);	//cargo el programa en buffer
	int largo;
	largo = strlen(buffer);
	log_info(loggerConsola, "Tamanio del programa a leer: %d. Tamanio leido: %d. strlen(buffer): %d\n",prog_tamanio,BytesLeidos, largo);

	enviarCodigoAlKernel(buffer);

	free(ubicacion_programa);
	free(buffer);
	fclose(programa);
}

void entrada(char* buffer){

	if(buffer[1] == 'N') {
		int numero;
		printf("Ingrese un número entre -2.147.483.648 y 2.147.483.647: \n");
		scanf("%d", &numero);
		enviarDatos(socket_kernel, string_itoa(numero));
	};

	if(buffer[1] == 'C') {
		char* string = string_new();
		printf("Ingrese una cadena de como máximo 5 caracteres: \n");
		scanf("%s", string);
		enviarDatos(socket_kernel, string);
	};

};

void salida(char* buffer){

	char* string = string_new();
	string = string_substring(buffer, 1, string_length(buffer)-1);
	printf("Salida estandar: %s \n", string);

};

void imprimirMensajeKernel(char* buffer){

	char* string = string_new();

	string_append(&string, string_substring(buffer, 1, string_length(buffer)-1));

	printf("Mensaje de Kernel: %s \n", string);
}

int main(){

	loggerConsola = log_create("consola.log", "CONSOLA", 0, LOG_LEVEL_DEBUG); //Creo el Log de Consola
	LeerConfigConsola();

	leerProgramaCompilado();	//carga el codigo binario y lo envia al kernel

	char buffer[BUFFERSIZE];

	while(1){

		recibirDatos(socket_kernel, buffer);

		if(strlen(buffer) == 0) exit(1);


		if(buffer[0] == 'E'){
			entrada(buffer);
		}
		if(buffer[0] == 'S'){
			salida(buffer);
		}
		if(buffer[0] == 'M'){
			imprimirMensajeKernel(buffer);
		}

	};

	return 1;
}

void LeerConfigConsola() {
	t_config* config = config_create(PATH_CONFIG);

	if(config->properties->table_current_size !=0){

		if(config_has_property(config, "IP_KERNEL")){
			ip_kernel = config_get_string_value(config, "IP_KERNEL");
		} else log_info(loggerConsola, "ERROR: no se pudo leer el parámetro IP_KERNEL");

		if(config_has_property(config, "PUERTO_KERNEL")){
			puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
		} else log_info(loggerConsola, "ERROR: no se pudo leer el parámetro PUERTO_KERNEL");
	} else {
		log_info(loggerConsola, "FATAL ERROR: no se pudo abrir el archivo de configuración");
		exit(1);
	}
	if(config != NULL){
		free(config);
	}
}
