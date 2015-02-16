/*
 * consola.h
 *
 *  Created on: 12/09/2014
 *      Author: utnso
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

#if 1
t_log* loggerConsola;
#define HANDSHAKE_CONSOLA "C"
#define BUFFERSIZE 1200

int socket_kernel;

#endif /* CONSOLA_H_ */

void LeerConfigConsola();
