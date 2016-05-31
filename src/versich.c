/*
 ============================================================================
 Programm    : Verwaltung und Abrechnung von Versicherungsverträgen
 File        : versich.c
 Projekt     : Einsendearbeit zum Kurs Programmieren in C, SoSe 2016
 Author      : Waidele, Stefan, #3794890
 Version     :
 Copyright   : (c) Stefan Waidele
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define INITIAL_SIZE_OF_VERTRAG_ARRAY 10
#define MAX_ZAHLUNGEN 20

enum zahlstatus {
	offen, faellig, bezahlt
};
enum zahlungsmodus {
	jaehrlich = 1, halbjaehrlich = 2, vierteljaehrlich = 4
};
enum laufzeit {
	l1 = 1, l2 = 2, l3 = 3, l4 = 4, l5 = 5
};
enum monate {
	keinmonat = 0,
	januar = 1,
	februar = 2,
	maerz = 3,
	april = 4,
	mai = 5,
	juni = 6,
	juli = 7,
	august = 8,
	september = 9,
	oktober = 10,
	november = 11,
	dezember = 12
};

typedef struct abrechnungstyp {
	unsigned int Betrag;		// http://martinfowler.com/eaaDev/quantity.html
	unsigned char FaelligMonat;		// 1 - 12
	unsigned int FaelligJahr;		// ...wir haben ein Jahr-65.536-Problem...
	enum zahlstatus Status;
} abrechnungstyp;


typedef struct {
	unsigned int VertragsID;
	char Name[25];
	char Vorname[25];
	char Strasse[25];
	char Hausnummer[5];
	char PLZ[10];				// https://de.wikipedia.org/wiki/Postleitzahl
	char Ort[25];
	char Land[25];// https://de.wikipedia.org/wiki/L%C3%A4ndercodes_im_Briefdienst
	unsigned int Jahresbeitrag;	// http://martinfowler.com/eaaDev/quantity.html
	enum zahlungsmodus Zahlungsmodus;// 1: Jährlich, 2: halbjährlich, 4: vierteljährlich
	enum laufzeit Laufzeit;			// 1 - 5 Jahre
	enum monate AbschlussMonat;		// 1 - 12
	unsigned int AbschlussJahr;		// ...wir haben ein Jahr-65.536-Problem...
	unsigned char GesamtAnzahlZahlungen; // = Laufzeit * Zahlungsmodus
	abrechnungstyp Abrechnungen[MAX_ZAHLUNGEN]; // 5 Jahre * 4 Zahlungen pro Jahr => max. 20 Abrechnungen
} vertragstyp;

typedef struct {
	unsigned int currentVertragsID;
	unsigned int sizeOfVertragArray;
	unsigned int i;				// Poition des nächsten freien Feldes im Array
	vertragstyp *Vertrag;
} versicherungstyp;

/*
 * Prototypen
 */
int getNextVertragsID(versicherungstyp *);


vertragstyp *vertragstypConstructor(versicherungstyp *vers) {
	vertragstyp *v = malloc(sizeof(*v));
	v->VertragsID = getNextVertragsID(vers);
	for(int i=0; i<MAX_ZAHLUNGEN; i++) {
		v->Abrechnungen[i].Betrag = 0;
		v->Abrechnungen[i].FaelligJahr = 0;
		v->Abrechnungen[i].FaelligMonat = 0;
		v->Abrechnungen[i].Status = offen;
	}
	return v;
}

