/*
 * instrucciones.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include <string.h>
#include "instrucciones.h"
#include "funciones.h"

//recibe un puntero a los registros del cpu, un string con la instruccion
// y los parametros para la funcion

int instrucciones(char* instruccion,  t_registros_cpu* registros, int parametros[]){

	void (*load_ptr)(t_registros_cpu*, int[]) = load;
	void (*getm_ptr)(t_registros_cpu*, int[]) = getm;
	void (*setm_ptr)(t_registros_cpu*, int[]) = setm;
	void (*movr_ptr)(t_registros_cpu*, int[]) = movr;
	void (*addr_ptr)(t_registros_cpu*, int[]) = addr;
	void (*subr_ptr)(t_registros_cpu*, int[]) = subr;
	void (*mulr_ptr)(t_registros_cpu*, int[]) = mulr;
	void (*modr_ptr)(t_registros_cpu*, int[]) = modr;
	void (*divr_ptr)(t_registros_cpu*, int[]) = divr;
	void (*incr_ptr)(t_registros_cpu*, int[]) = incr;
	void (*decr_ptr)(t_registros_cpu*, int[]) = decr;
	void (*comp_ptr)(t_registros_cpu*, int[]) = comp;
	void (*cgeq_ptr)(t_registros_cpu*, int[]) = cgeq;
	void (*cleq_ptr)(t_registros_cpu*, int[]) = cleq;
	void (*g0t0_ptr)(t_registros_cpu*, int[]) = g0t0;
	void (*jmpz_ptr)(t_registros_cpu*, int[]) = jmpz;
	void (*jpnz_ptr)(t_registros_cpu*, int[]) = jpnz;
	void (*inte_ptr)(t_registros_cpu*, int[]) = inte;
//	void (*flcl_ptr)(t_registros_cpu*, int[]) = flcl;
	void (*shif_ptr)(t_registros_cpu*, int[]) = shif;
	void (*nopp_ptr)(t_registros_cpu*, int[]) = nopp;
	void (*push_ptr)(t_registros_cpu*, int[]) = push;
	void (*take_ptr)(t_registros_cpu*, int[]) = take;
	void (*xxxx_ptr)(t_registros_cpu*, int[]) = xxxx;

	//arrays de punteros a funciones
	void* funciones[] = {
	load_ptr, getm_ptr, setm_ptr, movr_ptr,
	addr_ptr, subr_ptr, mulr_ptr, modr_ptr,
	divr_ptr, incr_ptr, decr_ptr, comp_ptr,
	cgeq_ptr, cleq_ptr, g0t0_ptr, jmpz_ptr,
	jpnz_ptr, inte_ptr, shif_ptr,
	nopp_ptr, push_ptr, take_ptr, xxxx_ptr
};

	void (*malc_ptr)(t_registros_cpu*, int[]) = malc;
	void (*FREE_ptr)(t_registros_cpu*, int[]) = FREE;
	void (*innn_ptr)(t_registros_cpu*, int[]) = innn;
	void (*innc_ptr)(t_registros_cpu*, int[]) = innc;
	void (*outn_ptr)(t_registros_cpu*, int[]) = outn;
	void (*outc_ptr)(t_registros_cpu*, int[]) = outc;
	void (*crea_ptr)(t_registros_cpu*, int[]) = crea;
	void (*join_ptr)(t_registros_cpu*, int[]) = join;
	void (*blok_ptr)(t_registros_cpu*, int[]) = blok;
	void (*wake_ptr)(t_registros_cpu*, int[]) = wake;


	void* funcionesProtegidas[] = {
		malc_ptr, FREE_ptr, innn_ptr, innc_ptr,
		outn_ptr, outc_ptr, crea_ptr, join_ptr,
		blok_ptr, wake_ptr
	};

	//el array de indices y el array de punteros estan asociados

	//arrays de indices
	const char *indices[] = {
		"LOAD", "GETM", "SETM", "MOVR",
		"ADDR", "SUBR", "MULR", "MODR",
		"DIVR", "INCR", "DECR", "COMP",
		"CGEQ", "CLEQ", "GOTO", "JMPZ",
		"JPNZ", "INTE", "SHIF",
		"NOPP", "PUSH", "TAKE", "XXXX"
	};

	const char* indicesProtegidas[]= {
		"MALC", "FREE", "INNN", "INNC",
		"OUTN", "OUTC", "CREA", "JOIN",
		"BLOK", "WAKE"
	};

	esProtegida = 0;
	flagTerminoPrograma = 0;

	//control de los for, indice -1 default
	int i;
	int j;
	int indice = -1;

	//busco la instruccion en el array de indices de instrucciones comunes
	for (i = 0; i < 23; i++){
		if (strcmp(instruccion, indices[i]) == 0 ){
			indice = i;
			break;
		}
	};

	//si la instruccion es protegida
	if (indice == -1){
		for (j = 0; j < 10; j++){
			if (strcmp(instruccion, indicesProtegidas[j]) == 0){
				indice = j;
				esProtegida = 1;
				break;
			}
		};
	};

	if (indice == -1){
		return 1; //si sigue siendo -1 hubo error de lectura o la instruccion no existe
	}

	//levanto el puntero con el indice

	if(!esProtegida){
		int (*ptr_funcion)(t_registros_cpu* ,int[]) = funciones[indice];
		(ptr_funcion)(registros, parametros);
	}else{
		if(esProtegida && registros->K){
			int (*ptr_funcion)(t_registros_cpu*, int[]) = funcionesProtegidas[indice];
			(ptr_funcion)(registros, parametros);

		}else{
			//le cortamos las piernas
			//TODO: mandar a consola, o no?
			printf("ERROR: Un TCB con KM = 0 invocó una llamada al sistema.");
			flagTerminoPrograma = 1;
		};
	};


	if(flagTerminoPrograma) return 1;

	return 0;
};

