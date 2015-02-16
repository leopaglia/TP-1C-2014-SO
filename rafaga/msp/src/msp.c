/*
 ============================================================================
 Name        : msp.c
 Author      : jjjls
 Version     : 1.0
 Copyright   : JJJLS - UTN FRBA 2014
 Description : Trabajo Practivo Sistemas Operativos 2C 2014
 Testing	 : Para probarlo es tan simple como ejecutar en el terminator la linea "$ telnet localhost 7000" y empezar a dialogar con el UMV.
 A tener en cuenta: organizar codigo : ctrl+Mayúscula+f
 PARA HACER
 - algun dump?
 - permitir definir lugar del archivo y nombre--
 ============================================================================
 */

#include "msp.h"

int main(int argv, char** argc) {
	//inicializamos los semaforos
	sem_init(&semaforoAccesoMemoria, 0, 1);
	sem_init(&semaforoMarcosLibres, 0, 0);

	// Instanciamos el archivo donde se grabará lo solicitado por consola
	g_ArchivoConsola = fopen(NOMBRE_ARCHIVO_CONSOLA, "wb");
	g_MensajeError = malloc(1 * sizeof(char));
	//char* temp_file = tmpnam(NULL);

	logger = log_create(NOMBRE_ARCHIVO_LOG, "msp", true, LOG_LEVEL_TRACE);

	// Levantamos el archivo de configuracion.
	LevantarConfig();

	//Creamos una Lista de Segmentos
	crearListaSegmentos();
	crearListaMarcosLibres();
	crearListaAlgoritmo();

	// Intentamos reservar la memoria principal
	reservarMemoriaPrincipal();

	//Lanzamos los hilos
	//Hilo consola
	int iThreadConsola = pthread_create(&hConsola, NULL, (void*) comenzar_msp,
			NULL );
	if (iThreadConsola) {
		fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadConsola);
		exit(EXIT_FAILURE);
	}

	//Hilo orquestador conexiones
	int iThreadOrquestador = pthread_create(&hOrquestadorConexiones, NULL,
			(void*) HiloOrquestadorDeConexiones, NULL );
	if (iThreadOrquestador) {
		fprintf(stderr,
				"Error al crear hilo - pthread_create() return code: %d\n",
				iThreadOrquestador);
		exit(EXIT_FAILURE);
	}

	pthread_join(hOrquestadorConexiones, NULL );
	pthread_join(hConsola, NULL );

	// Liberamos recursos
	if (g_BaseMemoria != NULL )
		free(g_BaseMemoria);
	list_clean_and_destroy_elements(listaSegmentos, (void*) segmento_destroy);
	if (g_MensajeError != NULL )
		free(g_MensajeError);

	//Cerramos el archivo.
	fclose(g_ArchivoConsola);

	return EXIT_SUCCESS;
}

void crearListaMarcosLibres() {
	listaMarcosLibres = lista_marco_create(g_CantidadMemoria);
}

void crearListaSegmentos() {
	listaSegmentos = list_create();
}

void crearListaAlgoritmo() {
	listaAlgoritmo = list_create();
}

int crearSegmento(int pid, int tamanio) {

	//Aca nos fijamos si hay memoria disponible en MP y SW
	if (tamanio != 0) {
		if (memoriaDisponible(tamanio)) {
			return agregarSegmento(pid, tamanio);
		} else {
			Error("No hay memoria disponible");
			return -1;
		}
	} else {
		Error("Se ingreso un tamanio 0");
		return -1;
	}
}

int memoriaDisponible(int tamanio) {
	/*Dividir el tamaño en 256 para saber cuantos marcos necesitamos y consultar
	 la lista de marcos libres de la MP y sino revisar el Swapping(manejamos lista?)
	 */
	int memoria = list_size(listaMarcosLibres) * TAMANIO_PAGINA;
	printf("La memoria total es de: %ld\n", (memoria + memSwap));
	if ((memoria + memSwap) >= tamanio) {
		return 1;
	} else {
		return 0;

	}
}

int agregarSegmento(int pid, int tamanio) {
	//Me fijo si existe un segmento con el pid del parametro
	bool _true(void *seg) {
		return (((t_segmento*) seg)->pid == pid);
	}
	numSegmento = list_count_satisfying(listaSegmentos, _true);
	//Enumero el segmento segun la cantidad que ya tenga
	bool _true2(void *seg) {
		return (((t_segmento*) seg)->pid == pid
				&& ((t_segmento*) seg)->nroSegmento == numSegmento);
	}
	if (numSegmento != 0) {
		numSegmento = 0;
		while ((list_count_satisfying(listaSegmentos, _true2)) != 0) {
			numSegmento++;
		}
	};
	list_add(listaSegmentos, segmento_create(pid, tamanio, numSegmento));
	return (numSegmento);
}

#if 1 // METODOS MANEJO DE ERRORES //
void Error(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);

	fprintf(stderr, "\nERROR: %s\n", nuevo);
	log_error(logger, "%s", nuevo);

	va_end(arguments);
	if (nuevo != NULL )
		free(nuevo);
}

#endif

void ErrorFatal(const char* mensaje, ...) {
	char* nuevo;
	va_list arguments;
	va_start(arguments, mensaje);
	nuevo = string_from_vformat(mensaje, arguments);
	printf("\nERROR FATAL--> %s \n", nuevo);
	log_error(logger, "\nERROR FATAL--> %s \n", nuevo);
	char fin;

	printf(
			"El programa se cerrara. Presione ENTER para finalizar la ejecución.");
	fin = scanf("%c", &fin);

	va_end(arguments);
	if (nuevo != NULL )
		free(nuevo);
	exit(EXIT_FAILURE);
}

#if 1 // METODOS CONFIGURACION //
void LevantarConfig() {
	t_config* config = config_create(PATH_CONFIG);
	// Nos fijamos si el archivo de conf. pudo ser leido y si tiene los parametros
	if (config->properties->table_current_size != 0) {

		// Preguntamos y obtenemos el puerto del config
		if (config_has_property(config, "PUERTO")) {
			g_Puerto = config_get_int_value(config, "PUERTO");
		} else
			Error("No se pudo leer el parametro PUERTO");

		// Preguntamos y obtenemos tamaño de la memoria (cantidad de bytes del 'gran malloc')
		if (config_has_property(config, "CANTIDAD_MEMORIA")) {
			g_CantidadMemoria = config_get_int_value(config,
					"CANTIDAD_MEMORIA");
		} else
			Error("No se pudo leer el parametro CANTIDAD_MEMORIA");

		// Preguntamos y obtenemos tamaño del archivo de swapping
		if (config_has_property(config, "CANTIDAD_SWAP")) {
			g_CantidadSwap = config_get_int_value(config, "CANTIDAD_SWAP");
			memSwap = g_CantidadSwap * 1024 * 1024;
		} else
			Error("No se pudo leer el parametro CANTIDAD_SWAP");

		// Obtenemos el algoritmo por defecto
		if (config_has_property(config, "SUST_PAGS")) {
			char* algoritmo = config_get_string_value(config, "SUST_PAGS");
			g_SustPags = algoritmo;
			//if (algoritmo != NULL )
			//	free(algoritmo);
		} else
			Error("No se pudo leer el parametro SUST_PAGS");
	} else {
		ErrorFatal("No se pudo abrir el archivo de configuracion");
	}
	if (config != NULL ) {
		free(config);
	}
}

#endif

