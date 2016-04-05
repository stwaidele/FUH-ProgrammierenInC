/*
 * abrechnungstyp.h
 *
 *  Created on: 03.04.2016
 *      Author: stwaidele
 */

#ifndef ABRECHNUNGSTYP_H_
#define ABRECHNUNGSTYP_H_

#include <stdbool.h>

// Datentypen
enum zahlstatus {
	offen, feallig, bezahlt
};

typedef struct abrechnungstyp {
	unsigned int Betrag;		// http://martinfowler.com/eaaDev/quantity.html
	unsigned char FaelligMonat;		// 1 - 12
	unsigned int FaelligJahr;		// ...wir haben ein Jahr-65.536-Problem...
	enum zahlstatus Status;
	struct abrechnungstyp *next;	// Einfach verlinkte Liste
} abrechnungstyp;

// Funktionsprototypen
abrechnungstyp *abrechnungstypConstructor(int Betrag, char FaelligMonat,
		int FaelligJahr);
void abrechnungstypDestructor(abrechnungstyp *);
bool istFaellig(abrechnungstyp *);
bool istBezahlt(abrechnungstyp *);
bool istOffen(abrechnungstyp *);

#endif /* ABRECHNUNGSTYP_H_ */