void zahlungsplan(vertragstyp *v) {
	int betragProAbrechnung;
	int restBetrag;
	/*
	 * Zahlbeträge ermitteln
	 */
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
	 * Zahlbeträge & Fälligkeiten zuweisen
	 */
	int monat, jahr, delta;
	jahr = v->AbschlussJahr;
	monat = v->AbschlussMonat - 1; // Um 1 reduziert, da Abschlussmonat erster Monat im Abrechnungszeitraum
	delta = 12 / v->Zahlungsmodus;

	int z = 0;
	while (z < v->GesamtAnzahlZahlungen) {
		monat += delta;	//Monat um "Delta" erhöhen
		if (monat>12) { // Überlauf…
			monat -= 12; // …Monat verringern
			jahr++;	// …Jahr hochzählen
		}
		v->Abrechnungen[z].Betrag = betragProAbrechnung;
		v->Abrechnungen[z].FaelligMonat = monat;
		v->Abrechnungen[z].FaelligJahr = jahr;
		v->Abrechnungen[z].Status = offen;
		z++;
	}
	/*
	 * Nach dem letzten Schleifendurchlauf ist der Betrag entweder
	 * korrekt (falls jede Zahlung gleich groß ist,
	 * oder inkorrekt (falls die letzte Zahlung kleiner ist).
	 *
	 * Korrektur des letzten Zahlbetrags ist somit
	 * evt. nicht notwendig, aber nie schädlich.
	 */
	v->Abrechnungen[z-1].Betrag = restBetrag;
}

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
	// ToDo: Auf existierenden Kunden prüfen
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

