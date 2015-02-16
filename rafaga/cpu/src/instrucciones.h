/*
 * instrucciones.h
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_

#include "cpupanel.h"
#include <stdlib.h>
#include <stdio.h>

	int esProtegida;

	//definicion de punteros a funciones
	//funciones definidas en funciones.c


	//	void (*malc_ptr)(t_registros_cpu*) = malc;
	//	void (*FREE_ptr)(t_registros_cpu*) = FREE;
	//	void (*innn_ptr)(t_registros_cpu*) = innn;
	//	void (*innc_ptr)(t_registros_cpu*) = innc;
	//	void (*outn_ptr)(t_registros_cpu*) = outn;
	//	void (*outc_ptr)(t_registros_cpu*) = outc;
	//	void (*crea_ptr)(t_registros_cpu*) = crea;
	//	void (*join_ptr)(t_registros_cpu*) = join;
	//	void (*blok_ptr)(t_registros_cpu*) = blok;
	//	void (*wake_ptr)(t_registros_cpu*) = wake;



	//	void* funcionesProtegidas[] = {
	//		malc, FREE, innn, innc,
	//		outn, outc, crea, join,
	//		blok, wake
	//	};

int instrucciones(char*, t_registros_cpu*, int[]);

#endif /* INSTRUCCIONES_H_ */

