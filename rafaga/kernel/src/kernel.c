#include "kernel.h"
#include <commons/collections/list.h>

int socket_msp;


int main() {

	sem_init(&semaforoTCB, 0, 0);
	sem_init(&semaforoCPU, 0, 0);
	sem_init(&sem_join, 0, 0);

//	pthread_mutex_init(&mutexEnviar= malloc (sizeof(int));Datos, NULL);
//	pthread_mutex_init(&mutexRecibirDatos, NULL);

	loggerKernel = log_create("kernel.log", "KERNEL", 0, LOG_LEVEL_DEBUG); //Creo el Log de Kernel
	leerConfigKernel();

	socket_msp = conectarMSP();

	//CREAMOS LAS LISTAS DEL PLANIFICADOR
	listaNew = list_create();
	listaReady = list_create();
	listaExec = list_create();
	listaBlock = list_create();
	listaExit = list_create();
	//Lista con las estructuras tcb
	listaTcb = list_create();
	//CREAMOS LA LISTA QUE GUARDA LOS CONTADORES DE LOS TID
	listaTcb = list_create();

	//Lista para las llamadas al sistema
	listaLlamadas = list_create();

	//lista que guarda los tcb que estan esperando un recurso y el id del mismo
	listaBlockXRecursos = list_create();

	listaCpuLibre = list_create();

//	structParametros *parametros = malloc(sizeof(structParametros));

	pthread_t hiloPlanificador;
	pthread_create(&hiloPlanificador, NULL, (void*) planificador, NULL);

	crearMultiplexor();


	pthread_join(hiloPlanificador, NULL );


	return (EXIT_SUCCESS);
}

