/*
 * versicherungstyp.h
 *
 *  Created on: 03.04.2016
 *      Author: stwaidele
 */

#ifndef VERSICHERUNGSTYP_H_
#define VERSICHERUNGSTYP_H_

#include <stdbool.h>

#define INITIAL_SIZE_OF_VERTRAG_ARRAY 10

// Datentypen
typedef struct {
	unsigned int currentVertragsID;
	unsigned int sizeOfVertragArray;
	unsigned int i;				// Poition des nächsten freien Feldes im Array
	vertragstyp *Vertrag;
} versicherungstyp;

// Funktionsprototypen
int getNextVertragsID(versicherungstyp *);
void versicherungstypConstructor(versicherungstyp *);

#endif /* VERSICHERUNGSTYP_H_ */