void reservarMemoriaPrincipal() {
// Reservamos la memoria
	g_BaseMemoria = (char*) malloc(g_CantidadMemoria);
// Rellenamos con ceros.
	memset(g_BaseMemoria, '0', g_CantidadMemoria * sizeof(char));

// si no podemos salimos y cerramos el programa.
	if (g_BaseMemoria == NULL ) {
		ErrorFatal("No se pudo reservar la memoria.");
	} else {
		//Traza("Se reservó la memoria principal OK. Tamaño de la memoria (%d)", g_TamanioMemoria);
		log_trace(logger,
				"MEMORIA PRINCIPAL RESERVADA. Tamaño de la memoria (%d)",
				g_CantidadMemoria);
	}

}

void comenzar_msp() {

	//vector_memoria_ocupada=abrir_socket(vector_memoria_ocupada);

	while (corte_msp() != 0) {
		operaciones_consola();//menu de consola para elegir la opcion a realizar en msp
	}
	printf("Se termino la ejecucion de la consola de la msp\n");
	//free(puntero_inicial);
}

int corte_msp() {
	int corte;
	printf(
			"\n\n\ningrese 0 si desea terminar la consola de msp, sino ingrese otro numero\n\n\n");

	scanf("%d", &corte);
	return corte;
}

int leerBinario(char *respuesta){
	int *t;
	char numero[4];
	numero[0] = respuesta[0];
	numero[1] = respuesta[1];
	numero[2] = respuesta[2];
	numero[3] = respuesta[3];
	t = numero;
	//printf("\nEL GRAN NUMERO POSTA DEL BINARIO:%d\n",*t);
	return *t;
}