int charToInt(char x) {
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

void atenderConsola(int socket_consola, char buffer[]) {
	//ABCCCC
	//A (CONSOLA = C)
	//B CANTIDAD DE DIGITOS A LEER TAMANIO
	//CCCC TAMANIO PROGRAMA A RECIBIR
	int cantDigitosTamanio = charToInt(buffer[1]);
	int size_prog = atoi(string_substring(buffer, 2, cantDigitosTamanio));
	log_trace(loggerKernel,
			"Buffer recibido por consola: %s. Tamanio programa: %d", buffer,
			size_prog);
	if (enviarDatos(socket_consola, MSJ_CONFIRMACION) == -1) {
		log_info(loggerKernel,
				"ERROR FATAL: no se pudo enviar confirmación de conexión con la consola.");
		exit(1);
	}
	log_info(loggerKernel, "Conexion con consola aceptada.");

	char* programa;
	programa = malloc(BUFFERSIZE * sizeof(char));

	int bytesRecibidos;
	if ((bytesRecibidos = recibirDatos(socket_consola, programa)) == 0) {
		log_info(loggerKernel, "ERROR FATAL: recepcion programa");
		exit(1);
	}
	if (bytesRecibidos != size_prog) {
		log_info(loggerKernel, "ERROR FATAL: se recibieron %d de %d",
				bytesRecibidos, size_prog);
		exit(1);
	}
	printf("El programa: %s\n",programa);

	if (enviarDatos(socket_consola, MSJ_CONFIRMACION) == -1) {
		log_info(loggerKernel,
				"ERROR FATAL: no se pudo enviar confirmación de conexión con la consola.");
		exit(1);
	}
	log_info(loggerKernel, "Se recibio el programa correctamente");

	loader(socket_consola, size_prog, programa);

	free(programa);

}


void interrupcion(int socket_cpu, char* buffer) {

	t_hilo* tcb = malloc(sizeof(t_hilo));

	//le sacamos la i
	strcpy(buffer, string_substring_from(buffer, 1));

	tcb = stringATcb(buffer);

	int lengthTCB = string_length(tcbAStringSinQuantum(tcb));

//	int direccion = atoi(string_substring_from(buffer, lengthTCB-1));
	int direccion = atoi(string_substring_from(buffer, lengthTCB));

	//log syscall interrupcion
//	instruccion_protegida("Interrupcion", tcb);

//	char* buffer2 = string_new();
//	buffer2 = string_substring_from(buffer, 1);
//	tcb = stringATcb(buffer2);
//	char* tcb_string = string_new();
//	strcpy(tcb_string, tcbAString(tcb));
//
//	int l = string_length(tcb_string);
//
//	int memoria = atoi(string_substring_from(buffer2, l - 6));
 	eliminarTCB_ListaPorTid(listaExec,tcb->tid);
 	list_add(listaBlock, tcb);
	structLlamadas* llamadaSist = malloc(sizeof(structLlamadas));
	llamadaSist->hilo = tcb;
	llamadaSist->dir_memoria = direccion;
	list_add(listaLlamadas, llamadaSist);
//	sem_post(&semaforoTCB);

	ejecutarLlamadaAlSist();
	//	free(tcb); //FIXME : VER
//	free(llamadaSist); // FIXME : VER
}

//Busca el tcb en modo kernel
bool _condicionKernelMode(void* tcb) {
	if (((t_hilo*) tcb)->kernel_mode == 1) {
		return 1;
	}
	return 0;
 }

void* buscarTCB_Lista(t_list* lista, t_hilo* tcb) {
	//Busco un tcb en una lista
	bool _condicionTcbIgual(void* tcb) {
		if ((t_hilo*) tcb == tcb) {
			return 1;
		}
			return 0;
		}

	return list_find(lista, _condicionTcbIgual);

}

//busca un Tcb por tid en una lista parametro
t_hilo* buscarTcbPorTid(t_list* lista, int tid_a_buscar){
	bool _condicionSocket(void* tcb){
		if (((t_hilo*) tcb)->tid == tid_a_buscar) {
					return 1;
				}
					return 0;
				}

	t_hilo* tcb_encontrado = malloc(sizeof(t_hilo));
	tcb_encontrado = list_find(lista, _condicionSocket);
	return tcb_encontrado;
}

//busca un Tcb por pid en una lista parametro
t_hilo* buscarTcbPorPid(t_list* lista, int pid_a_buscar){
	bool _condicionSocket(void* tcb){
		if (((t_hilo*) tcb)->pid == pid_a_buscar) {
					return 1;
				}
					return 0;
				}

	t_hilo* tcb_encontrado = malloc(sizeof(t_hilo));
	tcb_encontrado = list_find(lista, _condicionSocket);
	return tcb_encontrado;
}

int buscarSocketConsolaPorPid(int pid_a_buscar) {
	bool _condicionSocket(void* tcb) {
		if (((t_hilo*) tcb)->pid == pid_a_buscar) {
			return 1;
		}
		return 0;
	}

	structTcb* tcb_socket = malloc(sizeof(structTcb));
	tcb_socket = list_find(listaTcb, _condicionSocket);
	int socket_consola = tcb_socket->socket_consola;
//	free(tcb_socket); //FIXME: VER
	return socket_consola;
}

int buscarSocketConsolaPorTid(int tid_a_buscar){
	bool _condicionSocket(void* tcb){
		if (((t_hilo*) tcb)->tid == tid_a_buscar) {
					return 1;
				}
					return 0;
				}

	structTcb* tcb_socket = malloc(sizeof(structTcb));
	tcb_socket = list_find(listaTcb, _condicionSocket);
	int socket_consola = tcb_socket->socket_consola;
	//free(tcb_socket); //FIXME: VER
	return socket_consola;
}

bool siempreTrue(){
	return 1;
};

void ejecutarLlamadaAlSist(){

	int countLlamadas = list_count_satisfying(listaLlamadas, siempreTrue);

	if(countLlamadas > 0){

		if (list_find(listaBlock, _condicionKernelMode) != NULL ) {
			t_hilo* tcb_kernel = list_find(listaBlock, _condicionKernelMode);
			structLlamadas* llamada = malloc(sizeof(structLlamadas));
//			llamada = list_get(listaLlamadas, 0);
			llamada = list_remove(listaLlamadas,0);
			t_hilo* tcb = llamada->hilo;

			tcb_kernel->pid = tcb->pid;
			tcb_kernel->registros[0] = tcb->registros[0];
			tcb_kernel->registros[1] = tcb->registros[1];
			tcb_kernel->registros[2] = tcb->registros[2];
			tcb_kernel->registros[3] = tcb->registros[3];
			tcb_kernel->registros[4] = tcb->registros[4];

//			structLlamadas* llamada = malloc(sizeof(structLlamadas));
//			llamada = buscarTCB_Lista(listaLlamadas, tcb);

			tcb_kernel->puntero_instruccion = llamada->dir_memoria;
			eliminarTCB_ListaPorTid(listaBlock,tcb_kernel->tid);
			list_add_in_index(listaReady, 0, tcb_kernel);
			sem_post(&semaforoTCB);
			//free(llamada); //FIXME: VER
		}
	}

}

void finalizarLlamadaAlSist(t_hilo* tcb_km){
	t_hilo* tcb = malloc(sizeof(t_hilo));
	tcb = buscarTcbPorPid(listaBlock, tcb_km->pid);
	tcb->registros[0] = tcb_km->registros[0];
	tcb->registros[1] = tcb_km->registros[1];
	tcb->registros[2] = tcb_km->registros[2];
	tcb->registros[3] = tcb_km->registros[3];
	tcb->registros[4] = tcb_km->registros[4];

	tcb_km->pid = 1; //vuelve a tener su pid

	list_add(listaBlock, tcb_km);
	list_add(listaReady, tcb);
	sem_post(&semaforoTCB);
	eliminarTCB_ListaPorTid(listaExec, tcb_km->tid);
	eliminarTCB_ListaPorTid(listaBlock,tcb->tid);
	//free(tcb);//FIXME: VER
}

char* tcbAStringSinQuantum(t_hilo* tcbAEnviar) {

	int cant_digitos_pid = calcularCantDigitos(tcbAEnviar->pid);
	int cant_digitos_tid = calcularCantDigitos(tcbAEnviar->tid);
	int cant_digitos_base_segmento = calcularCantDigitos(
			tcbAEnviar->segmento_codigo);
	int cant_digitos_base_segmento_size = calcularCantDigitos(
			tcbAEnviar->segmento_codigo_size);
	int cant_digitos_puntero_instruccion = calcularCantDigitos(
			tcbAEnviar->puntero_instruccion);
	int cant_digitos_base_stack = calcularCantDigitos(tcbAEnviar->base_stack);
	int cant_digitos_puntero_stack = calcularCantDigitos(
			tcbAEnviar->cursor_stack);

	int cant_digitos_a = calcularCantDigitos(tcbAEnviar->registros[0]);
	int cant_digitos_b = calcularCantDigitos(tcbAEnviar->registros[1]);
	int cant_digitos_c = calcularCantDigitos(tcbAEnviar->registros[2]);
	int cant_digitos_d = calcularCantDigitos(tcbAEnviar->registros[3]);
	int cant_digitos_e = calcularCantDigitos(tcbAEnviar->registros[4]);

	char* buffer;
	buffer = string_new();

	string_append(&buffer, string_itoa(cant_digitos_pid));
	string_append(&buffer, string_itoa(tcbAEnviar->pid));
	string_append(&buffer, string_itoa(cant_digitos_tid));
	string_append(&buffer, string_itoa(tcbAEnviar->tid));
	string_append(&buffer, string_itoa(tcbAEnviar->kernel_mode));
	string_append(&buffer, string_itoa(cant_digitos_base_segmento));
	string_append(&buffer, string_itoa(tcbAEnviar->segmento_codigo));
	string_append(&buffer, string_itoa(cant_digitos_base_segmento_size));
	string_append(&buffer, string_itoa(tcbAEnviar->segmento_codigo_size));
	string_append(&buffer, string_itoa(cant_digitos_puntero_instruccion));
	string_append(&buffer, string_itoa(tcbAEnviar->puntero_instruccion));
	string_append(&buffer, string_itoa(cant_digitos_base_stack));
	string_append(&buffer, string_itoa(tcbAEnviar->base_stack));
	string_append(&buffer, string_itoa(cant_digitos_puntero_stack));
	string_append(&buffer, string_itoa(tcbAEnviar->cursor_stack));

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


char* tcbAString(t_hilo* tcbAEnviar) {
	//FORMATO A ENVIAR: ABBBBCDDDDEF...
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
	//P : CANTIDAD DE DIGITOS QUANTUM
	//QQ: QUANTUM

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
	int cant_digitos_base_segmento = calcularCantDigitos(
			tcbAEnviar->segmento_codigo);
	int cant_digitos_base_segmento_size = calcularCantDigitos(
			tcbAEnviar->segmento_codigo_size);
	int cant_digitos_puntero_instruccion = calcularCantDigitos(
			tcbAEnviar->puntero_instruccion);
	int cant_digitos_base_stack = calcularCantDigitos(tcbAEnviar->base_stack);
	int cant_digitos_puntero_stack = calcularCantDigitos(
			tcbAEnviar->cursor_stack);
	int cant_digitos_quantum = calcularCantDigitos(QUANTUM);

	int cant_digitos_a = calcularCantDigitos(tcbAEnviar->registros[0]);
	int cant_digitos_b = calcularCantDigitos(tcbAEnviar->registros[1]);
	int cant_digitos_c = calcularCantDigitos(tcbAEnviar->registros[2]);
	int cant_digitos_d = calcularCantDigitos(tcbAEnviar->registros[3]);
	int cant_digitos_e = calcularCantDigitos(tcbAEnviar->registros[4]);


	char* buffer = string_new();

	string_append(&buffer, string_itoa(cant_digitos_pid));
	string_append(&buffer, string_itoa(tcbAEnviar->pid));
	string_append(&buffer, string_itoa(cant_digitos_tid));
	string_append(&buffer, string_itoa(tcbAEnviar->tid));
	string_append(&buffer, string_itoa(tcbAEnviar->kernel_mode));
	string_append(&buffer, string_itoa(cant_digitos_base_segmento));
	string_append(&buffer, string_itoa(tcbAEnviar->segmento_codigo));
	string_append(&buffer, string_itoa(cant_digitos_base_segmento_size));
	string_append(&buffer, string_itoa(tcbAEnviar->segmento_codigo_size));
	string_append(&buffer, string_itoa(cant_digitos_puntero_instruccion));
	string_append(&buffer, string_itoa(tcbAEnviar->puntero_instruccion));
	string_append(&buffer, string_itoa(cant_digitos_base_stack));
	string_append(&buffer, string_itoa(tcbAEnviar->base_stack));
	string_append(&buffer, string_itoa(cant_digitos_puntero_stack));
	string_append(&buffer, string_itoa(tcbAEnviar->cursor_stack));
	string_append(&buffer, string_itoa(cant_digitos_quantum));
	string_append(&buffer, string_itoa(QUANTUM));

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


void entradaEstandar(int socket_cpu, char* buffer) {
 	//handshakes: EN para número, EC para cadena de caracteres
 	//se le pasa a la consola de la misma manera sin importar el tipo
 	//la consola implementa un if
 	//?
// 	while (1) {
 		char* pid_buffer = string_substring(buffer, 2, strlen(buffer)-2);
 		int pid_get = atoi(pid_buffer);
 		int socket_consola = buscarSocketConsolaPorPid(pid_get);
 		enviarDatos(socket_consola, buffer);
 		char buffer_cpu [BUFFERSIZE];
 		recibirDatos(socket_consola, buffer_cpu);
 		enviarDatos(socket_cpu, buffer_cpu);
// 	}
 }

void salidaEstandar(int socket_cpu, char* buffer) {
 	//handshakes: SN para número, SC para cadena de caracteres
 	//se le pasa a la consola de la misma manera sin importar el tipo
 	//la consola implementa un if
 	//?

// 	while (1) {
// 		int length_pid = atoi(&buffer[2]);
		int length_pid = atoi(string_substring(buffer, 2, 1));
		int pid_get = atoi(string_substring(buffer, 3, length_pid));
// 		char* pid_buffer = string_substring(buffer, 3, length_pid);
// 		int pid_get = atoi(pid_buffer - 1);
		int socket_consola = buscarSocketConsolaPorPid(pid_get);
 		char* buffer1 = string_new();
 		buffer1 = string_substring_until(buffer, 1);
 		char* buffer2 = string_new();
 		buffer2 = string_substring_from(buffer, (3 + length_pid));
 		string_append(&buffer1, buffer2);
 		enviarDatos(socket_consola, buffer1);

 		enviarDatos(socket_cpu, "1"); //ok para seguir el flujo
// 	}
}

char* crearHilo(int socket_cpu, char* buffer) {
	char* buffer2 = string_new();
	buffer2 = string_substring(buffer, 1, strlen(buffer));
	t_hilo* t_hilo_recv;
	//string_substring(buffer, 1, strlen(buffer));
	t_hilo_recv = stringATcb(buffer2);
	t_hilo* t_hilo_nuevo;

	//log llamada a syscall crear hilo
//	instruccion_protegida("Crear hilo", t_hilo_recv);

	uint32_t base_stack;
	base_stack = pedirSegmentoAMemoria(t_hilo_recv->pid, STACK);

	//devuelve -1 si da error.
	if (base_stack == -1) {
		log_info(loggerKernel, "ERROR FATAL: no hay memoria disponible.");
		close(socket_cpu);
	} else {
		t_hilo_nuevo = malloc(sizeof(t_hilo));

		//TODO:Mutex

		t_hilo* padre;
		padre = malloc(sizeof(t_hilo));
		padre = buscarTcbPorPid(listaBlock, t_hilo_recv->pid);

		t_hilo_nuevo->pid = t_hilo_recv->pid;

		int socket_consola = buscarSocketConsolaPorPid(t_hilo_recv->pid);
		t_hilo_nuevo->tid = tid;



		structTcb* new_pid = malloc(sizeof(structTcb));
		//cargo la estructura qe guarda la referencia al socket del cpu
		new_pid->pid =  t_hilo_recv->pid;
		new_pid->tid = t_hilo_nuevo->tid;
		new_pid->socket_consola = socket_consola;

		char* msjTid = string_new();
		msjTid = string_itoa(t_hilo_nuevo->tid);
		enviarDatos(socket_cpu, msjTid);

		list_add(listaTcb, new_pid);

		//acordarse que esta al reves
		int diff = padre->base_stack - padre->cursor_stack;

		t_hilo_nuevo->segmento_codigo = padre->segmento_codigo;
		t_hilo_nuevo->segmento_codigo_size = padre->segmento_codigo_size;
		t_hilo_nuevo->puntero_instruccion = t_hilo_recv->puntero_instruccion;
		t_hilo_nuevo->kernel_mode = t_hilo_recv->kernel_mode;
		t_hilo_nuevo->base_stack = base_stack;
		t_hilo_nuevo->cursor_stack = 0 + diff;
		t_hilo_nuevo->registros[0] = padre->registros[0];
		t_hilo_nuevo->registros[1] = padre->registros[1];
		t_hilo_nuevo->registros[2] = padre->registros[2];
		t_hilo_nuevo->registros[3] = padre->registros[3];
		t_hilo_nuevo->registros[4] = padre->registros[4];

		list_add(listaReady, t_hilo_nuevo); //Carga en planificador
		sem_post(&semaforoTCB);
	//	free(t_hilo_nuevo); //FIXME: VER
	//	free(new_pid); //FIXME: VER
		free(padre);
		free(t_hilo_recv);
		tid++;
	}
	char* hilo = tcbAStringSinQuantum(t_hilo_nuevo);

	return hilo;

}

t_hilo* buscarTcbPorTidJoin(int tid_a_buscar){
	bool _condicionSocket(void* tcb){
		if (((t_hilo*) tcb)->tid == tid_a_buscar) {
					return 1;
				}
					return 0;
				}

	t_hilo* tcb_encontrado = malloc(sizeof(t_hilo));
	tcb_encontrado = list_find(listaReady, _condicionSocket);
	if (tcb_encontrado == NULL){
		tcb_encontrado = list_find(listaExec, _condicionSocket);
	}
	return tcb_encontrado;
}

char* join(int socket_cpu, char* buffer) {
	while (1) {
		int length_tid_llamador = atoi(&buffer[0]);
		int tid_llamador = atoi(
				string_substring(buffer, 1, length_tid_llamador));
		int length_tid_a_esperar = atoi(&buffer[length_tid_llamador + 1]);
		int tid_a_esperar = atoi(
				string_substring(buffer, length_tid_llamador + 2,
						length_tid_a_esperar));
		t_hilo* tcb_llamador = buscarTcbPorTidJoin(tid_llamador);
		t_hilo* tcb_a_esperar = buscarTcbPorTidJoin(tid_a_esperar);

		//log syscall Join
//		instruccion_protegida("Join", tcb_llamador);

		list_add(listaBlock, tcb_llamador);
		tid_esperado_join = tcb_a_esperar->tid;
		sem_wait(&sem_join);
		tid_esperado_join = 0;
		eliminarTCB_ListaPorTid(listaBlock, tid_llamador);
		list_add(listaReady, tcb_llamador);

	}
	return 0;
}

void bloquear(int socket_cpu, char* buffer) {
	char* buffer2 = string_new();
	buffer2 = string_substring_from(buffer, 1);
	t_hilo* tcb = malloc(sizeof(t_hilo));
	tcb = stringATcb(buffer2);
	char* tcb_string = string_new();
	tcb_string = tcbAStringSinQuantum(tcb);
	int id_recurso = atoi(string_substring_from(buffer2, sizeof(tcb_string)+1));
	structRecursos* recurso = malloc(sizeof(structRecursos));
	recurso->tcb = tcb;
	recurso->id_recurso = id_recurso;

	//log syscall Join
//	instruccion_protegida("Block", tcb);

	list_add(listaBlockXRecursos, recurso);
	eliminarTCB_ListaPorTid(listaReady,tcb->tid);
	list_add(listaBlock, tcb);
	//free(tcb); //FIXME: VER
	//free(recurso); //FIXME: VER

}

structRecursos* buscarRecurso(int id_recurso_a_buscar){
	bool _condicionRecurso(void* recurso){
		if (((structRecursos*) recurso)->id_recurso == id_recurso_a_buscar) {
					return 1;
				}
					return 0;
				}

	structRecursos* recurso = malloc(sizeof(structRecursos));
	recurso = list_remove_by_condition(listaBlockXRecursos, _condicionRecurso);
	return recurso;
}

void despertar(int socket_cpu, char* buffer) {
	int id_recurso = atoi(string_substring_from(buffer,1));
	structRecursos* recurso = malloc(sizeof(structRecursos));
	recurso = buscarRecurso(id_recurso);
	t_hilo* tcb = malloc(sizeof(t_hilo));
	tcb = recurso->tcb;
	eliminarTCB_ListaPorTid(listaBlock,tcb->tid);

	//log syscall Join
//	instruccion_protegida("Wake", tcb);

	list_add(listaReady, tcb);
	sem_post(&semaforoTCB);
	//free(recurso); //FIXME: VER
	//free(tcb); //FIXME: VER
}

void crearMultiplexor() {
	int nbytesRecibidos;
	char buffer[BUFFERSIZE];
	char handshake;

	//int yes=1;        // para setsockopt() SO_REUSEADDR, más abajo
	//	struct sockaddr_in myaddr;     // dirección del servidor

	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	int fdmax;        // número máximo de descriptores de fichero
	int socketEscucha;     // descriptor de socket a la escucha
	int socketNuevaConexion; // descriptor de socket de nueva conexión aceptada
	int i;
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	struct sockaddr_in socket_cliente;
	int optval = 1;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		log_info(loggerKernel, "ERROR FATAL: socket");
		exit(1);
		//return 1;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socket_cliente.sin_family = AF_INET;
	socket_cliente.sin_addr.s_addr = htons(INADDR_ANY );
	socket_cliente.sin_port = htons(PUERTO_KERNEL);

	// Vincular el socket con una direccion de red almacenada en 'socket_cliente'.
	if (bind(socketEscucha, (struct sockaddr*) &socket_cliente,
			sizeof(socket_cliente)) != 0) {
		log_info(loggerKernel, "Error al bindear socket escucha");
		exit(1);
	}

	// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0) {
		log_info(loggerKernel, "ERROR FATAL: al poner a escuchar socket");
		exit(1);
	}
	log_trace(loggerKernel, "Escuchando conexiones entrantes en puerto (%d).\n",
			PUERTO_KERNEL);

	// añadir listener al conjunto maestro
	FD_SET(socketEscucha, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = socketEscucha; // por ahora es éste

	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL ) == -1) {
			log_info(loggerKernel, "ERROR FATAL: select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // ¡¡tenemos datos!!
				if (i == socketEscucha) {
					// gestionar nuevas conexiones
					// addrlen = sizeof(remoteaddr);
					if ((socketNuevaConexion = accept(socketEscucha, NULL, 0))
							== -1) {
						log_info(loggerKernel, "ERROR: accept");
					} else {
						FD_SET(socketNuevaConexion, &master); // añadir al conjunto maestro
						if (socketNuevaConexion > fdmax) { // actualizar el máximo
							fdmax = socketNuevaConexion;
						}

						printf("selectserver: una nueva conneccion socket %d\n",
								socketNuevaConexion);
					}
				} else {

					// gestionar datos de un cliente
					// Recibir hasta BUFFERSIZE datos y almacenarlos en 'buffer'.

					//Recibimos los datos del cliente
					nbytesRecibidos = recibirDatos(i, buffer);

					if (nbytesRecibidos <= 0) {
						// error o conexión cerrada por el cliente
						if (nbytesRecibidos == 0) {
							// conexión cerrada
							printf("selectserver: socket %d hung up\n", i);

							if(eliminarCPU_Lista(listaCpuLibre,i) == 1){
								sem_wait(&semaforoCPU);
								log_info(loggerKernel, "Se eliminó el cpu socket: %d de listaCpuLibre", i);
//								desconexion_cpu(i);
							}

						} else {
							log_info(loggerKernel, "ERROR: recv");
						}
						close(i);
						FD_CLR(i, &master); // eliminar del conjunto maestro
					} else {
						handshake = buffer[0];
						log_trace(loggerKernel, "Lee handhsake: %s", handshake);
						//Evaluamos los comandos
						switch (handshake) {
						case SOY_CONSOLA:
							atenderConsola(i, buffer);
							break;
						case SOY_CPU:
							//avisa que hay cpu libre
							atenderCPU(i, buffer);
							break;
						case SOY_CPU_INTE:
							//llama al servicio interrupcion
							interrupcion(i, buffer);
							break;
						case SOY_CPU_ENTRADA:
 							//llama al servicio entrada estandar
							entradaEstandar(i, buffer);
 							break;
 						case SOY_CPU_SALIDA:
 							//llama al servicio salida estandar
							salidaEstandar(i, buffer);
 							break;
 						case SOY_CPU_CREAR:
 							//llama al sevicio crear hilo
							crearHilo(i, buffer);
 							break;
 						case SOY_CPU_JOIN:
 							//llama al servicio join
							join(i, buffer);
 							break;
 						case SOY_CPU_BLOCK:
 							//llama al servicio bloquear
							bloquear(i, buffer);
 							break;
 						case SOY_CPU_DESPERTAR:
 							//llama al servicio despertar
							despertar(i, buffer);
 							break;
						default:
							//avisar error en conexion
							//		reportarErrorEnConexion(i);
							puts("ERROR: HANDSHAKE DESCONOCIDO");
							enviarDatos(i, MSJ_HANDSHAKE_DESCONOCIDO);
							log_info(loggerKernel,
									"ERROR: handshake desconocido.\n");
						}
						buffer[0] = '\0';
					}
				}
			}
		}
	}

	return;
}

