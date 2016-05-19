/*
 * vertragstyp.c
 *
 *  Created on: 04.04.2016
 *      Author: stwaidele
 *
 *  Der Datentyp „vertragstyp“ bildet die geschlossenen Versicherungsverträge
 *  ab.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#include "abrechnungstyp.h"
#include "vertragstyp.h"
#include "versicherungstyp.h"

vertragstyp *vertragstypConstructor(versicherungstyp *vers) {
	vertragstyp *v = malloc(sizeof(*v));
	v->VertragsID = getNextVertragsID(vers);
	v->OffeneZahlungenQueue = NULL;
	v->GeleisteteZahlungenQueue = NULL;
	return v;
}

void zahlungsplan(vertragstyp *v) {
	abrechnungstyp *current;
	abrechnungstyp *prev;

	int betragProAbrechnung;
	int restBetrag;

	if ((v->Jahresbeitrag * v->Laufzeit) % v->GesamtAnzahlZahlungen) {
		// Division mit Rest: Beträge so festlegen, dass die Abschlussrate die kleinste Rate ist.
		betragProAbrechnung = 1
				+ (v->Jahresbeitrag * v->Laufzeit) / v->GesamtAnzahlZahlungen;
		restBetrag = (v->Jahresbeitrag * v->Laufzeit)
				- betragProAbrechnung * (v->GesamtAnzahlZahlungen - 1);
	} else {
		// Kein Rest bei der Division, alle Zahlungen gleich
		betragProAbrechnung = (v->Jahresbeitrag * v->Laufzeit)
				/ v->GesamtAnzahlZahlungen;
	}

	/*
	 * Erste Abrechnung
	 * mit initialisierung der verknüpften Liste
	 */
	current = malloc(sizeof(*current));
	current->Betrag = betragProAbrechnung;
	v->OffeneZahlungenQueue = current;
	prev = current;

	// Alle weiteren Abrechnungen
	int z = 1;
	while (z < v->GesamtAnzahlZahlungen) {
		/*
		 * Schleife von 1  da die erste Zahlung (Nr. 0) bereits gespeichert ist.
		 * Nach dem letzten Schleifendurchlauf ist der Betrag entweder
		 * korrekt (falls jede Zahlung gleich groß ist,
		 * oder inkorrekt (falls die letzte Zahlung kleiner ist).
		 */
		current = malloc(sizeof(*current));
		// ToDo: Fälligkeitsdatum berechnen
		current->Betrag = betragProAbrechnung;
		prev->next = current;
		prev = current;
		z++;
	}
	/*
	 * Korrektur des letzten Zahlbetrags.
	 * Evt. nicht notwendig, aber nie schädlich.
	 */
	current->Betrag = restBetrag;
}