void operaciones_consola() {
	system("clear");
	printf("\n 1)CREAR SEGMENTO \n");
	printf("\n 2)DESTRUIR SEGMENTOS\n");
	printf("\n 3)ESCRIBIR MEMORIA\n");
	printf("\n 4)LEER MEMORIA\n");
	printf("\n 5)TABLA DE SEGMENTOS \n");
	printf("\n 6)TABLA DE PAGINAS \n");
	printf("\n 7)LISTAR MARCOS \n");
	printf("\n 8)LEER ESPECIAL\n");
//void *buffer;
//t_socket *socket=NULL;

	int variable_seleccion;
	scanf("%d", &variable_seleccion);

	int inicio, pid, offset;
	int base, tamanio, pagina;
	char *texto, c;	//Que onda el 1000?
	//texto = malloc(1000*sizeof(char));
	system("clear");
	switch (variable_seleccion) {

	case 1:
		log_info(logger, "Ejecutando crear segmento...\n");

		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		printf("\n Ingrese un tamanio\n");
		scanf("%d", &tamanio);
		system("clear");
		if (tamanio > TAMANIO_PAGINA * CANTIDAD_MAX_SEGMENTOS) {
			log_error(logger,
					"El el tamanio del segmento supera el maximo establecido.");
			break;
		}
		sem_wait(&semaforoAccesoMemoria);
		int baseSeg = crearSegmento(pid, tamanio);//el cero es para que se de cuenta que no lo llame por socket
		sem_post(&semaforoAccesoMemoria);
		//system("clear");
		if (baseSeg != -1) {
			printf("La direccion base del segmento es: %d", baseSeg);
		}
		break;

	case 2:
		log_info(logger, "Se realizo destruir segmento\n");

		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		printf("\n Ingrese la direccion base\n");
		scanf("%d", &base);
		sem_wait(&semaforoAccesoMemoria);
		destruirSegmento(pid, base);
		//CONTEMPLAR QUE AL ELIMINAR LIBERE LOS MARCOS DE MEMORIA
		sem_post(&semaforoAccesoMemoria);

		break;

	case 3:
		log_info(logger, "Se realizo escribir en memoria\n");
		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		printf("\n Ingrese la direccion base\n");
		scanf("%d", &base);
		printf("\n Ingrese el numero de pagina\n");
		scanf("%d", &pagina);
		printf("\n Ingrese el desplazamiento(offset)\n");
		scanf("%d", &offset);
		printf("\n Ingrese el tamañio a escribir\n");
		scanf("%d", &tamanio);
		printf("\n Ingrese el texto a escribir\n");
		int i = 0;
		getc(stdin);
		texto = malloc(sizeof(char));
		while ((c = getc(stdin)) != 10) {
			texto = realloc(texto, i + 1);
			texto[i++] = c;
		}
		texto = realloc(texto, tamanio + 1);
		memset((texto+i),' ',tamanio-i);
		texto[tamanio] = '\0';
		if (strlen(texto) <= tamanio) {
			char* aux;

			if ((offset + tamanio) > TAMANIO_PAGINA) {
				//printf("TAMANIO DE TEXTO:%d\n", strlen(texto));
				sem_wait(&semaforoAccesoMemoria);
				escribirMemoria(pid, base, pagina, offset,
						(TAMANIO_PAGINA - offset), texto);
				sem_post(&semaforoAccesoMemoria);
				//grabo lo que queda de la pagina despues del offset
				tamanio = tamanio - (TAMANIO_PAGINA - offset);
				char *aux2 = malloc(tamanio + 1);
				while (tamanio > TAMANIO_PAGINA) {
					pagina++;
					aux = (texto + (TAMANIO_PAGINA - offset));
					sem_wait(&semaforoAccesoMemoria);
					escribirMemoria(pid, base, pagina, 0, TAMANIO_PAGINA, aux);
					sem_post(&semaforoAccesoMemoria);
					tamanio = tamanio - TAMANIO_PAGINA;
					offset = 0;
				}
				pagina++;
				aux = (texto + (TAMANIO_PAGINA - offset));
				memset(aux2, ' ', tamanio);
				memcpy(aux2, aux, strlen(aux));
				//printf("ESTO ES AUX: %s\n", aux);
				//printf("PID:%d, BASE:%d, PAGINA:%d, Tamanio:%d\n", pid, base,
				//		pagina, tamanio);
				sem_wait(&semaforoAccesoMemoria);
				escribirMemoria(pid, base, pagina, 0, strlen(aux2), aux2);
				sem_post(&semaforoAccesoMemoria);
				printf("Se escribio en la memoria correctamente.\n");
				free(aux2);
			} else {
				//printf("TAMANIO DE TEXTO:%d\n", sizeof(texto));
				sem_wait(&semaforoAccesoMemoria);
				escribirMemoria(pid, base, pagina, offset, tamanio, texto);
				sem_post(&semaforoAccesoMemoria);
				printf("Se escribio en la memoria correctamente.\n");
			}
			free(texto);

		} else {
			printf("Error: Se ingreso mas texto de lo solicitado en tamanio\n");
			log_error(logger,
					"el tamanio ingresado por teclado es superior al tamanio solicitado");
			free(texto);
		}
		break;

	case 4:
		log_info(logger, "Se realizo leer memoria\n");
		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		int z=0;
		printf("\n Ingrese la direccion base\n");
		scanf("%d", &base);
		printf("\n Ingrese el numero de pagina\n");
		scanf("%d", &pagina);
		printf("\n Ingrese el desplazamiento(offset)\n");
		scanf("%d", &offset);
		printf("\n Ingrese el tamañio a leer\n");
		scanf("%d", &tamanio);
		char *lectura = malloc(tamanio + 1);
		printf("Contenido de la memoria solicitado:");
		if ((offset + tamanio) > TAMANIO_PAGINA) {
			sem_wait(&semaforoAccesoMemoria);
			leerMemoria(pid, base, pagina, offset, (TAMANIO_PAGINA - offset),
					lectura);
			sem_post(&semaforoAccesoMemoria);
			printf("Pagina %d: ", pagina);
			while(z<(TAMANIO_PAGINA-offset)){
				printf("%c",lectura[z++]);
			}
			printf("\n");
			//grabo lo que queda de la pagina despues del offset
			tamanio = tamanio - (TAMANIO_PAGINA - offset);
			while (tamanio > TAMANIO_PAGINA) {
				pagina++;
				sem_wait(&semaforoAccesoMemoria);
				leerMemoria(pid, base, pagina, 0, TAMANIO_PAGINA, lectura);
				sem_post(&semaforoAccesoMemoria);
				tamanio = tamanio - TAMANIO_PAGINA;
				printf("Pagina %d: ", pagina);
				while(z<TAMANIO_PAGINA){
					printf("%c",lectura[z++]);
				}
				printf("\n");
			}
			pagina++;
			sem_wait(&semaforoAccesoMemoria);
			leerMemoria(pid, base, pagina, 0, tamanio, lectura);
			sem_post(&semaforoAccesoMemoria);
			printf("Pagina %d: ", pagina);
			while(z<tamanio){
				printf("%c",lectura[z++]);
			}
			printf("\n");
		} else {
			sem_wait(&semaforoAccesoMemoria);
			leerMemoria(pid, base, pagina, offset, tamanio, lectura);
			sem_post(&semaforoAccesoMemoria);
			printf("Pagina %d: ", pagina);
			while(z<tamanio){
				printf("%c",lectura[z++]);
			}
			printf("\n");
		}
		free(lectura);
		break;

	case 5:
		log_info(logger, "Se realizo mostrar la tabla de segmentos\n");
		mostrarListaSegmentos();
		break;

	case 6:
		log_info(logger, "Se realizo mostrar la tabla de paginas\n");
		int segmento;
		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		system("clear");
		mostrarListaSegmentosYPaginas(pid);
		break;

	case 7:
		log_info(logger, "Se realizo listar marcos\n");
		mostrarListaMarcosLibres(listaMarcosLibres);
		printf("Cantidad total de Marcos Libres: %i\n",
				list_size(listaMarcosLibres));
		break;

	case 8:
		printf("\n Ingrese un PID\n");
		scanf("%d", &pid);
		if (pid == 0) {
			log_error(logger,
					"El PID no es correcto, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
			break;
		}
		printf("\n Ingrese la direccion base\n");
		scanf("%d", &base);
		printf("\n Ingrese el numero de pagina\n");
		scanf("%d", &pagina);
		printf("\n Ingrese el desplazamiento(offset)\n");
		scanf("%d", &offset);
		printf("\n Ingrese el tamañio a leer\n");
		scanf("%d", &tamanio);
		char *lectura2 = malloc(tamanio + 1);
		printf("Contenido de la memoria solicitado:");
		if ((offset + tamanio) > TAMANIO_PAGINA) {
			leerMemoria(pid, base, pagina, offset, (TAMANIO_PAGINA - offset),
					lectura2);
			printf("Pagina %d: %s", pagina, lectura2);
			//grabo lo que queda de la pagina despues del offset
			tamanio = tamanio - (TAMANIO_PAGINA - offset);
			while (tamanio > TAMANIO_PAGINA) {
				pagina++;
				leerMemoria(pid, base, pagina, 0, TAMANIO_PAGINA, lectura2);
				tamanio = tamanio - TAMANIO_PAGINA;
				printf("Pagina %d: %s", pagina, lectura2);
			}
			pagina++;
			leerMemoria(pid, base, pagina, 0, tamanio, lectura2);
			printf("Pagina %d: %s", pagina, lectura2);
		} else {
			leerMemoria(pid, base, pagina, offset, tamanio, lectura2);
			printf("Pagina %d: %s", pagina, lectura2);
		}
		printf("NUMERO: %i\n",leerBinario(lectura2));
		free(lectura2);
		break;

	default:
		operaciones_consola();
		break;
	}

}

int leerMemoria(int pid, int base, int pagina, int offset, int tamanio,
		char* aux) {
	t_segmento * tsegmento;
	t_pagina * tpagina;
	char * contenidoSwapping = malloc(TAMANIO_PAGINA +1);
	char * texto = malloc(tamanio * sizeof(char) + 1);

	//Buscamos el pid y el segmento dentro de la tabla de segmentos
	tsegmento = buscarListaPagina(pid, base);
	if (tsegmento == NULL ) {
		log_error(logger,
				"El PID no existe o el segmento no existe, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
		return 0;
	}

	//Aca verificamos que lo que se va a leer no sea mas que el tamanio del segmento
	if (tsegmento->tamanio < tamanio) {
		log_error(logger, "Segmentation Fault");
		return 0;
	}

	//Obtenemos la pagina de la lista de paginas
	tpagina = buscarPagina(tsegmento, pagina);
	if (tpagina == NULL ) {
		log_error(logger,
				"La pagina no existe en el segmento, vuelva a intentarlo.\n");
		return 0;
	}

	//Verificamos que desde el offset de la pagina se pueda escribir o que haya mas paginas
	if (((TAMANIO_PAGINA - offset)
			+ (cantidadPaginas(tpagina, tsegmento->lista_paginas)
					* TAMANIO_PAGINA)) < tamanio) {
		log_error(logger, "Segmentation Fault");
		return 0;
	}

	//Aca verificamos que la pagina no este swappeada
	if (tpagina->swapping == 1) {
		//ASIGNARLE UN MARCO LIBRE Y TRAER DE SWAPPING A MEMORIA
		while (!asignarMarcoLibre(tpagina, pid, base, 0)) {
			swappear(tpagina, pid, base);
			//log_error(logger,"No hay memoria disponible");
			//return 0;
		}
		//La funcion traerSwapping devuelve el contenido del archivo de Swap de la Pagina
		//printf("ROMPIO ACA\n");
		contenidoSwapping = (traerSwapping(tpagina, pid, base));
		/*if(contenidoSwapping){
		 log_error(logger,
		 "Error Grave: No existe archivo de swapping");
		 return 0;
		 }*/
		//printf("O ROMPIO ACA\n");
		if (!grabarEnMemoria(tpagina->marco, 0, TAMANIO_PAGINA,
				contenidoSwapping)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al escribir en memoria principal");
			return 0;
		}
		if (!leerEnMemoria(tpagina->marco, offset, tamanio, aux)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al leer en memoria");
		}
	} else {
		if (tpagina->marco == -1) {
			//ASIGNARLE UN MARCO LIBRE
			while (!asignarMarcoLibre(tpagina, pid, base, 0)) {
				swappear(tpagina, pid, base);
				//log_error(logger,"No hay memoria disponible");
				//return 0;
			}
		}
		if (!leerEnMemoria(tpagina->marco, offset, tamanio, aux)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al leer en memoria");
		}
	}
	return 1;
}

int leerEnMemoria(int nroMarco, int posicion, int tamanio, char * buffer) {
	char * memoria = g_BaseMemoria;
	int i = 0;
	int contador = tamanio;
	char* aux = malloc(tamanio + 1);
	memset(aux, '0', tamanio * sizeof(char)); //Setea todo el aux a 0

	memoria = memoria + ((nroMarco * TAMANIO_PAGINA) + posicion);
	while (contador--) {
		aux[i] = memoria[i];
		i++;
	}
	aux[i] = '\0';
	leerBinario(aux);
	memcpy(buffer, aux, tamanio); //Copia todo el aux en buffer
	return 1;
}

int cantidadPaginas(t_pagina* pagina, t_list *lista_paginas) {
	return (list_size(lista_paginas) - (pagina->nroPagina) - 1);
}

char* traerSwapping(t_pagina *pagina, int pid, int base) {

	char* fileName = generarNombreArchivo(pagina->nroPagina, pid, base);
	int elementosLeidos;
	//Calcula tamano del elemento a leer
	//struct stat statSwap;
	//stat(fileName, &statSwap);

	//Abro el archivo
	FILE* archSwap = fopen(fileName, "rb");
	char* swap_read = malloc(TAMANIO_PAGINA);
	char* swap_leer = malloc(TAMANIO_PAGINA);
	memset(swap_leer, ' ', TAMANIO_PAGINA * sizeof(char));//Setea todo el aux a 0
	//Leo el archivo y lo guardo en swap_read, el 1 es xq solo hay un elemento en el archivo
	elementosLeidos = fread(swap_read, sizeof(char), TAMANIO_PAGINA, archSwap);
	memcpy(swap_leer, swap_read, TAMANIO_PAGINA);//Copia todo texto en el aux
	//printf("se leyeron %d elementos \n",elementosLeidos);
	//printf("%s fin\n",swap_read);
	fclose(archSwap);
	remove(fileName);	//Elimina el archivo cuando se levanta el swap

	pagina->swapping = 0;
	return swap_leer;
}

char* generarNombreArchivo(int pagina, int pid, int base) {

//Paso a string todos los int para el nombre del archivo
	char* nroPag = string_itoa(pagina);
	char* nroPid = string_itoa(pid);
	char* nroSeg = string_itoa(base);
	char* fileName = string_new();

//Concateno todos los string para el formato de nombre del archivo "PidSegPag.bin"
	string_append(&fileName, nroPid);
	string_append(&fileName, "-");
	string_append(&fileName, nroSeg);
	string_append(&fileName, "-");
	string_append(&fileName, nroPag);
	string_append(&fileName, ".bin");

	return fileName;
}

int obtenerMarcoLibre() {
	t_marco * marco;
	int nroMarco;
	marco = list_get(listaMarcosLibres, 0);
	if (marco == NULL ) {
		return -1;
	}
	nroMarco = marco->nroMarco;
	list_remove(listaMarcosLibres, 0);
	return nroMarco;
}

int asignarMarcoLibre(t_pagina* tpagina, int pid, int base, int modo) { //modo es para el BM del clock, si escribe una pagina swapeada cambia a 1
	int marcoLibre = obtenerMarcoLibre();
	if ((tpagina->marco = (marcoLibre)) != -1) {
		agregarAListaAlgoritmo(marcoLibre, tpagina->nroPagina, pid, base, modo);
		return 1;
	} else {
		log_info(logger, "No se pudo obtener un marco libre\n");
		return 0;
	}
}

int swappear(t_pagina* tpagina, int pid, int base) {
	//Busco el marco a swappear por algoritmos
	//Creo el archivo de swapp de la pagina seleccionada
	//Devuelvo el marco donde va a grabar la nueva pagina
	t_algoritmoSust* elemSwap = sustitucionPag(g_SustPags);
	printf("Swapping antes de crear el archivo: %ld\n", (memSwap));
	char* nomArchivo = generarNombreArchivo(elemSwap->nroPagina, elemSwap->pid,
			elemSwap->base);
	FILE* archivoSwap = fopen(nomArchivo, "wb");
	grabarArchivoSwap(elemSwap->marco, archivoSwap);
	fclose(archivoSwap);
	memSwap = memSwap - TAMANIO_PAGINA; //Le resto al total de swap el archivo escrito en disco.
	printf("Swapping despues de crear el archivo: %ld\n", (memSwap));
	return elemSwap->marco;
}

void grabarArchivoSwap(int nroMarco, FILE* archivoSwap) {
	char * memoria = g_BaseMemoria;
	int tamanio = TAMANIO_PAGINA;
	//char* txtSwap =  malloc(tamanio);
	//memset(aux,'0', tamanio* sizeof(char));
	char*txtSwap = malloc(TAMANIO_PAGINA);
	memset(txtSwap, '0', tamanio * sizeof(char));
	memoria = memoria + ((nroMarco * TAMANIO_PAGINA));
	//while(tamanio--){
	memcpy(txtSwap, memoria, tamanio);
	/*
	 char* aux =  malloc(tamanio);
	 memset(aux,'0', tamanio* sizeof(char));
	 memcpy(aux, texto, strlen(texto)+1);
	 char * memoria = g_BaseMemoria;
	 memoria = memoria + (nroMarco*TAMANIO_PAGINA) + posicion;
	 while(tamanio--){
	 *memoria++ = *(aux++); //VERIFICAR QUE ESTO FUNCIONE
	 printf("%c",*(memoria-1));
	 }
	 //NO HACE FREE DE AUX
	 return 1;
	 */

	//}

	fwrite(txtSwap, sizeof(char), TAMANIO_PAGINA, archivoSwap);

}

t_algoritmoSust* sustitucionPag(char* algoritmo) {
	t_algoritmoSust* elemSwap;
	char* fifo = "FIFO";
	char* clockm = "CLOCKMODIFICADO";
	if (!string_equals_ignore_case(algoritmo, fifo)) {
		//Fifo
		elemSwap = list_remove(listaAlgoritmo, 0); //Elimino el primer elemento de la lista por Fifo
		list_add(listaMarcosLibres, marco_create(elemSwap->marco)); //Libero el marco
		t_segmento* segmentoSw = buscarListaPagina(elemSwap->pid,
				elemSwap->base); //Busco el segmento a swapear
		t_pagina* paginaSw = buscarPagina(segmentoSw, elemSwap->nroPagina); //Busco la pagina para cambiar el parametro swapping
		paginaSw->swapping = 1;
		paginaSw->marco = -1;
	} else if (!string_equals_ignore_case(algoritmo, clockm)) {
		//Clock Modificado

		int pauxiliar = puntero; //para ver cuando dio toda una vuelta a la lista de marcos

		bool _true1(void* swap) {
			return (((t_algoritmoSust *) swap)->bm == 0
					&& ((t_algoritmoSust *) swap)->bu == 0);
		} //Para Caso 1

		//Caso 1, busco pagina con BU=0 y BM=0
		elemSwap = list_find(listaAlgoritmo, _true1);

		//Falla Caso 1
		//Caso 2, busco BU=0 y BM=1 y voy cambiando BU en 1 a 0
		while (elemSwap == NULL ) {
			elemSwap = list_get(listaAlgoritmo, puntero);
			if (elemSwap->bm == 1 && elemSwap->bu == 0) {
				break;
			} else {
				elemSwap->bu = 0;
				puntero++;
				elemSwap = NULL;
				if (puntero == list_size(listaAlgoritmo)) {
					puntero = 0;
				}
			}
			if (puntero == pauxiliar && elemSwap == NULL ) {
				elemSwap = list_find(listaAlgoritmo, _true1); //Caso 2 fallo, vuelvo a hacer Caso 1
			}
		}

		puntero = elemSwap->marco; //el puntero pasa al numero de marco que se encontro para escribir
		elemSwap = list_remove(listaAlgoritmo, puntero); //Elimino el elemento donde se encuentra el puntero
		list_add(listaMarcosLibres, marco_create(elemSwap->marco)); //Libero el marco
		t_segmento* segmentoSw = buscarListaPagina(elemSwap->pid,
				elemSwap->base); //Busco el segmento a swapear
		t_pagina* paginaSw = buscarPagina(segmentoSw, elemSwap->nroPagina); //Busco la pagina para cambiar el parametro swapping
		paginaSw->swapping = 1;
		paginaSw->marco = -1;
	}

	return elemSwap;
}

int agregarAListaAlgoritmo(int marco, int nroPagina, int pid, int base,
		int modo) {
	t_algoritmoSust* elemento = create_elemento(marco, nroPagina, pid, base);
	if (modo == 1) {
		elemento->bm = 1;
	}
	list_add(listaAlgoritmo, elemento);
	return 1;
}

int grabarEnMemoria(int nroMarco, int posicion, int tamanio, char * texto) {
	char* aux = malloc(tamanio);
	int i = 0;
	memset(aux, ' ', tamanio); //Setea todo el aux a 0
	//printf("\n Es BASURA: %s",aux);
	//printf("ACA TERMINASTE\n");
	memcpy(aux, texto, tamanio); //Copia todo texto en el aux
	char * memoria = g_BaseMemoria;
	memoria = memoria + (nroMarco * TAMANIO_PAGINA) + posicion;
	while (tamanio--) {
		*(memoria++) = *(aux++); //VERIFICAR QUE ESTO FUNCIONE
		printf("%c",*(memoria-1));
	}
	return 1;
}

int escribirMemoria(int pid, int base, int pagina, int offset, int tamanio,
		char*texto) {
	t_segmento * tsegmento;
	t_pagina * tpagina;
	char * contenidoSwapping = malloc(TAMANIO_PAGINA);

	//Buscamos el pid y el segmento dentro de la tabla de segmentos
	tsegmento = buscarListaPagina(pid, base);
	if (tsegmento == NULL ) {
		log_error(logger,
				"El PID no existe o el segmento no existe, vuelva a intentarlo, muchas gracias, estamos trabajando para usted");
		return 0;

	}

	//Aca verificamos que lo que se va a escribir no sea mas que el tamanio del segmento
	if (tsegmento->tamanio < tamanio) {
		log_error(logger, "Segmentation Fault");
		return 0;
	}

	//Obtenemos la pagina de la lista de paginas
	tpagina = buscarPagina(tsegmento, pagina);
	if (tpagina == NULL ) {
		log_error(logger,
				"La pagina no existe en el segmento, vuelva a intentarlo.\n");
		return 0;
	}

	//Verificamos que desde el offset de la pagina se pueda escribir o que haya mas paginas
	if (((TAMANIO_PAGINA - offset)
			+ (cantidadPaginas(tpagina, tsegmento->lista_paginas)
					* TAMANIO_PAGINA)) < tamanio) {
		log_error(logger, "Segmentation Fault");
		return 0;
	}

	//Aca verificamos que la pagina no este swappeada
	if (tpagina->swapping == 1) {
		//ASIGNARLE UN MARCO LIBRE Y TRAER DE SWAPPING A MEMORIA
		while (!asignarMarcoLibre(tpagina, pid, base, 1)) {
			swappear(tpagina, pid, base);
			//log_error(logger,"No hay memoria disponible");
			//return 0;
		}
		//La funcion traerSwapping devuelve el contenido del archivo de Swap de la Pagina
		contenidoSwapping = (traerSwapping(tpagina, pid, base));

		/*if (contenidoSwapping = (traerSwapping(tpagina, pid, base))) {
		 log_error(logger, "Error Grave: No existe archivo de swapping");
		 return 0;
		 }*/
		if (!grabarEnMemoria(tpagina->marco, 0, TAMANIO_PAGINA, //offset 0 xq levanto todo el archivo
				contenidoSwapping)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al grabar en memoria principal");
			return 0;
		}
		//Una vez que levante el swapping grabo lo nuevo que quiero grabar
		if (!grabarEnMemoria(tpagina->marco, offset, TAMANIO_PAGINA, texto)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al grabar en memoria principal");
			return 0;
		}
	} else {
		if (tpagina->marco == -1) {
			//ASIGNARLE UN MARCO LIBRE
			while (!asignarMarcoLibre(tpagina, pid, base, 0)) {
				swappear(tpagina, pid, base);
				//log_error(logger,"No hay memoria disponible");
				//return 0;
			}
		}
		if (!grabarEnMemoria(tpagina->marco, offset, tamanio, texto)) {
			log_error(logger,
					"Error Grave: hubo un error inesperado al grabar en memoria");
		}
	}
	return 1;
}

void mostrarListaSegmentosYPaginas(int pid) {
	printf("       ID:%i  \n", pid);
	void _list_elements(t_segmento *seg) {
		if (seg->pid == pid) {
			printf("Segmento:%9i \n", seg->nroSegmento);
			mostrarListaPaginas(seg->lista_paginas);
		}
	}
	list_iterate(listaSegmentos, (void*) _list_elements);
}

void mostrarListaPaginas(t_list * lista) {
	printf("Paginas      Marco      Swapping \n");
	void _list_elements(t_pagina *pag) {
		printf("%9i %9i %9i   \n", pag->nroPagina, pag->marco, pag->swapping);
	}
	list_iterate(lista, (void*) _list_elements);
}

void mostrarListaMarcosLibres(t_list * lista) {
	printf("Marco   \n");
	void _list_elements(t_marco *marco) {
		printf("%9i \n", marco->nroMarco);
	}
	list_iterate(lista, (void*) _list_elements);
}

t_segmento *buscarListaPagina(int pid, int segmento) {
	bool _true(void* seg) {
		if (((t_segmento*) seg)->pid == pid
				&& ((t_segmento*) seg)->nroSegmento == segmento) {
			return 1;
		}
		return 0;
	}
	return (t_segmento*) list_find(listaSegmentos, _true);
}

t_pagina *buscarPagina(t_segmento* segmento, int pagina) {
	bool _true(void* pag) {
		return (((t_pagina *) pag)->nroPagina == pagina);
	}
	return (t_pagina*) list_find(segmento->lista_paginas, _true);
}

void mostrarListaSegmentosDe(int pid) {
	printf("       ID  Segmento    Tamaño  \n");
	void _list_elements(t_segmento *seg) {
		if (seg->pid == pid)
			printf("%9i %9i %9i   \n", seg->pid, seg->nroSegmento,
					seg->tamanio);
	}
	list_iterate(listaSegmentos, (void*) _list_elements);
}

int eliminarArchivoSwapping(int pid, int nroSegmento, int nroPagina) {
	char* archSwap = generarNombreArchivo(pid, nroSegmento, nroPagina);
	printf("Swapping antes de eliminar el archivo: %ld\n", (memSwap));
	remove(archSwap);

	memSwap = memSwap + TAMANIO_PAGINA; //Libero el espacio de Swapping del archivo que se elimino
	printf("Swapping despues de eliminar el archivo: %ld\n", (memSwap));
	return 1;
}

void mostrarListaSegmentos() {
	printf("       ID  Segmento    Tamaño  \n");
	void _list_elements(t_segmento *seg) {
		printf("%9i %9i %9i   \n", seg->pid, seg->nroSegmento, seg->tamanio);
	}

	list_iterate(listaSegmentos, (void*) _list_elements);
}

void liberarYEliminarMarcosOSwapping(t_segmento *segmento) {
	void _list_elements(t_pagina *pag) {
		if (pag->swapping == 1) {
			eliminarArchivoSwapping(segmento->pid, segmento->nroSegmento,
					pag->nroPagina);
		} else if (pag->marco != -1) {
			list_add(listaMarcosLibres, marco_create(pag->marco));
			pag->marco = -1;
		}
	}

	list_iterate(segmento->lista_paginas, (void*) _list_elements);
}

int destruirSegmento(int pid, int base) {
	//Me fijo si existe un segmento con el pid del parametro y la direccion base
	bool _true(void* seg) {
		if (((t_segmento*) seg)->pid == pid
				&& ((t_segmento*) seg)->nroSegmento == base) {
			return 1;
		}
		return 0;
	}

	//Busco el segmento que cumple con las dos condiciones
	t_segmento * segmentoDestruir = buscarListaPagina(pid, base);
	if (segmentoDestruir == NULL ) {
		log_error(logger, "El segmento y/o PID no existen");
		return 0;
	}
	//Libero los marcos asignados o elimino los archivos de swapping de las paginas
	liberarYEliminarMarcosOSwapping(segmentoDestruir);
	//Libero la lista de paginas
	list_destroy(segmentoDestruir->lista_paginas);
	//Elimino el segmento de la lista de segmentos
	list_remove_by_condition(listaSegmentos, _true);
	//Libero SegmentoDestruir
	free(segmentoDestruir);
	return 1;
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
		Error(
				"Ocurrio un error al intentar recibir datos desde uno de los clientes. Socket: %d",
				socket);
	}

	log_trace(logger, "RECIBO DATOS. socket: %d. buffer: %s tamanio:%i", socket,
			(char*) bufferAux, strlen(bufferAux));
	return bufferAux; //--> buffer apunta al lugar de memoria que tiene el mensaje completo completo.
}

int EnviarDatos(int socket, char *buffer, int cantidadDeBytesAEnviar) {
// Retardo antes de contestar una solicitud (Se solicita en enunciado de TP)
	//sleep(g_Retardo / 1000);

	int bytecount;

	//printf("CantidadBytesAEnviar:%i\n",cantidadDeBytesAEnviar);

	if ((bytecount = send(socket, buffer, cantidadDeBytesAEnviar, 0)) == -1)
		Error("No puedo enviar información a al clientes. Socket: %d", socket);

	//Traza("ENVIO datos. socket: %d. buffer: %s", socket, (char*) buffer);
	char * bufferLogueo = malloc(cantidadDeBytesAEnviar+1);
	bufferLogueo[cantidadDeBytesAEnviar] = '\0';
	memcpy(bufferLogueo,buffer,cantidadDeBytesAEnviar);
	log_info(logger, "ENVIO DATOS. socket: %d. Buffer:%s ",socket,
			(char*) bufferLogueo);

	return bytecount;
}

int chartToInt(char x) {
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

int posicionDeBufferAInt(char* buffer, int posicion) {
	int logitudBuffer = 0;
	logitudBuffer = strlen(buffer);

	if (logitudBuffer <= posicion)
		return 0;
	else
		return chartToInt(buffer[posicion]);
}

int ObtenerComandoMSJ(char* buffer) {
//Hay que obtener el comando dado el buffer.
//El comando está dado por el primer caracter, que tiene que ser un número.
	return posicionDeBufferAInt(buffer, 0);
}

void CerrarSocket(int socket) {
	close(socket);
	//Traza("SOCKET SE CIERRA: (%d).", socket);
	log_trace(logger, "SOCKET SE CIERRA: (%d).", socket);
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

char* RespuestaClienteOk(char *buffer) {
	/*int tamanio = sizeof(char) * 2;
	 buffer = realloc(buffer, tamanio * sizeof(char));
	 memset(buffer, 0, tamanio * sizeof(char));
	 sprintf(buffer, "%s", "1");*/

	if (buffer != NULL )
		free(buffer);

	buffer = string_new();
	string_append(&buffer, "1");

	return buffer;
}

char* RespuestaClienteError(char *buffer, char *msj) {
	//int cantidadBytesBuffer = strlen(buffer);
	if (buffer != NULL )
		free(buffer);
	buffer = string_new();
	string_append(&buffer, msj);

	/*	int tamanio = (strlen(msj) + 1) * sizeof(char);
	 buffer = realloc(buffer, tamanio * sizeof(char));
	 memset(buffer, 0, tamanio * sizeof(char));
	 sprintf(buffer, "%s%s", "0", msj);*/
	//free(msj);
	return buffer;
}

void SetearErrorGlobal(const char* mensaje, ...) {
	va_list arguments;
	va_start(arguments, mensaje);
	if (g_MensajeError != NULL )
		// NMR COMENTADO POR ERROR A ULTIMO MOMENTO	free(g_MensajeError);
		g_MensajeError = string_from_vformat(mensaje, arguments);
	va_end(arguments);
}

int obtenerPaginayOffset(int desplazamiento, int * pagina, int *offset) {
	*pagina = desplazamiento / TAMANIO_PAGINA;
	*offset = desplazamiento % TAMANIO_PAGINA;
	return 1;
}

char* ComandoEscribirMemoria(char *buffer, int *idProg, int tipoCliente,
		int socket, int *longitud) {
	// Graba en la memoria
	// Formato del mensaje: CABBBBCDDDDFGGGGHIIIOOOOOOOOO....
	// C = Codigo de mensaje ( = 2)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el segmento
	// DDDD = El numero del segmento (hasta 4096)
	// F = Cantidad de digitos que tiene el desplazamiento
	// GGGG = Numero del desplazamiento(pagina y desplazamiento en la misma)
	// H = Cantidad de digitos que tiene la cantidad de caracteres que se quieren leer
	// I = Cantidad de caracteres que se quieren leer
	// OOOOOOOOO = Cantidad de caracteres que se quieren leer

	// Retorna: 1 + Bytes si se leyo ok
	//			0 + mensaje error si no se pudo leer

	int ok = 0;
	int bytesRecibidos;

	int base = 0;
	int pagina = 0;
	int desplazamiento = 0;
	int longitudBuffer = 0;
	int offset = 0;

	int cantidadDigitosPID = 0;
	int cantidadDigitosBase = 0;
	int cantidadDigitosDesplazamiento = 0;
	int cantidadDigitoslongitudBuffer = 0;
	int posicion = 1;
	// Me fijo cuantos digitos tiene el numero de PID
	cantidadDigitosPID = posicionDeBufferAInt(buffer, posicion);
	// Grabo el PID del proceso
	posicion++;

	*idProg = subCadenaAInt(buffer, posicion, cantidadDigitosPID);

	log_trace(logger, "COMANDO Escribir Memoria. Id prog: %d", *idProg);
	posicion = posicion + cantidadDigitosPID;
	// Me fijo cuantos digitos tiene el tamaño de la base
	cantidadDigitosBase = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidadigitos Base: %d\n", cantidadDigitosBase);

	posicion++;
	// Grabo la base que me pasan como cadena
	base = subCadenaAInt(buffer, posicion, cantidadDigitosBase);
	printf("CantidaddBase: %d\n", base);
	posicion = posicion + cantidadDigitosBase;

	// Me fijo cuantos digitos tiene el tamaño del desplazamiento
	cantidadDigitosDesplazamiento = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidadd digitos desplazamiento: %d\n",
			cantidadDigitosDesplazamiento);
	// Grabo el valor del desplazamiento

	posicion++;

	desplazamiento = subCadenaAInt(buffer, posicion,
			cantidadDigitosDesplazamiento);

	if (!obtenerPaginayOffset(desplazamiento, &pagina, &offset)) {
		//CONTEMPLAR ERROR
		printf("ACA HAY ERROR\n");
	}
	printf("Cantidad desplazamiento: %d\n", desplazamiento);
	posicion = posicion + cantidadDigitosDesplazamiento;
	cantidadDigitoslongitudBuffer = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidad long buffer: %d\n", cantidadDigitoslongitudBuffer);

	posicion++;
	longitudBuffer = subCadenaAInt(buffer, posicion,
			cantidadDigitoslongitudBuffer);
	//printf("Longitud de buffer: %d\n", longitudBuffer);
	posicion = posicion + cantidadDigitoslongitudBuffer;
	*longitud = longitudBuffer;
	buffer = RecibirDatos(socket, buffer, &bytesRecibidos);
	int z=0;
	printf("\nAca arranca:");
	while(z<bytesRecibidos){
		printf("%c",buffer[z++]);
	}
	printf(":Aca termina\n:");
	char * aux = buffer;
	char* escritura = malloc(longitudBuffer * sizeof(char)+1);
	memset(escritura, ' ', longitudBuffer);
	memcpy(escritura, aux,longitudBuffer + 1);
	printf("Escritura: %s\n", escritura);
	sem_wait(&semaforoAccesoMemoria);
	ok = escribirMemoria(*idProg, base, pagina, offset, longitudBuffer,
			escritura);
	sem_post(&semaforoAccesoMemoria);


	printf("Memoria Escrita: %d\n", longitudBuffer);
	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(longitudBuffer+1);
		memcpy(buffer,escritura,longitudBuffer+1);
		leerBinario(buffer);
		//buffer = string_new();
		//string_append(&buffer, escritura);

		/*int tamanio = (longitudBuffer + 1) * sizeof(char);
		 buffer = realloc(buffer, tamanio * sizeof(char));
		 memset(buffer, 0, tamanio * sizeof(char));
		 sprintf(buffer, "%s%s", "1", lectura);*/
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal(
				"ERROR GRABAR MEMORIA. %s. Id programa: %d, base: %d, pagina: %d, desplazamiento: %d, longitud buffer: %d",
				stringErrorAux, idProg, base, pagina, desplazamiento,
				longitudBuffer);
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(2);
		string_append(&buffer, string_itoa(-1));
		*longitud= strlen(buffer);
		printf("LA LONGITUD DE BUFFER: %i\n",longitud);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}

	if (escritura != NULL )
		free(escritura);
	return buffer;
}

char* ComandoCrearSegmento(char *buffer, int* idProg, int tipo_Cliente,int* longitud) {
	// Crear Segmento en la memoria
	// Formato del mensaje: CABBBBCDDDDDDD
	// C = Codigo de mensaje ( = 5)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el tamaño del segmento a crear
	// DDDDDDDD = El tamaño maximo de un segmento (hasta 9999999)
	// Retorna: 1 + Bytes si se leyo ok
	//			0 + mensaje error si no se pudo leer

	int ok = 0;

	int tamanio = 0;

	int cantidadDigitosPID = 0;
	int cantidadDigitosTamanio = 0;

	// Me fijo cuantos digitos tiene el numero de PID
	cantidadDigitosPID = posicionDeBufferAInt(buffer, 1);
	// Grabo el PID del proceso
	*idProg = subCadenaAInt(buffer, 2, cantidadDigitosPID);

	log_trace(logger, "COMANDO Crear Segmento en Memoria. Id prog: %d",
			*idProg);

	// Me fijo cuantos digitos tiene el tamaño del segmento
	cantidadDigitosTamanio = posicionDeBufferAInt(buffer,
			2 + cantidadDigitosPID);
	// Grabo el tamanio que me pasan como cadena
	tamanio = subCadenaAInt(buffer, 2 + cantidadDigitosPID + 1,
			cantidadDigitosTamanio);
	sem_wait(&semaforoAccesoMemoria);
	ok = crearSegmento(*idProg, tamanio);
	sem_post(&semaforoAccesoMemoria);
	printf("Direccion Base del Segmento Creado: %i\n", ok);

	if (buffer != NULL )
		free(buffer);
	buffer = string_new();
	//string_append(&buffer, "5");
	string_append(&buffer, string_itoa(ok));
	*longitud=strlen(buffer);
	return buffer;
}

char* ComandoDestruirSegmento(char* buffer, int tipo_Cliente) {
	// Destruir segmento de la memoria
	// Formato del mensaje: CABBBBCDDDD....
	// C = Codigo de mensaje ( = 6)
	// A = Cantidad de digitos que tiene el pid
	// BBBB = Pid del proceso (hasta 9999)
	// C = Cantidad de digitos que tiene el segmento
	// DDDD = El numero del segmento (hasta 4096)

	// Retorna: 1 + si se ejecuto correctamente
	//			0 + mensaje error si no se pudo leer

	int ok = 0;

	int base = 0;

	int cantidadDigitosPID = 0;
	int cantidadDigitosBase = 0;
	int posicion = 1;
	int idProg;
	// Me fijo cuantos digitos tiene el numero de PID
	cantidadDigitosPID = posicionDeBufferAInt(buffer, posicion);
	// Grabo el PID del proceso
	posicion++;

	idProg = subCadenaAInt(buffer, posicion, cantidadDigitosPID);
	log_trace(logger, "COMANDO Leer Memoria. Id prog: %d", idProg);
	posicion = posicion + cantidadDigitosPID;
	// Me fijo cuantos digitos tiene el tamaño de la base
	cantidadDigitosBase = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidadigitos Base: %d\n", cantidadDigitosBase);

	posicion++;
	// Grabo la base que me pasan como cadena
	base = subCadenaAInt(buffer, posicion, cantidadDigitosBase);
	printf("CantidaddBase: %d\n", base);
	posicion = posicion + cantidadDigitosBase;
	printf("EL PID:%i Y BASE:%i",idProg,base);
	sem_wait(&semaforoAccesoMemoria);
	ok = destruirSegmento(idProg, base);
	sem_post(&semaforoAccesoMemoria);
	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();
		//string_append(&buffer, "1");
		string_append(&buffer, string_itoa(ok));
		/*int tamanio = (longitudBuffer + 1) * sizeof(char);
		 buffer = realloc(buffer, tamanio * sizeof(char));
		 memset(buffer, 0, tamanio * sizeof(char));
		 sprintf(buffer, "%s%s", "1", lectura);*/
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal(
				"ERROR LEER MEMORIA. %s. Id programa: %d, base: %d, pagina: %d, desplazamiento: %d, longitud buffer: %d",
				stringErrorAux, &idProg, base);
		buffer = RespuestaClienteError(buffer, g_MensajeError);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}
	return buffer;
}