int pidARemover(t_hilo tcb, int pidABorrar) {

	if (tcb.pid == pidABorrar) {
		return 1;
	} else {
		return 0;
	}

}

int eliminarCPU_Lista(t_list* lista, int socket_cpu) {
	//Me fijo si existe un segmento con el pid del parametro y la direccion base
	int estado;

	bool _true(void* socket) {
	if ((int)socket == socket_cpu) {
		estado = 1;
		return estado;
	}
		estado = 0;
		return estado;
	}

	list_remove_by_condition(lista, _true);
	return estado;
}

void eliminarTCB_Lista(t_list* lista, int pid) {
	//Me fijo si existe un segmento con el pid del parametro y la direccion base
	bool _true(void* tcb) {
	if (((t_hilo*) tcb)->pid == pid) {
		return 1;
	}
		return 0;
	}

	list_remove_by_condition(lista, _true);

}

void eliminarTCB_ListaPorTid(t_list* lista, int pTid) {
	//Me fijo si existe un segmento con el pid del parametro y la direccion base
	bool _true(void* tcb) {
	if (((t_hilo*) tcb)->tid == pTid) {
		return 1;
	}
		return 0;
	}

	list_remove_by_condition(lista, _true);

}

void planificador() {

	while(1){
		sem_wait(&semaforoCPU);
		ejecutarLlamadaAlSist(); //si hay
		sem_wait(&semaforoTCB);

		t_hilo *tcbAEnviar;
		tcbAEnviar = list_get(listaReady, 0);

		char* buffer = string_new();
		string_append(&buffer, tcbAString(tcbAEnviar));

		int socket_cpu = (int) list_get(listaCpuLibre,0);
		list_remove(listaCpuLibre, 0);

		if (send(socket_cpu, buffer, BUFFERSIZE, 0) == -1) {
			log_info(loggerKernel,
					"No puedo enviar información. Socket: %d. Config: %s",
					socket_cpu, PATH_CONFIG);
		} else {
			eliminarTCB_ListaPorTid(listaReady, (tcbAEnviar->tid));
			list_add(listaExec, tcbAEnviar);
		}
	}
}

