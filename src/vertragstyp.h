/*
 * vertragstyp.h
 *
 *  Created on: 03.04.2016
 *      Author: stwaidele
 */

#ifndef VERTRAGSTYP_H_
#define VERTRAGSTYP_H_

#include <stdbool.h>

// Datentypen

enum zahlungsmodus {
	jaehrlich = 1, halbjaehrlich = 2, vierteljaehrlich = 4
};
enum laufzeit {
	l1 = 1, l2 = 2, l3 = 3, l4 = 4, l5 = 5
};
enum monate {
	keinmonat,
	januar,
	februar,
	maerz,
	april,
	mai,
	juni,
	juli,
	august,
	september,
	oktober,
	november,
	dezember
};

typedef struct {
	unsigned int VertragsID;
	char Name[25];
	char Vorname[25];
	char Strasse[25];
	char PLZ[10];				// https://de.wikipedia.org/wiki/Postleitzahl
	char Ort[25];
	char Land[25];// https://de.wikipedia.org/wiki/L%C3%A4ndercodes_im_Briefdienst
	unsigned int Jahresbeitrag;	// http://martinfowler.com/eaaDev/quantity.html
	enum zahlungsmodus Zahlungsmodus;// 1: Jährlich, 2: halbjährlich, 4: vierteljährlich
	enum laufzeit Laufzeit;			// 1 - 5 Jahre
	enum monate AbschlussMonat;		// 1 - 12
	unsigned int AbschlussJahr;		// ...wir haben ein Jahr-65.536-Problem...
	unsigned char GesamtAnzahlZahlungen; // = Laufzeit * Zahlungsmodus
	abrechnungstyp *GeleisteteZahlungenQueue;	//
	abrechnungstyp *OffeneZahlungenQueue; 		//
} vertragstyp;

// Funktionsprototypen

#endif /* VERTRAGSTYP_H_ */
