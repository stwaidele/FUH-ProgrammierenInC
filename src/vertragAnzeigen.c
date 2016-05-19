#include <stdio.h>

#include "abrechnungstyp.h"
#include "vertragstyp.h"
#include "versicherungstyp.h"

void vertragAnzeigen(versicherungstyp *vers) {
	printf("Vertrag anzeigen\n");

	for (int c = 0; c < vers->i; c++) {
			printf(
					"%d: %s, %s: Beitrag: %i; Laufzeit: %i; Zahlungsmodus: %i; ab %2i/%4i, %i Zahlungen \n",
					vers->Vertrag[c].VertragsID,
					vers->Vertrag[c].Name, vers->Vertrag[c].Vorname,
					vers->Vertrag[c].Jahresbeitrag,
					vers->Vertrag[c].Laufzeit,
					vers->Vertrag[c].Zahlungsmodus,
					vers->Vertrag[c].AbschlussMonat,
					vers->Vertrag[c].AbschlussJahr,
					vers->Vertrag[c].GesamtAnzahlZahlungen);
			abrechnungstyp *a = vers->Vertrag[c].OffeneZahlungenQueue;
			while (a != NULL) {
				printf("%5i, ", a->Betrag);
				a = a->next;
			}
			printf("\n");
		}
}