void limpiarMSP(int pid, int base_segmento){

	// Graba en la memoria
		// Formato del mensaje: CABBBBCDDDDHIIIJOOOOOOOOO....
		// C = Codigo de mensaje ( = 6)
		// A = Cantidad de digitos que tiene el pid
		// BBBB = Pid del proceso (hasta 9999)
		// C = Cantidad de digitos que tiene el segmento
		// DDDD = El numero del segmento (hasta 4096)

		int cant_digitos_pid;
		int cant_digitos_base_segmento;
		cant_digitos_pid = calcularCantDigitos(pid);
		cant_digitos_base_segmento = calcularCantDigitos(base_segmento);


		char* buffer = string_new();
		string_append(&buffer, string_itoa(MSJ_DESTRUIR_SEGMENTO));
		string_append(&buffer, string_itoa(cant_digitos_pid));
		string_append(&buffer, string_itoa(pid));
		string_append(&buffer, string_itoa(cant_digitos_base_segmento));
		string_append(&buffer, string_itoa(base_segmento));

		if (enviarDatos(socket_msp, buffer) == -1 ) {
			log_info(loggerKernel,
					"ERROR FATAL: no se pudo enviar mensaje destruir segmento a memoria");
			close(socket_msp);
		}

		char buffer2[BUFFERSIZE];

		recibirDatos(socket_msp, buffer2);

		if(strcmp(buffer2, "1") != 0){
			log_info(loggerKernel,
						"ERROR: msp no devolvio confirmacion de destruir segmento");
		}


};