void vertragErfassen(versicherungstyp *vers) {
	printf("\nVertrag erfassen\n");
	printf("----------------\n");

	vertragstyp *meinVertrag = vertragstypConstructor(vers);
		if (meinVertrag) {
			printf("Name: ");
			scanf("%s", meinVertrag->Name);
			printf("Vorname: ");
			scanf("%s", meinVertrag->Vorname);
			printf("Straße: ");
			scanf("%s", meinVertrag->Strasse);
			printf("PLZ: ");
			scanf("%s", meinVertrag->PLZ);
			printf("Ort: ");
			scanf("%s", meinVertrag->Ort);
			printf("Land: ");
			scanf("%s", meinVertrag->Land);
			printf("Jahresbeitrag in ¢: ");
			scanf("%d", &meinVertrag->Jahresbeitrag);

			bool inputOK = false;
			int i;
			do {
				printf("Zahlungsmodus (1: jährlich / 2: halbjährlich / 4: vierteljährlich): ");
				scanf("%d", &i);
				switch(i) {
				case 1:
					meinVertrag->Zahlungsmodus = jaehrlich;
					inputOK = true;
					break;
				case 2:
					meinVertrag->Zahlungsmodus = halbjaehrlich;
					inputOK = true;
					break;
				case 4:
					meinVertrag->Zahlungsmodus = vierteljaehrlich;
					inputOK = true;
					break;
				default:
					printf("Falsche Eingabe! Bitte nur 1, 2 oder 4 eingeben.\n");
					inputOK = false;
				}
			} while (!inputOK);

			do {
				printf("Laufzeit in Jahren (1…5): ");
				scanf("%d", &(meinVertrag->Laufzeit));
				if ((meinVertrag->Laufzeit <= 5) && (meinVertrag->Laufzeit >= 1)) {
					inputOK = true;
				} else {
					printf("Falsche Eingabe! Bitte nur 1, 2, 3, 4 oder 5 eingeben.\n");
					inputOK=false;
				}
			} while(!inputOK);

			do {
				printf("Abschlussdatum - Monat (1…12): ");
				scanf("%d", &(meinVertrag->AbschlussMonat));
				if ((meinVertrag->AbschlussMonat <= 12) && (meinVertrag->AbschlussMonat >= 1)) {
					inputOK = true;
				} else {
					printf("Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
					inputOK=false;
				}
			} while(!inputOK);

			printf("Abschlussdatum - Jahr: ");
			scanf("%d", &(meinVertrag->AbschlussJahr));
			meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
												* meinVertrag->Zahlungsmodus;
			zahlungsplan(meinVertrag);
			int idx = addVertrag(vers, meinVertrag);
			printf("%d\n", vers->Vertrag[idx].VertragsID);
			printf("Vertrag (%s %s) mit ID %d hinzugefügt.\n", meinVertrag->Vorname, meinVertrag->Name, meinVertrag->VertragsID);
		} else {
			puts("Fehler beim Erzeugen des Vertrags");
		}
}

void vertragAnzeigen(versicherungstyp *vers) {
	int vnum;
	bool ausgegeben = false;

	printf("\nVertrag anzeigen\n");
	printf("----------------\n");

	printf("Bitte geben Sie die Vertragsnummer ein (1…%d): ", vers->i);
	scanf("%i", &vnum);

	for (int c = 0; c < vers->i; c++) {
		if (vers->Vertrag[c].VertragsID == vnum) {
			printf(
					"%d: %s, %s: Jahresbeitrag: %i; Laufzeit: %i; Zahlungsmodus: %i; ab %2i/%4i, %i Zahlungen \n",
					vers->Vertrag[c].VertragsID, vers->Vertrag[c].Name,
					vers->Vertrag[c].Vorname, vers->Vertrag[c].Jahresbeitrag,
					vers->Vertrag[c].Laufzeit, vers->Vertrag[c].Zahlungsmodus,
					vers->Vertrag[c].AbschlussMonat,
					vers->Vertrag[c].AbschlussJahr,
					vers->Vertrag[c].GesamtAnzahlZahlungen);
			for(int i=0; i<MAX_ZAHLUNGEN; i++) {
				if(vers->Vertrag[c].Abrechnungen[i].Betrag>0) {
					printf("%2d/%4d: %d¢, ", vers->Vertrag[c].Abrechnungen[i].FaelligMonat,
											 vers->Vertrag[c].Abrechnungen[i].FaelligJahr,
											 vers->Vertrag[c].Abrechnungen[i].Betrag);
				}
			}
			printf("\n");
			ausgegeben = true;
		}
	}
	if (!ausgegeben) {
		printf("Kein Vertrag mit Nummer %d gefunden.\n", vnum);
	}
}

void kundenlisteAnzeigen(versicherungstyp *vers) {
	printf("\nKundenliste anzeigen\n");
	printf("--------------------\n");

	vertragstyp *vertrag; // Benötigt für sizeof() ToDo: Wirklich?
	int size = sizeof(*vertrag); // Nur eine Schätzung der Größe. Es werden nicht alle Felder ausgegeben, dafür aber Füll- und Trennzeichen

	char *temp;
	char *kunden[vers->i][size];
	temp = malloc(size+1);
	/*
	 * Kundenliste erzeugen
	 */
	for(int i=0; i<vers->i; i++) {
		snprintf(temp, size,"%s, %s, Vertrag #%d, %s %s, %s %s %s, seit %02d.%04d, %d¢pro Jahr.", vers->Vertrag[i].Name,
												  vers->Vertrag[i].Vorname,
												  vers->Vertrag[i].VertragsID,
												  vers->Vertrag[i].Strasse,
												  vers->Vertrag[i].Hausnummer,
												  vers->Vertrag[i].PLZ,
												  vers->Vertrag[i].Ort,
												  vers->Vertrag[i].Land,
												  vers->Vertrag[i].AbschlussMonat,
												  vers->Vertrag[i].AbschlussJahr,
												  vers->Vertrag[i].Jahresbeitrag
				);
		strcpy(kunden[i], temp);
	}
	/*
	 * Kundenliste sortieren
	 * Bubblesort mit Dreieckstausch,
	 * z.B. http://www.c4learn.com/c-programs/c-program-for-sorting-the-list-of-strings.html
	 */

	   for (int i = 0; i < vers->i; i++) {
	      for (int j = 0; j < vers->i - 1; j++) {
	         if (strcmp(kunden[j], kunden[j + 1]) > 0) {
	            strcpy(temp, kunden[j]);
	            strcpy(kunden[j], kunden[j + 1]);
	            strcpy(kunden[j + 1], temp);
	         }
	      }
	   }

	/*
	 * Kundenliste ausgeben
	 */
	for(int i=0; i<vers->i; i++) {
		printf("%s\n", kunden[i]);
	}
	free(temp);
	// free(kunden);
}

void beitragslisteAnzeigen(versicherungstyp *vers) {
	printf("\nBeitragsliste anzeigen\n");
	printf("----------------------\n");

	vertragstyp *vertrag; // Benötigt für sizeof() ToDo: Wirklich?
	int size = sizeof(*vertrag); // Nur eine Schätzung der Größe. Es werden nicht alle Felder ausgegeben, dafür aber Füll- und Trennzeichen

	char *temp;
	char *beitraege[(vers->i)*20][size]; // Maximal 20 Beiträge pro Versicherung können fällig sein. (Ende der Vertragslaufzeit, keine geleisteten Zahlungen)
	temp = malloc(size+1);

	/*
	 * Monat und Jahr zur Fälligkeitsbestimmung einlesen
	 */
	bool inputOK = false;
	enum monate monat;		// 1 - 12
	unsigned int jahr;		// ...wir haben ein Jahr-65.536-Problem...

	do {
		printf("Bewertung zum Datum - Monat (1…12): ");
		scanf("%d", &monat);
		if ((monat <= 12) && (monat >= 1)) {
			inputOK = true;
		} else {
			printf("Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
			inputOK=false;
		}
	} while(!inputOK);

	printf("Bewertung zum Datum - Jahr: ");
	scanf("%d", &jahr);

	/*
	 * Beitragsliste erzeugen
	 */
	for(int i=0; i<vers->i; i++) {
		for(int j=0; j<20; j++) {
			if( (vers->Vertrag[i].Abrechnungen[j].Betrag > 0)
				&& (vers->Vertrag[i].Abrechnungen[j].FaelligJahr == jahr )
				&& (vers->Vertrag[i].Abrechnungen[j].FaelligMonat == monat )
				&& (vers->Vertrag[i].Abrechnungen[j].Status != bezahlt )
			  ) {
				snprintf(temp, size,"Vertrag #%d, %s, %s, %d¢.",
												  vers->Vertrag[i].VertragsID,
												  vers->Vertrag[i].Name,
												  vers->Vertrag[i].Vorname,
												  vers->Vertrag[i].Abrechnungen[j].Betrag
						);
				strcpy(beitraege[i], temp);
			}
		}
	}
	/*
	 * Beitragsliste sortieren
	 * Bubblesort mit Dreieckstausch,
	 * z.B. http://www.c4learn.com/c-programs/c-program-for-sorting-the-list-of-strings.html
	 */

	   for (int i = 0; i < vers->i; i++) {
	      for (int j = 0; j < vers->i - 1; j++) {
	         if (strcmp(beitraege[j], beitraege[j + 1]) > 0) {
	            strcpy(temp, beitraege[j]);
	            strcpy(beitraege[j], beitraege[j + 1]);
	            strcpy(beitraege[j + 1], temp);
	         }
	      }
	   }

	/*
	 * Beitragsliste ausgeben
	 */
	for(int i=0; i<vers->i; i++) {
		if (strlen(beitraege[i])>0) {
			printf("%s\n", beitraege[i]);
		}
	}
	free(temp);
}

void zahlungErfassen(versicherungstyp *vers) {
	printf("\nZahlung erfassen\n");

	int vnum;
	bool vertragGefunden = false;
	bool abrechnungGefunden = false;
	enum monate monat = 0;		// 1 - 12
	unsigned int jahr = 0;		// ...wir haben ein Jahr-65.536-Problem...

	printf("Bitte geben Sie die Vertragsnummer ein (1…%d): ", vers->i);
	scanf("%i", &vnum);

	for (int i = 0; i < vers->i; i++) {
		if (vers->Vertrag[i].VertragsID == vnum) {
			vertragGefunden = true;

			/*
			 * Monat und Jahr zur Fälligkeitsbestimmung einlesen
			 */
			bool inputOK = false;

			do {
				printf("Zahlung zum Datum - Monat (1…12): ");
				scanf("%d", &monat);
				if ((monat <= 12) && (monat >= 1)) {
					inputOK = true;
				} else {
					printf(
							"Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
					inputOK = false;
				}
			} while (!inputOK);

			printf("Zahlung zum Datum - Jahr: ");
			scanf("%d", &jahr);

			for (int j = 0; j < 20; j++) {
				if ((vers->Vertrag[i].Abrechnungen[j].Betrag > 0)
						&& (vers->Vertrag[i].Abrechnungen[j].FaelligJahr == jahr)
						&& (vers->Vertrag[i].Abrechnungen[j].FaelligMonat
								== monat)) {
					abrechnungGefunden = true;
					if (vers->Vertrag[i].Abrechnungen[j].Status == bezahlt) {
						printf(
								"Zahlung für Vertrag %d in %2d.%4d war bereits geleistet.\nKEINE BUCHUNG DURCHGEFÜHRT.\n",
								vers->Vertrag[i].VertragsID,
								vers->Vertrag[i].Abrechnungen[j].FaelligMonat,
								vers->Vertrag[i].Abrechnungen[j].FaelligJahr);
					} else {
						vers->Vertrag[i].Abrechnungen[j].Status = bezahlt;
						printf(
								"Zahlung für Vertrag %d in %2d.%4d durchgeführt.\n",
								vnum, monat, jahr);
					}
				}
			}
			if (!abrechnungGefunden) {
						printf(
								"Keine vorgesehene Zahlung für Vertrag %d in %2d.%4d.\nKEINE BUCHUNG DURCHGEFÜHRT.\n",
								vnum, monat, jahr);
			}
		}

	}
	if (!vertragGefunden) {
		printf("Kein Vertrag mit Nummer %d gefunden.\n", vnum);
	}
}

void abgelaufeneVertraegeLoeschen(versicherungstyp *vers) {
	printf("\nAbgelaufene Verträge löschen\n");
}

/*
 *  Implementation von Hilfsfunktionen
 */

void clearScreen(void) {
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
		strcpy(meinVertrag->Strasse, "Ensisheimer Straße");
		strcpy(meinVertrag->Hausnummer, "2");
		strcpy(meinVertrag->PLZ, "79395");
		strcpy(meinVertrag->Ort, "Neuenburg am Rhein");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 10001;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = februar;
		meinVertrag->AbschlussJahr = 2014;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Stefan Waidele) hinzugefügt.");

	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Stark");
		strcpy(meinVertrag->Vorname, "Ayia");
		strcpy(meinVertrag->Strasse, "Hauptstraße");
		strcpy(meinVertrag->Hausnummer, "1");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Königsmund");
		strcpy(meinVertrag->Land, "SIEBEN KÖNIGSLANDE");
		meinVertrag->Jahresbeitrag = 100;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = mai;
		meinVertrag->AbschlussJahr = 2013;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Arya Stark) hinzugefügt.");

	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Pond");
		strcpy(meinVertrag->Vorname, "Amelia");
		strcpy(meinVertrag->Strasse, "Mainstreet");
		strcpy(meinVertrag->Hausnummer, "17");
		strcpy(meinVertrag->PLZ, "12W 90N");
		strcpy(meinVertrag->Ort, "Leadworth");
		strcpy(meinVertrag->Land, "GREAT BRITAIN");
		meinVertrag->Jahresbeitrag = 10001;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 1;
		meinVertrag->AbschlussMonat = august;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
	}
	puts("Vertrag (Amelia Pond) hinzugefügt.");

}

/*
 * Implementation "main" - Menü
 */

int main(void) {
	versicherungstyp Versicherung;
	versicherungstypConstructor(&Versicherung);

	int exit = 0;
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
		switch (input) {
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
			exit = 0;
		}
	} while (exit == 0);

	// Todo: Versicherungsobjekt in Datei speichern

	return EXIT_SUCCESS;
}

