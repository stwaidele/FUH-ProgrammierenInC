/*
 ============================================================================
 Programm    : Verwaltung und Abrechnung von Versicherungsverträgen
 File        : versich.c
 Projekt     : Einsendearbeit zum Kurs Programmieren in C, SoSe 2016
 Author      : Waidele, Stefan, ToDo: Matrikelnummer
 Version     :
 Copyright   : (c) Stefan Waidele
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "abrechnungstyp.h"
#include "vertragstyp.h"
#include "versicherungstyp.h"

// Funktionsprototypen (die mehrere Includes benötigen)
vertragstyp *vertragstypConstructor(versicherungstyp *vers);

// Globale Variablen
versicherungstyp Versicherung;

int main(void) {
	versicherungstypInitialize(&Versicherung);

	vertragstyp *meinVertrag = vertragstypConstructor(&Versicherung);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Waidele");
		strcpy(meinVertrag->Vorname, "Stefan");
		strcpy(meinVertrag->Strasse, "Ensisheimer Straße 2");
		strcpy(meinVertrag->PLZ, "79395");
		strcpy(meinVertrag->Ort, "Neuenburg am Rhein");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 10000;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = februar;
		meinVertrag->AbschlussJahr = 2014;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(&Versicherung, meinVertrag);
		printf("%d\n", Versicherung.Vertrag[idx].VertragsID);
	}
	for (int d = 0; d < 3; d++) {
		meinVertrag = vertragstypConstructor(&Versicherung);
		if (meinVertrag) {
			int idx = addVertrag(&Versicherung, meinVertrag);
			printf("%d\n", Versicherung.Vertrag[idx].VertragsID);
		}
	}

	for (int c = 0; c < Versicherung.i; c++) {
		printf(
				"%d: %s, %s: Beitrag: %i; Laufzeit: %i; Zahlungsmodus: %i; ab %2i/%4i, %i Zahlungen \n",
				Versicherung.Vertrag[c].VertragsID,
				Versicherung.Vertrag[c].Name, Versicherung.Vertrag[c].Vorname,
				Versicherung.Vertrag[c].Jahresbeitrag,
				Versicherung.Vertrag[c].Laufzeit,
				Versicherung.Vertrag[c].Zahlungsmodus,
				Versicherung.Vertrag[c].AbschlussMonat,
				Versicherung.Vertrag[c].AbschlussJahr,
				Versicherung.Vertrag[c].GesamtAnzahlZahlungen);
		abrechnungstyp *a = Versicherung.Vertrag[c].OffeneZahlungenQueue;
		while (a != NULL) {
			printf("%5i, ", a->Betrag);
			a = a->next;
		}
		printf("\n");
	}

	puts("Hello world."); /* prints Hello world. */
	return EXIT_SUCCESS;
}