void atenderCPU(int socket_cpu, char* buffer) {

	//Formato del msj: AB
	// A = 'U'
	// B = mensaje
		//0 devuelve tcb por quantum
		//1 libre
		//2 devuelve tcb finalizado
		//3 segmentation fault

	int libre = charToInt(buffer[1]);

//	enviarDatos(socket_cpu, "1"); //ok para recibir 2do mensaje

	if (libre == 1) {
		list_add(listaCpuLibre, (void*) socket_cpu);
		sem_post(&semaforoCPU);
	}

	if (libre == 0){
		enviarDatos(socket_cpu, "1"); //ok para seguir el flujo

		char buffer2[BUFFERSIZE];
		recibirDatos(socket_cpu, buffer2);

		t_hilo* tcb = stringATcb(buffer2);
		list_add(listaReady, tcb);
		sem_post(&semaforoTCB);
		eliminarTCB_Lista(listaExec, tcb->pid);

	}

	if (libre == 2){
		enviarDatos(socket_cpu, "1"); //ok para seguir el flujo

		char buffer2[BUFFERSIZE];
		recibirDatos(socket_cpu, buffer2);

		t_hilo* tcb = stringATcb(buffer2);

		if (tcb->kernel_mode == 1) {
			finalizarLlamadaAlSist(tcb);
		} else {
			if (tid_esperado_join != 0) {
				if (tcb->tid == tid_esperado_join) {
					sem_post(&sem_join);
				}
			}
			list_add(listaExit, tcb);
			limpiarMSP(tcb->pid, tcb->segmento_codigo);
			limpiarMSP(tcb->pid, tcb->base_stack);
//			free(tcb); //probar
			char* mensaje = string_new();
			string_append(&mensaje, "MEl programa termino correctamente.");
			int socket_consola = buscarSocketConsolaPorPid(tcb->pid);
			enviarDatos(socket_consola, mensaje);
			eliminarTCB_Lista(listaExec, tcb->pid);
		}

	}
	if (libre == 3){
		enviarDatos(socket_cpu, "1"); //ok para seguir el flujo

		char buffer2[BUFFERSIZE];
		recibirDatos(socket_cpu, buffer2);

		t_hilo* tcb = stringATcb(buffer2);

		list_add(listaExit, tcb);
		limpiarMSP(tcb->pid, tcb->segmento_codigo);
		limpiarMSP(tcb->pid, tcb->base_stack);

		char* mensaje = string_new();
		string_append(&mensaje, "MSegmentation fault.");
		int socket_consola = buscarSocketConsolaPorPid(tcb->pid);
		enviarDatos(socket_consola, mensaje);
		eliminarTCB_Lista(listaExec, tcb->pid);

	}

}

