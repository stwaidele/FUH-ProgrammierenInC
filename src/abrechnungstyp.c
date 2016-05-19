/*
 * abrechnungstyp.c
 *
 *  Created on: 04.04.2016
 *      Author: stwaidele
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "abrechnungstyp.h"

abrechnungstyp *abrechnungstypConstructor(int b, char m, int j) {
	abrechnungstyp *a = malloc(sizeof(*a));
	if (a) {
		a->Betrag = b;
		a->FaelligMonat = m;
		a->FaelligJahr = j;
		a->Status = offen;
	} else {
		// ToDo: Fehlermeldung ausgeben
	}

	return a;
}

void abrechnungstypDestructor(abrechnungstyp *a) {
	// ToDo: funktioniert nicht! Warum?
	// free(&a);
}

char *displayAbrechnung(abrechnungstyp *a) {
	char *output;

	return output;
}

bool istFaellig(abrechnungstyp *a) {
	return (a->Status == feallig);
}
bool istBezahlt(abrechnungstyp *a) {
	return (a->Status == bezahlt);
}
bool istOffen(abrechnungstyp *a) {
	return (a->Status == offen);
}
