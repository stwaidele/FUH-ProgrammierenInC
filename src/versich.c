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
	offen, bezahlt
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
	unsigned int Betrag;// In Cent! http://martinfowler.com/eaaDev/quantity.html
	unsigned char FaelligMonat;		// 1 - 12
	unsigned int FaelligJahr;		// ...wir haben ein Jahr-65.536-Problem...
	enum zahlstatus Status;
} abrechnungstyp;

typedef struct vertragstyp {
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

typedef struct versicherungstyp {
	unsigned int currentVertragsID;
	unsigned int sizeOfVertragArray;
	unsigned int i;				// Poition des nächsten freien Feldes im Array
	vertragstyp *Vertrag;
} versicherungstyp;

/*
 * Prototypen
 */
int getNextVertragsID(versicherungstyp *);
unsigned int addVertrag(versicherungstyp *, vertragstyp *);
/*
 *  Implementation von Hilfsfunktionen
 */
void warteAufTaste() {
	printf("Bitte <enter> drücken um fortzufahren");
	getc(stdin);
}

void waitAndClearScreen(void) {
	/*
	 * Löscht nicht wirklich den Bildschriminhalt
	 * (siehe http://cboard.cprogramming.com/c-programming/23780-how-do-i-clear-screen-c-program.html )
	 * Sorgt aber für ausreichend visuellen Abstand vor den neuen Ausgaben.
	 */
	warteAufTaste();
	printf("\n\n\n\n\n\n");
}

/*
 * Verwaltungsfunktionen
 *
 * Direkter Bezug zur Aufgabenstellung „Versicherungsverwaltung“
 */

vertragstyp *vertragstypConstructor(versicherungstyp *vers) {
	vertragstyp *v = malloc(sizeof(*v));
	v->VertragsID = getNextVertragsID(vers);
	for (int i = 0; i < MAX_ZAHLUNGEN; i++) {
		v->Abrechnungen[i].Betrag = 0;
		v->Abrechnungen[i].FaelligJahr = 0;
		v->Abrechnungen[i].FaelligMonat = 0;
		v->Abrechnungen[i].Status = bezahlt;
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
		if (monat > 12) { // Überlauf…
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
	v->Abrechnungen[z - 1].Betrag = restBetrag;
}

void versicherungstypConstructor(versicherungstyp *vers) {
	/*
	 * Hier werden die Werte aus der Datei gelesen bzw. eine leere „Instanz“ initialisiert.
	 */
	vertragstyp *vertrag= malloc(sizeof(*vertrag));

	vers->currentVertragsID = 0;
	vers->sizeOfVertragArray = INITIAL_SIZE_OF_VERTRAG_ARRAY;
	vers->Vertrag = malloc(INITIAL_SIZE_OF_VERTRAG_ARRAY * sizeof(*vertrag));
	if (vers->Vertrag == 0) {
		printf("malloc fehlgeschlagen!\n");
		exit(EXIT_FAILURE);
	}
	vers->i = 0;

	FILE *fp;
	if ((fp = fopen("vertrag.txt", "rb"))) {
		// Erster Datensatz enthält die als nächstes zu vergebene VertragsID
		fread(vertrag, sizeof(vertragstyp), 1, fp);
		vers->currentVertragsID = vertrag->VertragsID;
		do {
			fread(vertrag, sizeof(vertragstyp), 1, fp);
			addVertrag(vers, vertrag);
		} while(!feof(fp));

	} else {
		printf("Datei 'vertrag.txt' nicht gefunden.\nLeere Vertragsdatenbank\n");
	}
}

unsigned int addVertrag(versicherungstyp *vers, vertragstyp *v) {
	// ToDo: Auf existierenden Kunden prüfen
	if (vers->i >= vers->sizeOfVertragArray) {
		// Array vergrößern: http://stackoverflow.com/questions/4694401/how-to-replicate-vector-in-c
		void* newMem = realloc(vers->Vertrag,
				2 * vers->sizeOfVertragArray * sizeof(*v));
		if (!newMem) {
			printf("Fehler bei der Reservierung von zusätzlichem Speicher\n");
			exit(EXIT_FAILURE);
		}
		vers->Vertrag = newMem;
		vers->sizeOfVertragArray = 2 * vers->sizeOfVertragArray;
		printf("Speicher Vergrößert auf %i.\n", vers->sizeOfVertragArray);
	}
	vers->Vertrag[vers->i] = *v;
	printf("Vertrag # %d an Stelle %d hinzugefügt (%s %s).\n", v->VertragsID,
			vers->i, v->Vorname, v->Name);
	return vers->i++;
}

void delVertrag(versicherungstyp *vers, unsigned int vid) {
	for (int i = 0; i < vers->i; i++) {
		if (vers->Vertrag[i].VertragsID == vid) {
			vers->Vertrag[i].VertragsID = 0;
		}
	} // for (Versicherungen)
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

/*
 * Hauptfunktionen
 *
 * Spiegeln die Menüoptionen wieder
 */
void vertragErfassen(versicherungstyp *vers) {
	printf("\nVertrag erfassen\n");
	printf("----------------\n");

	vertragstyp *meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		printf("Name: ");
		scanf("%s", meinVertrag->Name);
		getc(stdin); // Newline „weglesen“
		printf("Vorname: ");
		scanf("%s", meinVertrag->Vorname);
		getc(stdin); // Newline „weglesen“
		printf("Straße: ");
		scanf("%s", meinVertrag->Strasse);
		getc(stdin); // Newline „weglesen“
		printf("Hausnummer: ");
		scanf("%s", meinVertrag->Hausnummer);
		getc(stdin); // Newline „weglesen“
		printf("PLZ: ");
		scanf("%s", meinVertrag->PLZ);
		getc(stdin); // Newline „weglesen“
		printf("Ort: ");
		scanf("%s", meinVertrag->Ort);
		getc(stdin); // Newline „weglesen“
		printf("Land: ");
		scanf("%s", meinVertrag->Land);
		getc(stdin); // Newline „weglesen“
		printf("Jahresbeitrag in ¢: ");
		scanf("%d", &meinVertrag->Jahresbeitrag);
		getc(stdin); // Newline „weglesen“

		bool inputOK = false;
		int i;
		do {
			printf(
					"Zahlungsmodus (1: jährlich / 2: halbjährlich / 4: vierteljährlich): ");
			scanf("%d", &i);
			getc(stdin); // Newline „weglesen“
			switch (i) {
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
			getc(stdin); // Newline „weglesen“
			if ((meinVertrag->Laufzeit <= 5) && (meinVertrag->Laufzeit >= 1)) {
				inputOK = true;
			} else {
				printf(
						"Falsche Eingabe! Bitte nur 1, 2, 3, 4 oder 5 eingeben.\n");
				inputOK = false;
			}
		} while (!inputOK);

		do {
			printf("Abschlussdatum - Monat (1…12): ");
			scanf("%d", &(meinVertrag->AbschlussMonat));
			getc(stdin); // Newline „weglesen“
			if ((meinVertrag->AbschlussMonat <= 12)
					&& (meinVertrag->AbschlussMonat >= 1)) {
				inputOK = true;
			} else {
				printf(
						"Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
				inputOK = false;
			}
		} while (!inputOK);

		printf("Abschlussdatum - Jahr: ");
		getc(stdin); // Newline „weglesen“
		scanf("%d", &(meinVertrag->AbschlussJahr));
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		int idx = addVertrag(vers, meinVertrag);
		printf("%d\n", vers->Vertrag[idx].VertragsID);
		printf("Vertrag (%s %s) mit ID %d hinzugefügt.\n", meinVertrag->Vorname,
				meinVertrag->Name, meinVertrag->VertragsID);
	} else {
		puts("Fehler beim Erzeugen des Vertrags");
	}
	waitAndClearScreen();
	return;
}

void vertragAnzeigen(versicherungstyp *vers) {
	int vnum;
	bool ausgegeben = false;

	printf("\nVertrag anzeigen\n");
	printf("----------------\n");

	printf("Bitte geben Sie die Vertragsnummer ein (1…%d): ", vers->i);
	scanf("%i", &vnum);
	getc(stdin); // Newline „weglesen“

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
			for (int i = 0; i < MAX_ZAHLUNGEN; i++) {
				if (vers->Vertrag[c].Abrechnungen[i].Betrag > 0) {
					printf("%2d/%4d: %d ¢ ",
							vers->Vertrag[c].Abrechnungen[i].FaelligMonat,
							vers->Vertrag[c].Abrechnungen[i].FaelligJahr,
							vers->Vertrag[c].Abrechnungen[i].Betrag);
					if (vers->Vertrag[c].Abrechnungen[i].Status == bezahlt) {
						printf("(bezahlt), ");
					} else {
						printf("(offen  ), ");
					}
				}
			}
			printf("\n");
			ausgegeben = true;
		}
	}
	if (!ausgegeben) {
		printf("Kein Vertrag mit Nummer %d gefunden.\n", vnum);
	}
	waitAndClearScreen();
	return;
}

void kundenlisteAnzeigen(versicherungstyp *vers) {
	printf("\nKundenliste anzeigen\n");
	printf("--------------------\n");

	int size = 210;
	bool empty = true;

	char *temp;
	char *kunden[vers->i][size];
	temp = malloc(size + 1);
	/*
	 * Kundenliste erzeugen
	 */
	printf(
			"Name                      | Vorname                   | Vertrag #  | Straße                    | Haus  | PLZ        | Ort                       | Land                      | Abschluss | Jahresbeitrag\n");
	printf(
			"--------------------------+---------------------------+------------+---------------------------+-------+------------+---------------------------+---------------------------+-----------+------------------\n");
	for (int i = 0; i < vers->i; i++) {
		if (vers->Vertrag[i].VertragsID) {
			empty = false;
			snprintf(temp, size,
					"%-25s | %-25s | %10d | %-25s | %5s | %-10s | %-25s | %-25s |  %02d.%04d | %15d ¢",
					vers->Vertrag[i].Name, vers->Vertrag[i].Vorname,
					vers->Vertrag[i].VertragsID, vers->Vertrag[i].Strasse,
					vers->Vertrag[i].Hausnummer, vers->Vertrag[i].PLZ,
					vers->Vertrag[i].Ort, vers->Vertrag[i].Land,
					vers->Vertrag[i].AbschlussMonat,
					vers->Vertrag[i].AbschlussJahr,
					vers->Vertrag[i].Jahresbeitrag);
			strcpy(kunden[i], temp);
		} else {
			strcpy(kunden[i], "");
		} // if(VertragsID)
	} // for(Verträge)

	if (empty) {
		printf("Keine Aktiven Verträge\n");
	} else {
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
		int j = 0;
		for (int i = 0; i < vers->i; i++) {
			printf("%s\n", kunden[i]);
			j++;
			if (j % 15 == 0) {
				warteAufTaste();
			}
		}
	}
	free(temp);
	waitAndClearScreen();
	return;
}

void beitragslisteAnzeigen(versicherungstyp *vers) {
	printf("\nBeitragsliste anzeigen\n");
	printf("----------------------\n");

	int size = 85;
	bool empty = true;

	char *temp;
	char *beitraege[(vers->i) * 20][size]; // Maximal 20 Beiträge pro Versicherung können fällig sein. (Ende der Vertragslaufzeit, keine geleisteten Zahlungen)
	temp = malloc(size + 1);

	/*
	 * Monat und Jahr zur Fälligkeitsbestimmung einlesen
	 */
	bool inputOK = false;
	enum monate monat;		// 1 - 12
	unsigned int jahr;		// ...wir haben ein Jahr-65.536-Problem...

	do {
		printf("Bewertung zum Datum - Monat (1…12): ");
		scanf("%d", &monat);
		getc(stdin); // Newline „weglesen“
		if ((monat <= 12) && (monat >= 1)) {
			inputOK = true;
		} else {
			printf(
					"Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
			inputOK = false;
		}
	} while (!inputOK);

	printf("Bewertung zum Datum - Jahr: ");
	scanf("%d", &jahr);
	getc(stdin); // Newline „weglesen“

	printf(
			"Vertrag #  | Name                      | Vorname                   | Betrag in ¢\n");
	printf(
			"-----------+---------------------------+---------------------------+--------------\n");
	/*
	 * Beitragsliste erzeugen
	 */
	for (int i = 0; i < vers->i; i++) {
		for (int j = 0; j < 20; j++) {
			if ((vers->Vertrag[i].Abrechnungen[j].Betrag > 0)
					&& (vers->Vertrag[i].Abrechnungen[j].FaelligJahr == jahr)
					&& (vers->Vertrag[i].Abrechnungen[j].FaelligMonat == monat)
					&& (vers->Vertrag[i].Abrechnungen[j].Status != bezahlt)) {
				empty = false;
				snprintf(temp, size, "%10d | %-25s | %-25s | %10d ¢",
						vers->Vertrag[i].VertragsID, vers->Vertrag[i].Name,
						vers->Vertrag[i].Vorname,
						vers->Vertrag[i].Abrechnungen[j].Betrag);
				strcpy(beitraege[i], temp);
			}
		}
	}
	if (empty) {
		printf("Keine fälligen Beiträge für diesen Zeitraum\n");
	} else {
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
		int j = 0;

		for (int i = 0; i < vers->i; i++) {
			if (strlen(beitraege[i]) > 0) {
				printf("%s\n", beitraege[i]);
				j++;
				if (j % 15 == 0) {
					warteAufTaste();
				}
			}
		}
	} // if (empty) else
	free(temp);
	waitAndClearScreen();
	return;
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
	getc(stdin); // Newline „weglesen“

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
				getc(stdin); // Newline „weglesen“
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
			getc(stdin); // Newline „weglesen“

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
	waitAndClearScreen();
	return;
}

void abgelaufeneVertraegeLoeschen(versicherungstyp *vers) {
	printf("\nAbgelaufene Verträge löschen\n");
	printf("----------------------------\n");

	/*
	 * Monat und Jahr zur Fälligkeitsbestimmung einlesen
	 */
	bool inputOK = false;
	enum monate monat;		// 1 - 12
	unsigned int jahr;		// ...wir haben ein Jahr-65.536-Problem...

	do {
		printf("Ablaufdatum - Monat (1…12): ");
		scanf("%d", &monat);
		getc(stdin); // Newline „weglesen“
		if ((monat <= 12) && (monat >= 1)) {
			inputOK = true;
		} else {
			printf(
					"Falsche Eingabe! Bitte nur 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 oder 12 eingeben.\n");
			inputOK = false;
		}
	} while (!inputOK);

	printf("Ablaufdatum - Jahr: ");
	scanf("%d", &jahr);
	getc(stdin); // Newline „weglesen“

	int anzAbgelaufen = 0;

	for (int i = 0; i < vers->i; i++) {
		if (((vers->Vertrag[i].AbschlussJahr + vers->Vertrag[i].Laufzeit) < jahr) // Ablauf aus Jahreszahl erkennbar
				|| (((vers->Vertrag[i].AbschlussJahr + vers->Vertrag[i].Laufzeit)
						== jahr) // Oder muss der Monat betrachtet werden
				&& (vers->Vertrag[i].AbschlussMonat < monat))) {
			/*
			 * Der Vertrag ist abgelaufen, wurde er auch komplett bezahlt?
			 */
			bool komplettBezahlt = true;
			for (int j = 0; j < 20; j++) {
				if (vers->Vertrag[i].Abrechnungen[j].Status == offen) {
					komplettBezahlt = false;
				} // if (!bezahlt)
			} // for (Abrechnungen)
			/*
			 * Abgelaufen und bezahlt -> Vertrag löschen
			 */
			if (komplettBezahlt) {
				printf(
						"Vertrag %d abgelaufen und komplett bezahlt wird gelöscht.\n",
						vers->Vertrag[i].VertragsID);
				delVertrag(vers, vers->Vertrag[i].VertragsID);
				anzAbgelaufen++;
			} else {
				printf(
						"Vertrag %d abgelaufen aber nicht komplett bezahlt. NICHT GELÖSCHT.\n",
						vers->Vertrag[i].VertragsID);
			} // if(komplettBezahlt)
		} // if (Abgelaufen)
	} // for (Verträge)
	if (anzAbgelaufen == 0) {
		printf("Keine abgelaufenen Verträge zum angegebenen Datum gefunden.\n");
	}
	waitAndClearScreen();
	return;
}

void test(versicherungstyp *vers) {
	/*
	 * Erzeugen von Verträgen zu Testzwecken
	 *
	 * Kann im Hauptmenü über die nicht angezeigte Option „t“ aufgerufen werden.
	 */

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
		addVertrag(vers, meinVertrag);
	}
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
		addVertrag(vers, meinVertrag);
	}
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
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Meier");
		strcpy(meinVertrag->Vorname, "Rudolf");
		strcpy(meinVertrag->Strasse, "Astraße");
		strcpy(meinVertrag->Hausnummer, "1");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 1;
		meinVertrag->AbschlussMonat = juni;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Meyer");
		strcpy(meinVertrag->Vorname, "Quasimodo");
		strcpy(meinVertrag->Strasse, "Bstraße");
		strcpy(meinVertrag->Hausnummer, "2");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 1;
		meinVertrag->AbschlussMonat = mai;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Mayer");
		strcpy(meinVertrag->Vorname, "Petra");
		strcpy(meinVertrag->Strasse, "Dstraße");
		strcpy(meinVertrag->Hausnummer, "3");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 1;
		meinVertrag->AbschlussMonat = april;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Müller");
		strcpy(meinVertrag->Vorname, "Oskar");
		strcpy(meinVertrag->Strasse, "Einestraße");
		strcpy(meinVertrag->Hausnummer, "4");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 2;
		meinVertrag->AbschlussMonat = maerz;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Schultze");
		strcpy(meinVertrag->Vorname, "Norbert");
		strcpy(meinVertrag->Strasse, "Parkstraße");
		strcpy(meinVertrag->Hausnummer, "5");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 2;
		meinVertrag->AbschlussMonat = februar;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Schulze");
		strcpy(meinVertrag->Vorname, "Maria");
		strcpy(meinVertrag->Strasse, "Schlossallee");
		strcpy(meinVertrag->Hausnummer, "6");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 2;
		meinVertrag->AbschlussMonat = januar;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Meier");
		strcpy(meinVertrag->Vorname, "Luise");
		strcpy(meinVertrag->Strasse, "Poststraße");
		strcpy(meinVertrag->Hausnummer, "7");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = dezember;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Meyer");
		strcpy(meinVertrag->Vorname, "Klaus");
		strcpy(meinVertrag->Strasse, "Dudenstraße");
		strcpy(meinVertrag->Hausnummer, "8");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = november;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Maier");
		strcpy(meinVertrag->Vorname, "Jürgen");
		strcpy(meinVertrag->Strasse, "Teslastraße");
		strcpy(meinVertrag->Hausnummer, "9");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = oktober;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Mayer");
		strcpy(meinVertrag->Vorname, "Ingrid");
		strcpy(meinVertrag->Strasse, "Adastraße");
		strcpy(meinVertrag->Hausnummer, "10");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 4;
		meinVertrag->AbschlussMonat = september;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Müller");
		strcpy(meinVertrag->Vorname, "Heidi");
		strcpy(meinVertrag->Strasse, "Pascalstraße");
		strcpy(meinVertrag->Hausnummer, "11");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 4;
		meinVertrag->AbschlussMonat = august;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Schultze");
		strcpy(meinVertrag->Vorname, "Gerald");
		strcpy(meinVertrag->Strasse, "Neumannstraße");
		strcpy(meinVertrag->Hausnummer, "12");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 4;
		meinVertrag->AbschlussMonat = juli;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Schulze");
		strcpy(meinVertrag->Vorname, "Fiona");
		strcpy(meinVertrag->Strasse, "Turingstraße");
		strcpy(meinVertrag->Hausnummer, "13");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 2;
		meinVertrag->AbschlussMonat = juni;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Bauer");
		strcpy(meinVertrag->Vorname, "Egon");
		strcpy(meinVertrag->Strasse, "Boolstraße");
		strcpy(meinVertrag->Hausnummer, "14");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 3;
		meinVertrag->AbschlussMonat = mai;
		meinVertrag->AbschlussJahr = 2016;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Baur");
		strcpy(meinVertrag->Vorname, "Doris");
		strcpy(meinVertrag->Strasse, "Javaboulevard");
		strcpy(meinVertrag->Hausnummer, "15");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 4;
		meinVertrag->AbschlussMonat = april;
		meinVertrag->AbschlussJahr = 2017;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Bauer");
		strcpy(meinVertrag->Vorname, "Celine");
		strcpy(meinVertrag->Strasse, "Perlstraße");
		strcpy(meinVertrag->Hausnummer, "16");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 234217;
		meinVertrag->Zahlungsmodus = jaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = maerz;
		meinVertrag->AbschlussJahr = 2010;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Baur");
		strcpy(meinVertrag->Vorname, "Bruno");
		strcpy(meinVertrag->Strasse, "Quarkstraße");
		strcpy(meinVertrag->Hausnummer, "17");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 471100;
		meinVertrag->Zahlungsmodus = halbjaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = februar;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	meinVertrag = vertragstypConstructor(vers);
	if (meinVertrag) {
		strcpy(meinVertrag->Name, "Hoffmann");
		strcpy(meinVertrag->Vorname, "Axel");
		strcpy(meinVertrag->Strasse, "Bahnhofstraße");
		strcpy(meinVertrag->Hausnummer, "18");
		strcpy(meinVertrag->PLZ, "12345");
		strcpy(meinVertrag->Ort, "Berlin");
		strcpy(meinVertrag->Land, "GERMANY");
		meinVertrag->Jahresbeitrag = 12345;
		meinVertrag->Zahlungsmodus = vierteljaehrlich;
		meinVertrag->Laufzeit = 5;
		meinVertrag->AbschlussMonat = januar;
		meinVertrag->AbschlussJahr = 2015;
		meinVertrag->GesamtAnzahlZahlungen = meinVertrag->Laufzeit
				* meinVertrag->Zahlungsmodus;
		zahlungsplan(meinVertrag);
		addVertrag(vers, meinVertrag);
	}
	waitAndClearScreen();
}

int alleDatenSpeichern(versicherungstyp *vers) {
	bool allesIstGut = true;

	FILE *fp;
	vertragstyp nummernull;

	nummernull.VertragsID = vers->currentVertragsID;

	if ((fp = fopen("vertrag.txt", "wb"))) {
		if (!fwrite(&nummernull, sizeof(vertragstyp), 1, fp)) {
			allesIstGut = false;
		}
		for (int i = 0; i < vers->i; i++) {
			if (vers->Vertrag[i].VertragsID) { // ID <>0 -> nicht gelöscht
				if (fwrite(&(vers->Vertrag[i]), sizeof(vertragstyp), 1, fp)) {
					printf(".");
				} else { // if (write vertrag)
					allesIstGut = false;
				}
			}
		} // for (Vertrag[i]
	} else { // if (open)
		printf(
				"Fehler beim Öffnen der Datei. ");
		waitAndClearScreen();
		allesIstGut = false;
	} // if (open)

	if (allesIstGut) {
		printf("\nDatei gespeichert.\n");
		return (EXIT_SUCCESS);
	} else {
		printf("\nFehler beim Speichern. We appologize for your loss of data…\n");
		return (EXIT_FAILURE);
	}
}

/*
 * main()
 *
 * Implementation des Hauptmenüs
 */

int main(void) {
	versicherungstyp Versicherung;
	versicherungstypConstructor(&Versicherung);

	int exit = 0;
	char input;
	do {
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
		getc(stdin); // Newline „weglesen“
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

	return alleDatenSpeichern(&Versicherung);
}