void leerConfigKernel() {

	t_config* config = config_create(PATH_CONFIG);
	log_info(loggerKernel, "Archivo de configuración: %s", PATH_CONFIG);

	if (config->properties->table_current_size != 0) {	//tiene data

		if (config_has_property(config, "PUERTO")) {//retorna TRUE si PUERTO se encuentra en config
			PUERTO_KERNEL = config_get_int_value(config, "PUERTO");
		} else {
			printf("ERROR: No se pudo leer el parametro PUERTO \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro PUERTO \n",
					PATH_CONFIG);
		}

		if (config_has_property(config, "PUERTO_MSP")) {
			PUERTO_MSP = config_get_string_value(config, "PUERTO_MSP");
		} else {
			printf("ERROR: No se pudo leer el parametro PUERTO_MSP \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro PUERTO_MSP \n",
					PATH_CONFIG);
		}

		if (config_has_property(config, "IP_MSP")) {
			IP_MSP = config_get_string_value(config, "IP_MSP");
		} else {
			printf("ERROR: No se pudo leer el parametro IP_MSP \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro IP_MSP \n",
					PATH_CONFIG);
		}

		if (config_has_property(config, "QUANTUM")) {
			QUANTUM = config_get_int_value(config, "QUANTUM");
		} else {
			printf("ERROR: No se pudo leer el parametro QUANTUM \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro QUANTUM \n",
					PATH_CONFIG);
		}

		if (config_has_property(config, "SYSCALLS")) {
			SYSCALLS = config_get_string_value(config, "SYSCALLS");
		} else {
			printf("ERROR: No se pudo leer el parametro SYSCALLS \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro SYSCALLS \n",
					PATH_CONFIG);
		}

		if (config_has_property(config, "STACK")) {
			STACK = config_get_int_value(config, "STACK");
		} else {
			printf("ERROR: No se pudo leer el parametro STACK \n");
			log_info(loggerKernel,
					"ERROR: No se pudo leer el parametro STACK \n",
					PATH_CONFIG);
		}

	}
}

