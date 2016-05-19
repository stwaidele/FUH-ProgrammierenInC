/*
 * versicherungstyp.c
 *
 *  Created on: 03.04.2016
 *      Author: stwaidele
 *
 *  Der Datentyp „versicherungstyp“ bildet das Versicherungsunternehmen
 *  ab. Hier werden alle Daten gespeichert, die Unternehmensweit benötigt werden.
 *
 *  Die Versicherungsverträge werden hier ebenfalls gespeichert.
 *  In der gegebenen Aufgabenstellung wird nur eine „Instanz“ vom Typ
 *  „versicherungstyp“ benötigt.
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "abrechnungstyp.h"
#include "vertragstyp.h"

#include "versicherungstyp.h"

void versicherungstypConstructor(versicherungstyp *vers) {
	/*
	 * Hier werden die Werte aus der Datei gelesen bzw. eine leere „Instanz“ initialisiert.
	 */
	vertragstyp *vertrag; // Benötigt für sizeof() ToDo: Wirklich?

	vers->currentVertragsID = 0;
	vers->sizeOfVertragArray = INITIAL_SIZE_OF_VERTRAG_ARRAY;
	vers->Vertrag = malloc(INITIAL_SIZE_OF_VERTRAG_ARRAY * sizeof(*vertrag));
	if (vers->Vertrag==0){
		printf("malloc fehlgeschlagen!\n");
	}
	vers->i = 0;
}

unsigned int addVertrag(versicherungstyp *vers, vertragstyp *v) {
	if (vers->i >= vers->sizeOfVertragArray) {
		// Array vergrößern: http://stackoverflow.com/questions/4694401/how-to-replicate-vector-in-c
		void* newMem = realloc(vers->Vertrag,
				2 * vers->sizeOfVertragArray * sizeof(*v));
		if (!newMem) {
			// ToDo: Fehlermeldung
		}
		vers->Vertrag = newMem;
		vers->sizeOfVertragArray = 2 * vers->sizeOfVertragArray;
		printf("Speicher Vergrößert auf %i.\n", vers->sizeOfVertragArray);
	}
	vers->Vertrag[vers->i] = *v;
	return vers->i++;
}

int getNextVertragsID(versicherungstyp *v) {
	int nextID;
	/*
	 * Die nächste verfügbare Vertragsnummer muss eigenständig gespeichert werden,
	 * um doppelvergaben zu vermeiden. Sie kann nicht aus den gespeicherten Verträgen
	 * ermittelt werden: Die IDs von gelöschten Verträgen am Ende der Liste würden
	 * erneut vergeben werden.
	 */
	/*
	 * Achtung, potentielle Racecondition:
	 * Lesen und Hochzählen der Variablen muss in Multithreading–Umgebungen durch
	 * Mutex o.ä. geschützt werden.
	 */
	nextID = ++v->currentVertragsID;
	return nextID;
}