char* ComandoLeerMemoria(char *buffer, int *idProg, int tipoCliente,int *longitud) {
// Lee la memoria
// Formato del mensaje: CABBBBCDDDDFGGGGHHHHHH....
// C = Codigo de mensaje ( = 1)
// A = Cantidad de digitos que tiene el pid
// BBBB = Pid del proceso (hasta 9999)
// C = Cantidad de digitos que tiene el segmento
// DDDD = El numero del segmento (hasta 4096)
// E = Cantidad de digitos que tiene el desplazamiento
// F = Desplazamiento
// G = Cantidad de digitos que tiene la cantidad de caracteres que se quieren leer
// H = Cantidad de caracteres que se van a leer

// Retorna: Lo solicitado en la lectura
//			0 + mensaje error si no se pudo leer

	int ok = 0;

	int base = 0;
	int pagina = 0;
	int desplazamiento = 0;
	int longitudBuffer = 0;
	int offset = 0;

	int cantidadDigitosPID = 0;
	int cantidadDigitosBase = 0;
	int cantidadDigitosPagina = 0;
	int cantidadDigitosDesplazamiento = 0;
	int cantidadDigitoslongitudBuffer = 0;
	int posicion = 1;
	// Me fijo cuantos digitos tiene el numero de PID
	cantidadDigitosPID = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidad Digitos del Pid:%d\n", cantidadDigitosPID);
	// Grabo el PID del proceso
	posicion++;

	*idProg = subCadenaAInt(buffer, posicion, cantidadDigitosPID);
	printf("Pid:%d\n", *idProg);
	log_trace(logger, "COMANDO Leer Memoria. Id prog: %d", *idProg);
	posicion = posicion + cantidadDigitosPID;
	// Me fijo cuantos digitos tiene el tamaño de la base
	cantidadDigitosBase = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidadigitos Base: %d\n", cantidadDigitosBase);

	posicion++;
	// Grabo la base que me pasan como cadena
	base = subCadenaAInt(buffer, posicion, cantidadDigitosBase);
	printf("Base: %d\n", base);
	posicion = posicion + cantidadDigitosBase;

	// Me fijo cuantos digitos tiene el tamaño del desplazamiento
	cantidadDigitosDesplazamiento = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidadd digitos desplazamiento: %d\n",
			cantidadDigitosDesplazamiento);

	// Grabo el valor del desplazamiento
	posicion++;
	desplazamiento = subCadenaAInt(buffer, posicion,
			cantidadDigitosDesplazamiento);

	if (!obtenerPaginayOffset(desplazamiento, &pagina, &offset)) {
		//CONTEMPLAR ERROR
		printf("ACA HAY ERROR\n");
	}
	printf("Cantidad desplazamiento: %d\n", desplazamiento);
	posicion = posicion + cantidadDigitosDesplazamiento;
	cantidadDigitoslongitudBuffer = posicionDeBufferAInt(buffer, posicion);
	printf("Cantidad long buffer: %d\n", cantidadDigitoslongitudBuffer);

	posicion++;
	longitudBuffer = subCadenaAInt(buffer, posicion,
			cantidadDigitoslongitudBuffer);
	printf("Longitud de buffer: %d\n", longitudBuffer);
	*longitud = longitudBuffer;
	posicion = posicion + cantidadDigitoslongitudBuffer;

	char* lectura = malloc(longitudBuffer + 1);

	printf(
			"PARAMETROS: PID:%d, Base:%d, Pagina%d, Offset%d, LongitudBuffer:%d\n",
			*idProg, base, pagina, offset, longitudBuffer);
	sem_wait(&semaforoAccesoMemoria);
	ok = leerMemoria(*idProg, base, pagina, offset, longitudBuffer, lectura);
	sem_post(&semaforoAccesoMemoria);
	//printf("\nESTE ES EL POSTA POSTA:");

	//lectura[longitudBuffer] = '\0';

	//printf("LECTURA:%s \n", lectura);

	if (ok) {
		if (buffer != NULL )
			free(buffer);
		buffer = malloc(longitudBuffer+1);
		memcpy(buffer,lectura,longitudBuffer+1);
		leerBinario(buffer);
		/*int tamanio = (longitudBuffer + 1) * sizeof(char);
		 buffer = realloc(buffer, tamanio * sizeof(char));
		 memset(buffer, 0, tamanio * sizeof(char));
		 sprintf(buffer, "%s%s", "1", lectura);*/
	} else {
		char* stringErrorAux = string_new();
		string_append(&stringErrorAux, g_MensajeError);
		SetearErrorGlobal(
				"ERROR LEER MEMORIA. %s. Id programa: %d, base: %d, pagina: %d, desplazamiento: %d, longitud buffer: %d",
				stringErrorAux, idProg, base, pagina, desplazamiento,
				longitudBuffer);
		if (buffer != NULL )
					free(buffer);
		buffer = malloc(3);
		string_append(&buffer, "-1");
		*longitud= strlen(buffer);
		if (stringErrorAux != NULL )
			free(stringErrorAux);
	}

	if (lectura != NULL )
		free(lectura);
	return buffer;
}

