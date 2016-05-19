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

//#include "vertragErfassen.c"
//#include "vertragAnzeigen.c"
//#include "kundenlisteAnzeigen.c"
//#include "beitragslisteAnzeigen.c"
//#include "zahlungErfassen.c"
//#include "abgelaufeneVertraegeLoeschen.c"


// Funktionsprototypen (die mehrere Includes benötigen)
vertragstyp *vertragstypConstructor(versicherungstyp *);
void zahlungsplan(vertragstyp *);
unsigned int addVertrag(versicherungstyp *, vertragstyp *);

void vertragErfassen(versicherungstyp *);
void vertragAnzeigen(versicherungstyp *);
void kundenlisteAnzeigen(versicherungstyp *);
void beitragslisteAnzeigen(versicherungstyp *);
void zahlungErfassen(versicherungstyp *) ;
void abgelaufeneVertraegeLoeschen(versicherungstyp *);



// Globale Variablen
//versicherungstyp Versicherung;

void clearScreen(void){
	/*
	 * Löscht nicht wirklich den Bildschriminhalt
	 * (siehe http://cboard.cprogramming.com/c-programming/23780-how-do-i-clear-screen-c-program.html )
	 * Sorgt aber für ausreichend visuellen Abstand vor den neuen Ausgaben.
	 *
	 */
	printf("\n\n\n\n\n\n");
}

void test(versicherungstyp *vers) {
	vertragstyp *meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Waidele");
		strcpy(meinVertrag->Vorname, "Stefan");
		strcpy(meinVertrag->Strasse, "Ensisheimer Straße 2");
		strcpy(meinVertrag->PLZ, "79395");
		strcpy(meinVertrag->Ort, "Neuenburg am Rhein");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 10001;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = februar;
		meinVertrag->AbschlussJahr = 2014;
		meinVertrag->GesamtAnzahlZahlungen
					= meinVertrag->Laufzeit * meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Stefan Waidele) hinzugefügt.");

	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Stark");
		strcpy(meinVertrag->Vorname, "Ayia");
		strcpy(meinVertrag->Strasse, "Hauptstraße 1");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Königsmund");
		strcpy(meinVertrag->Land, "SIEBEN KÖNIGSLANDE");
		meinVertrag->Jahresbeitrag = 100;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = mai;
		meinVertrag->AbschlussJahr = 2013;
		meinVertrag->GesamtAnzahlZahlungen
					= meinVertrag->Laufzeit * meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Arya Stark) hinzugefügt.");

	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Pond");
		strcpy(meinVertrag->Vorname, "Amelia");
		strcpy(meinVertrag->Strasse, "Mainstreet 17");
		strcpy(meinVertrag->PLZ, "12W 90N");
		strcpy(meinVertrag->Ort, "Leadworth");
		strcpy(meinVertrag->Land, "GREAT BRITAIN");
		meinVertrag->Jahresbeitrag = 10001;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 1;
		meinVertrag->AbschlussMonat = august;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen
					= meinVertrag->Laufzeit * meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Amelia Pond) hinzugefügt.");

}


int main(void) {
	versicherungstyp Versicherung;
	versicherungstypConstructor(&Versicherung);

	int exit=0;
	char input;
	do {
	clearScreen();

	printf("Versicherungsmanagement 00810\n");
	printf("=============================\n");
	printf("Hauptmenü\n");
	printf("----\n");
	printf(" 1  Vertrag erfassen\n");
	printf(" 2  Vertrag anzeigen\n");
	printf(" 3  Kundenliste anzeigen\n");
	printf(" 4  Beitragsliste anzeigen\n");
	printf(" 5  Beitragszahlung erfassen\n");
	printf(" 6  Abgelaufene Verträge löschen\n");
	printf("----\n");
	printf(" 7  Programm beenden\n");
	printf("\n");
	printf("Bitte wählen Sie (1…7): ");

	scanf(" %c", &input);
		switch(input) {
		case '1':
			vertragErfassen(&Versicherung);
			break;
		case '2':
			vertragAnzeigen(&Versicherung);
			break;
		case '3':
			kundenlisteAnzeigen(&Versicherung);
			break;
		case '4':
			beitragslisteAnzeigen(&Versicherung);
			break;
		case '5':
			zahlungErfassen(&Versicherung);
			break;
		case '6':
			abgelaufeneVertraegeLoeschen(&Versicherung);
			break;
		case 't':
			test(&Versicherung);
			break;
		case '7':
			exit = 1;
			break;
		default:
			exit=0;
		}
	} while(exit==0);

	// Todo: Versicherungsobjekt in Datei speichern

	return EXIT_SUCCESS;
}