int conectarMSP() {

	//ESTRUCTURA DE SOCKETS; EN ESTE CASO CONECTA CON MSP
	log_info(loggerKernel, "Intentando conectar a memoria\n");

	//conectar con memoria
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	if (getaddrinfo(IP_MSP, PUERTO_MSP, &hints, &serverInfo) != 0) {// Carga en serverInfo los datos de la conexion
		log_info(loggerKernel,
				"ERROR: cargando datos de conexion socket_memoria");
	}

	int socket_memoria;
	if ((socket_memoria = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol)) < 0) {
		log_info(loggerKernel, "ERROR: crear socket_memoria");
	}
	if (connect(socket_memoria, serverInfo->ai_addr, serverInfo->ai_addrlen)
			< 0) {
		log_info(loggerKernel, "ERROR: conectar socket_memoria");
	}
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	return (socket_memoria);
}

void loader(int socket_consola, int prog_size, char* buffer) {

	 if(flag){//si no existe un tcb kernel

		 flag = 0;

		 FILE * programa;
		 programa = fopen(SYSCALLS,"rb");

		 size_t prog_tamanio_syscalls;
		 fseek(programa,0, SEEK_END);	//nos situa en el final del archivo
		 prog_tamanio_syscalls=ftell(programa);	//devuelve el tamaño del archivo en bytes
		 rewind(programa);				//nos situa en el comienzo del archivo

		 uint32_t base_segmento_k;
		 base_segmento_k = pedirSegmentoAMemoria(1,prog_tamanio_syscalls); //1 pid del kernel

		 uint32_t base_stack_k;
		 base_stack_k = pedirSegmentoAMemoria(1, STACK); //1 pid del kernel,

		 char* bufferProgramaKernel;
		 bufferProgramaKernel = (char*) malloc(prog_tamanio_syscalls);
		 fread(bufferProgramaKernel,sizeof(char),prog_tamanio_syscalls,programa);

		 escribirMemoria(pid, base_segmento_k, prog_tamanio_syscalls, bufferProgramaKernel);


		 t_hilo* tcb_kernel = malloc(sizeof(t_hilo));

		 tcb_kernel->pid = pid;
		 pid++;
		 tcb_kernel->tid = 1;
		 tcb_kernel->segmento_codigo = base_segmento_k;
		 tcb_kernel->segmento_codigo_size = prog_tamanio_syscalls;
		 tcb_kernel->puntero_instruccion = 0;
		 tcb_kernel->kernel_mode = 1;
		 tcb_kernel->base_stack = base_stack_k;
		 tcb_kernel->cursor_stack = 0;
		 tcb_kernel->registros[0] = 0;
		 tcb_kernel->registros[1] = 0;
		 tcb_kernel->registros[2] = 0;
		 tcb_kernel->registros[3] = 0;
		 tcb_kernel->registros[4] = 0;


		 list_add(listaBlock, tcb_kernel);
		 //free(bufferProgramaKernel); //FIXME: VER
		 //free(tcb_kernel); //FIXME: VER
	 }

	uint32_t base_segmento;
	base_segmento = pedirSegmentoAMemoria(pid, prog_size);

//	printf("La base del segmento recibida es:%d\n", base_segmento);
//	printf("Valor semaforoTCB antes del signal: %d", semaforoTCB);

	uint32_t base_stack;
	base_stack = pedirSegmentoAMemoria(pid, STACK);

	printf("DEBUG SEBA STYLE: PID = %d, base_segmento = %d, base_stack = %d \n", pid, base_segmento, base_stack);

	//devuelve -1 si da error.
	if (base_segmento == -1) {
		log_info(loggerKernel, "ERROR FATAL: no hay memoria disponible.");
		close(socket_consola);
	} else {
		t_hilo* new_tcb = malloc(sizeof(t_hilo));
		structTcb* new_pid = malloc(sizeof(structTcb));

		new_pid->pid = pid;
		new_pid->tid = tid;
		new_pid->socket_consola = socket_consola;

		list_add(listaTcb, new_pid);
		//TODO:Mutex

		new_tcb->pid = pid;
		//tid inicializado en 1
		new_tcb->tid = tid;
		new_tcb->segmento_codigo = base_segmento;
		new_tcb->segmento_codigo_size = prog_size;
		new_tcb->puntero_instruccion = 0;
		new_tcb->kernel_mode = false;
		new_tcb->base_stack = base_stack;
		new_tcb->cursor_stack = 0;
		new_tcb->registros[0] = 0;
		new_tcb->registros[1] = 0;
		new_tcb->registros[2] = 0;
		new_tcb->registros[3] = 0;
		new_tcb->registros[4] = 0;

		int respuesta_msp = escribirMemoria(pid, base_segmento, prog_size, buffer);

		//free(new_tcb); //FIXME: VER
		//free(new_pid); //FIXME: VER
		if(respuesta_msp != -1) {
			list_add(listaReady, new_tcb); //Carga en planificador
			sem_post(&semaforoTCB);
		}else{
			log_info(loggerKernel,
					"ERROR: No recibio confirmación de escritura de la msp.");
		};

	}

	pid++;
	tid++;
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
	string_append(&buffer, string_itoa(MSJ_CREAR_SEGMENTO));
	string_append(&buffer, string_itoa(cant_digitos_pid));
	string_append(&buffer, string_itoa(pid));
	string_append(&buffer, string_itoa(cant_digitos_prog_size));
	string_append(&buffer, string_itoa(prog_size));

	if (enviarDatos(socket_msp, buffer) == -1) {
		log_info(loggerKernel,
				"ERROR FATAL: no se pudo enviar MSJ_CREAR_SEGMENTO.");
		exit(1);
	}

	char respuestaMSP[BUFFERSIZE];
	recibirDatos(socket_msp, respuestaMSP);

	int direccion;
	direccion = atoi(respuestaMSP);

	return direccion;
}