int AtiendeCliente(void * arg) {
	int socket = (int) arg;

//Es el ID del programa con el que está trabajando actualmente el HILO.
//Nos es de gran utilidad para controlar los permisos de acceso (lectura/escritura) del programa.
//(en otras palabras que no se pase de vivo y quiera acceder a una posicion de memoria que no le corresponde.)
	int id_Programa = 0;
	int tipo_Cliente = 0;
	int longitudBuffer;
	printf("ENTRE");

// Es el encabezado del mensaje. Nos dice que acción se le está solicitando a la msp
	int tipo_mensaje = 0;

// Dentro del buffer se guarda el mensaje recibido por el cliente.
	char* buffer;
	buffer = malloc(1000000 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.

// Cantidad de bytes recibidos.
	int bytesRecibidos;

// La variable fin se usa cuando el cliente quiere cerrar la conexion: chau chau!
	int desconexionCliente = 0;

// Código de salida por defecto
	int code = 0;

	while ((!desconexionCliente) & g_Ejecutando) {
		//	buffer = realloc(buffer, 1 * sizeof(char)); //-> de entrada lo instanciamos en 1 byte, el tamaño será dinamico y dependerá del tamaño del mensaje.
		if (buffer != NULL )
			free(buffer);
		buffer = string_new();
		//Recibimos los datos del cliente
		buffer = RecibirDatos(socket, buffer, &bytesRecibidos);

		if (bytesRecibidos > 0) {
			//Analisamos que peticion nos está haciendo (obtenemos el comando)
			tipo_mensaje = ObtenerComandoMSJ(buffer);

			//Evaluamos los comandos
			switch (tipo_mensaje) {
			case MSJ_LEER_MEMORIA:
				buffer = ComandoLeerMemoria(buffer, &id_Programa, tipo_Cliente, &longitudBuffer);
				break;
			case MSJ_ESCRIBIR_MEMORIA:
				buffer = ComandoEscribirMemoria(buffer, &id_Programa,
						tipo_Cliente, socket,&longitudBuffer);
				break;
			case MSJ_HANDSHAKE:
				//	buffer = ComandoHandShake(buffer, &tipo_Cliente);
				break;
			case MSJ_CAMBIO_PROCESO:
				//	buffer = ComandoCambioProceso(buffer, &id_Programa);
				break;
			case MSJ_CREAR_SEGMENTO:
				buffer = ComandoCrearSegmento(buffer, &id_Programa,
						tipo_Cliente,&longitudBuffer);
				break;
			case MSJ_DESTRUIR_SEGMENTO:
				buffer = ComandoDestruirSegmento(buffer, tipo_Cliente);
				longitudBuffer = 1;
				break;
			default:
				buffer = RespuestaClienteError(buffer,
						"El ingresado no es un comando válido\n");
				longitudBuffer=strlen(buffer);
				break;
			}
			printf("\nRespuesta: %s\n",buffer);
			// Enviamos datos al cliente.
			EnviarDatos(socket, buffer,longitudBuffer);
		} else
			desconexionCliente = 1;

	}

	CerrarSocket(socket);

	return code;
}

void HiloOrquestadorDeConexiones() {

	int socket_host;
	struct sockaddr_in client_addr;
	struct sockaddr_in my_addr;
	int yes = 1;
	socklen_t size_addr = 0;

	socket_host = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_host == -1)
		ErrorFatal(
				"No se pudo inicializar el socket que escucha a los clientes");

	if (setsockopt(socket_host, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		ErrorFatal("Error al hacer el 'setsockopt'");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(g_Puerto);
	my_addr.sin_addr.s_addr = htons(INADDR_ANY );
	memset(&(my_addr.sin_zero), '\0', 8 * sizeof(char));

	if (bind(socket_host, (struct sockaddr*) &my_addr, sizeof(my_addr)) == -1)
		ErrorFatal("Error al hacer el Bind. El puerto está en uso");

	if (listen(socket_host, 10) == -1) // el "10" es el tamaño de la cola de conexiones.
		ErrorFatal(
				"Error al hacer el Listen. No se pudo escuchar en el puerto especificado");

	//Traza("El socket está listo para recibir conexiones. Numero de socket: %d, puerto: %d", socket_host, g_Puerto);
	log_trace(logger,
			"SOCKET LISTO PARA RECBIR CONEXIONES. Numero de socket: %d, puerto: %d",
			socket_host, g_Puerto);

	while (g_Ejecutando) {
		int socket_client;

		size_addr = sizeof(struct sockaddr_in);

		if ((socket_client = accept(socket_host,
				(struct sockaddr *) &client_addr, &size_addr)) != -1) {
			//Traza("Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, socket_client);
			log_trace(logger,
					"NUEVA CONEXION ENTRANTE. Se ha conectado el cliente (%s) por el puerto (%d). El número de socket del cliente es: %d",
					inet_ntoa(client_addr.sin_addr), client_addr.sin_port,
					socket_client);
			// Aca hay que crear un nuevo hilo, que será el encargado de atender al cliente
			pthread_t hNuevoCliente;
			pthread_create(&hNuevoCliente, NULL, (void*) AtiendeCliente,
					(void *) socket_client);
		} else {
			Error("ERROR AL ACEPTAR LA CONEXIÓN DE UN CLIENTE");
		}
	}
	CerrarSocket(socket_host);
}