int escribirMemoria(int pid, int base_segmento, int prog_size, char* programa) {
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
	cant_digitos_pid = calcularCantDigitos(pid);
	cant_digitos_base_segmento = calcularCantDigitos(base_segmento);
	cant_digitos_prog_size = calcularCantDigitos(prog_size);


	char* buffer = string_new();
	string_append(&buffer, string_itoa(MSJ_ESCRIBIR_MEMORIA));
	string_append(&buffer, string_itoa(cant_digitos_pid));
	string_append(&buffer, string_itoa(pid));
	string_append(&buffer, string_itoa(cant_digitos_base_segmento));
	string_append(&buffer, string_itoa(base_segmento));
	string_append(&buffer, "1");
	string_append(&buffer, "0");
	string_append(&buffer, string_itoa(cant_digitos_prog_size));
	string_append(&buffer, string_itoa(prog_size));

	if (enviarDatos(socket_msp, buffer) == -1 ) {
		log_info(loggerKernel,
				"ERROR FATAL: no se pudo enviar datos de programa a memoria");
		close(socket_msp);
		return -1;
	} else {
		if (enviarDatosConTamanio(socket_msp, programa, prog_size) == -1) {
			log_info(loggerKernel,
					"ERROR FATAL: no se pudo escribir programa en memoria");
			return -1;
			close(socket_msp);
		}else{
			char buffer2 [BUFFERSIZE];
			recibirDatos(socket_msp,buffer2);

			int respuesta_msp = atoi(buffer2);
			return respuesta_msp;

		}


	}
}

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

int enviarDatos(int socket, void *buffer) {

//	pthread_mutex_lock(&mutexEnviarDatos);

	int bytecount;

	if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1) {
		printf("ERROR: no se pudo enviar información. \n");
		log_info(loggerKernel, "ERROR: no se pudo enviar información. \n");
	}
	log_info(loggerKernel, "ENVIO datos. socket: %d. buffer: %s", socket,
			(char*) buffer);

//	pthread_mutex_unlock(&mutexEnviarDatos);

	return (bytecount);
}

int enviarDatosConTamanio(int socket, void *buffer, int tamanio){
	int bytecount;

		if ((bytecount = send(socket, buffer, tamanio, 0)) == -1){
			printf("ERROR: no se pudo enviar información. \n");
					log_info(loggerKernel, "ERROR: no se pudo enviar información. \n");
		}
		log_info(loggerKernel, "ENVIO datos. socket: %d. buffer: %s", socket,
					(char*) buffer);

		return bytecount;
}

int recibirDatos(int socket, char *buffer) {

//	pthread_mutex_lock(&mutexRecibirDatos);

	int bytecount;
	// memset se usa para llenar el buffer con 0s
	memset(buffer, 0, BUFFERSIZE);

	//Nos ponemos a la escucha de las peticiones que nos envie el kernel
	//aca si recibo 0 bytes es que se desconecto el otro, cerrar el hilo.
	if ((bytecount = recv(socket, buffer, BUFFERSIZE, 0)) == -1)//1 pid del kernel,
		log_info(loggerKernel, "ERROR: error al intentar recibir datos");

	log_info(loggerKernel, "RECIBO datos. socket: %d. buffer: %s\n", socket,
			(char*) buffer);

//	pthread_mutex_lock(&mutexRecibirDatos);

	return (bytecount);
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

t_hilo* stringATcb(char* buffer) {

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
	/*
	 * cant a
	 * a
	 * cant b
	 * b
	 * cant c
	 * c
	 * cant d
	 * d
	 * cant e
	 * e
	 */

	t_hilo* hilo = malloc(sizeof(t_hilo));

	int posicion = 0;
		int cantidadDigitosPID = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->pid = subCadenaAInt(buffer, posicion, cantidadDigitosPID);

		posicion = posicion + cantidadDigitosPID;
		int cantidadDigitosTID = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->tid = subCadenaAInt(buffer, posicion, cantidadDigitosTID);

		posicion = posicion + cantidadDigitosTID;
		hilo->kernel_mode = subCadenaAInt(buffer, posicion, 1);

		posicion++;
		int cantidadDigitosBase = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->segmento_codigo = subCadenaAInt(buffer, posicion, cantidadDigitosBase);

		posicion = posicion + cantidadDigitosBase;
		int cantidadDigitosBaseSize = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->segmento_codigo_size = subCadenaAInt(buffer, posicion, cantidadDigitosBaseSize);

		posicion = posicion + cantidadDigitosBaseSize;
		int cantidadDigitosIP = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->puntero_instruccion = subCadenaAInt(buffer, posicion, cantidadDigitosIP);

		posicion = posicion + cantidadDigitosIP;
		int cantidadDigitosBaseStack = posicionDeBufferAInt(buffer, posicion);
		posicion ++;
		hilo->base_stack = subCadenaAInt(buffer, posicion, cantidadDigitosBaseStack);

		posicion = posicion +cantidadDigitosBaseStack;
		int cantidadDigitosPtrStack= posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->cursor_stack = subCadenaAInt(buffer, posicion, cantidadDigitosPtrStack);

		posicion = posicion + cantidadDigitosPtrStack;
		int cantidadDigitosA = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->registros[0] = subCadenaAInt(buffer, posicion, cantidadDigitosA);

		posicion = posicion + cantidadDigitosA;
		int cantidadDigitosB = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->registros[1] = subCadenaAInt(buffer, posicion, cantidadDigitosB);

		posicion = posicion + cantidadDigitosB;
		int cantidadDigitosC = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->registros[2] = subCadenaAInt(buffer, posicion, cantidadDigitosC);

		posicion = posicion + cantidadDigitosC;
		int cantidadDigitosD = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->registros[3] = subCadenaAInt(buffer, posicion, cantidadDigitosD);

		posicion = posicion + cantidadDigitosD;
		int cantidadDigitosE = posicionDeBufferAInt(buffer, posicion);
		posicion++;
		hilo->registros[4] = subCadenaAInt(buffer, posicion, cantidadDigitosE);

	return (hilo);
}
