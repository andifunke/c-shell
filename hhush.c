// Andreas Funke
// Info 2, HHU SoSe 2014

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>

// Makro zur Fehlerbehandlung bei der Speicherallozierung
#define MCHECK(x)	if(x == NULL) { \
						fprintf(stderr, "memory allocation error\n"); \
						exit(EXIT_FAILURE); \
					}

// Makro zur einfachen Reservierung von Speicher für den Rückgabe-String
#define MRUECK(x)	*rueckgabe = malloc(sizeof(x)); \
					MCHECK(*rueckgabe) \
					strcpy(*rueckgabe, x);

// Deklaration der Listen-Struktur
struct list {
	char* eintrag;
	struct list* next;
};

// Deklaration und Initialisierung globaler Variablen
const char invarg[] = "invalid arguments\n";
const char nofile[] = "no such file or directory\n";
const char nocommand[] = "command not found\n";
const char histfilename[] = "/.hhush.histfile";
const char cwderror[] = "couldn't get current working directory\n";


// FUNKTIONEN

/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
	Auslesen des aktuellen Verzeichnisses, speichern im dirbuffer-Array
*/
void getdir(char dirbuffer[]) {

	if(getcwd(dirbuffer, BUFSIZ) == NULL) {
		fprintf(stderr, cwderror);
		exit(EXIT_FAILURE);
	}
}


/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
	History-Liste: Einträge erstellen und Speicher zur Laufzeit reservieren
*/
void histlist(struct list** histstart, int* histcount, char inbuffer[]) {

	struct list* histptr;

	// Falls noch nicht vorhanden, wird der Listenanfang erstellt: zunächst wird Speicher für das 
	// erste Listenelement reserviert. Danach wird Speicher für den ersten Listeneintrag reserviert. 
	// Dieser speichert den Eingabestring (des Nutzers oder aus dem histfile). Der Listenzähler wird 
	// auf 1 gesetzt.
	if(*histstart == NULL) {
		*histstart = malloc(sizeof(struct list));
		MCHECK(*histstart)
		(*histstart)->eintrag = malloc(sizeof(char)*(strlen(inbuffer)+1));
		MCHECK((*histstart)->eintrag)
		strcpy((*histstart)->eintrag, inbuffer);
		(*histstart)->next = NULL;
		*histcount = 1;
	}

	// Listeneintrag anhängen: histptr wird zunächst an die Adresse des Listenanfangs gesetzt und 
	// läuft bis zum letzten Listenelement durch. Dann wird Speicher für das neue Listenelement 
	// reserviert und histptr auf dieses Element gesetzt. Danach wird Speicher für den neuen 
	// Listeneintrag reserviert. Dieser speichert den Eingabestring (des Nutzers oder aus dem 
	// histfile). Der Listenzähler wird um 1 erhöht.
	else {
		histptr = *histstart;
		while(histptr->next != NULL)
			histptr = histptr->next;
		histptr->next = malloc(sizeof(struct list));
		MCHECK(histptr->next)
		histptr = histptr->next;
		histptr->eintrag = malloc(sizeof(char)*(strlen(inbuffer)+1));
		MCHECK(histptr->eintrag)
		strcpy(histptr->eintrag, inbuffer);
		histptr->next = NULL;
		(*histcount)++;
	}
}


/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
	History-Liste: Speicher freigeben
*/
void freehist(struct list* histstart) {

	struct list* histptr = histstart;

	// Die Schleife durchläuft alle Listenelemente bis zum letzten Element. Bei jedem 
	// Schleifendurchlauf wird zunächst der Speicher des Listeneintrags freigegeben, histstart auf 
	// die Adresse des aktuellen Elements gesetzt, dann histptr auf das nächste Element gesetzt und 
	// schließlich der Speicher des vorherigen Listenelements (histstart) freigegeben.
	while(histptr->next != NULL) {
		free(histptr->eintrag);
		histstart = histptr;
		histptr = histptr->next;
		free(histstart);
	}

	// schließlich wird auch der Speicher des letzten Elements der Liste freigegeben.
	free(histptr->eintrag);
	free(histptr);
}


// *********************************************************************
//	EXIT:

void exitshell(char** rueckgabe, int* argcount, int* status, int* haspipe) {

	// nur beenden, wenn keine Argumente übergeben wurden
	if(*argcount > 0) {
		MRUECK(invarg)
	}

// haspipe = 0 bricht die Pipeschleife ab, status = 2 bricht die Programmschleife ab
	else {
		*haspipe = 0;
		*status = 2;
	}
}


// *********************************************************************
//	ECHO:

void echo(char** rueckgabe, int* argcount, char* argumente[]) {

	// Die übergebenen Argumente werden als ein String in argbuf[] kopiert.
	char argbuf[BUFSIZ] = {""};
	for(int i=1; i < *argcount+1; i++) {
		strcat(argbuf, argumente[i]);
		if(i != *argcount)
			strcat(argbuf, " ");
	}
	strcat(argbuf, "\n");

	// für den Rückgabestring wird ausreichend Speichert reserviert und ihm wird argbuf übergeben.
	*rueckgabe = malloc(sizeof(char)*(strlen(argbuf)+1));;
	MCHECK(*rueckgabe)
	strcpy(*rueckgabe, argbuf);
}


// *********************************************************************
//	DATE:

void date(char** rueckgabe, int* argcount) {

	// Anzahl der Argumente überprüfen:
	if(*argcount > 0) {
		MRUECK(invarg)
	}

	// Die aktuelle Zeit in Sekunden wird mit time() abgerufen und mit localtime() in Ortszeit 
	// umgewandelt und in der tm-Struktur gespeichert. strftime() übergibt diese formatiert an den 
	// Rückgabe-String, für den zuvor Speichert reserviert wurde.
	else {
		struct tm* zick;
		time_t sekunde;
		*rueckgabe = malloc(sizeof(char)*26);
		MCHECK(*rueckgabe)
		time(&sekunde);
		zick = localtime(&sekunde);
		strftime(*rueckgabe, 26, "%a %b %d %H:%M:%S %Y\n", zick);
	}
}


// *********************************************************************
//	LS:

void ls(char** rueckgabe, int* argcount, char cwdbuffer[BUFSIZ]) {

	// Anzahl der Argumente überprüfen:
	if(*argcount > 0) {
		MRUECK(invarg)
	}
	else {

		// Holen des aktuellen Verzeichnisses in die Struktur DIR und Anlegen der Sturktur dirent 
		// für die Verzeichnis-Einträge:
		DIR* directory = opendir(cwdbuffer);
		struct dirent* direintrag;

		// Der Inhalt des aktuellen Verzeichnisses wird durchlaufen und die Länge der Namen der 
		// Einträge im Zähler dircount gespeichert. Versteckte Dateien/Verzeichnisse werden 
		// übersprungen.
		int dircount = 0;
		while((direintrag = readdir(directory)) != NULL)
			if(direintrag->d_name[0] != '.')
				dircount = dircount+(strlen(direintrag->d_name))+1;

		// Für die Rückgabe wird Speicherplatz in der zuvor ermittelten Größe reserviert
		*rueckgabe = malloc(sizeof(char)*(dircount+1));
		MCHECK(*rueckgabe)
		strcpy(*rueckgabe, "");

		// Der Inhalt des aktuellen Verzeichnisses wird erneut durchlaufen und die Einträge diesmal 
		// in die Rückgabe kopiert. Versteckte Dateien/Verzeichnisse werden übersprungen.
		rewinddir(directory);
		while((direintrag = readdir(directory)) != NULL) {
			if(direintrag->d_name[0] != '.') {
				strcat(*rueckgabe, direintrag->d_name);
				strcat(*rueckgabe, "\n");
			}
		}
		closedir(directory);
	}
}


// *********************************************************************
//	CD:

void cd(char** rueckgabe, int* argcount, char* argumente[], char cwdbuffer[BUFSIZ]) {

	// Anzahl der Argumente überprüfen:
	if(*argcount != 1) {
		MRUECK(invarg)
		return;
	}

	// die Funktion chdir erledigt dann alle Arbeit von alleine
	int directory = chdir(argumente[1]);

	// falls chdir -1 zurück gibt, existiert kein Verzeichnis mit dem übergebenen Namen
	if(directory == -1) {
		MRUECK(nofile)
	}
	else
		getdir(cwdbuffer);
}


// *********************************************************************
//	GREP:

void grep(char** rueckgabe, int* argcount, char* argumente[], int* haspipe, char** vorgabe) {

	// grep vor Pipe: Durchsuchen einer Datei, falls zwei Argumente übergeben wurden:
	if(*haspipe != 1 && *argcount == 2) {

		// Öffnen des Datei-Streams
		FILE* grepfile;
		grepfile = fopen(argumente[2], "r");

		// Überprüfen, ob Datei vorhanden ist; Fehlermeldung via *rueckgabe
		if(grepfile == NULL) {
			MRUECK(nofile)
			return;
		}

		// zeilenweises Durchsuchen der Datei nach dem Suchstring und Ausgabe der gefundenen Zeilen 
		// via *rueckgabe. Für die Rückgabe wird der Einfachheit halber Speicher von der Größe der 
		// zu durchsuchenden Datei reserviert. Sonst müsste der Datei-Stream zum Bestimmen des exakt 
		// benötigten Speichers zweimal durchlaufen oder Speicher bei jedem Schleifendurchlauf 
		// realloziert werden. Da der reservierte Speicher aber vor dem nächsten Durchlauf der 
		// Programmschleife ohnehin wieder freigegeben wird, ist der möglicherweise zu viel 
		// reservierte Speicher zu vernachlässigen.
		else {
			fseek(grepfile, 0L, SEEK_END);
			long grepsize = ftell(grepfile);
			*rueckgabe = malloc(sizeof(char)*(grepsize+2));
			MCHECK(*rueckgabe)
			strcpy(*rueckgabe, "");
			rewind(grepfile);
			char grepbuf[grepsize];
			while(fgets(grepbuf, grepsize, grepfile))
				if(strstr(grepbuf, argumente[1]) != NULL) {
					strtok(grepbuf, "\n");
					strcat(*rueckgabe, grepbuf);
					strcat(*rueckgabe, "\n");
				}
			fclose(grepfile);
		}
	}

	// grep nach Pipe: Durchsuchen der Daten aus der Pipe:
	else if(*haspipe == 1 && *argcount == 1 && *vorgabe != NULL) {

		// Durchsuchen der Pipe-Daten (*vorgabe) zeilenweise nach dem Suchstring und Rückgabe via 
		// Zeiger *rueckgabe. Auch hier wird wieder der Einfachheit halber Speicher von der Größe 
		// der zu durchsuchenden Daten reserviert. Zum Durchsuchen zeigt grepbuf mit Hilfe von 
		// strtok auf die jeweiligen Zeilen, bevor sie an den Rückgabe-String übergeben oder 
		// verworfen werden.
		*rueckgabe = malloc(sizeof(char)*(strlen(*vorgabe)+1));
		MCHECK(*rueckgabe)
		strcpy(*rueckgabe, "");
		char* grepbuf;
		grepbuf = strtok(*vorgabe, "\n");
		while(grepbuf != NULL) {
			if(strstr(grepbuf, argumente[1]) != NULL) {
				strcat(*rueckgabe, grepbuf);
				strcat(*rueckgabe, "\n");
			}
			grepbuf = strtok(NULL, "\n");
		}
	}

	// falls die falsche Anzahl Argumente oder keine durchsuchbaren Daten übergeben wurden
	if(*rueckgabe == NULL) {
		MRUECK(invarg)
	}
}


// *********************************************************************
//	HISTORY:

void hist(char** rueckgabe, int* argcount, char* argumente[], struct list** histstart, 
	int* histcount) {

	struct list* histptr;
	int i;
	int histoffset = 0;
	int histsize = 0;
	char* histcheck = NULL;
	char histbuffer[BUFSIZ+12];

	// Funktion beenden, falls zu viele Argumente übergeben wurden
	if(*argcount > 1) {
		MRUECK(invarg)
		return;
	}
	else if(*argcount == 1) {

		// bei clear history Speicher freigeben und Anfangspointer auf NULL setzen
		if(strcmp(argumente[1], "-c") == 0) {
			freehist(*histstart);
			*histstart = NULL;
			return;
		}
		else {

			// überprüft, ob Werte > 0 eingegeben wurden
			if((i = strtol(argumente[1], &histcheck, 10)) < 1) {
				MRUECK(invarg)
				return;
			}

			// überprüft, ob nur Ziffern eingegeben wurden
			if(strcmp(histcheck, "") != 0) {
				MRUECK(invarg)
				return;
			}

			// histoffset ist die Differenz aus dem history-Listenzähler und der Anzahl 
			// anzuzeigender Einträge, d.h. so viele Einträge werden am Anfang NICHT angezeigt. 
			// Negative Werte werden abgefangen.
			histoffset = *histcount-i;
			if(histoffset < 0)
				histoffset = 0;
		}
	}

	// beim ersten Listendurchlauf wird zunächst ermittelt, wieviel Speicherplatz für die Rückgabe 
	// benötigt wird.
	histptr = *histstart;
	for(i = 0; i < *histcount; i++) {
		if(i >= histoffset) {
			sprintf(histbuffer, "%d %s", i, histptr->eintrag);
			histsize = histsize+strlen(histbuffer);
		}
		histptr = histptr->next;
	}

	// entsprechend großer Speicher wird angefordert.
	*rueckgabe = malloc(sizeof(char)*(histsize+1));
	MCHECK(*rueckgabe)
	strcpy(*rueckgabe, "");

	// beim zweiten Listendurchlauf werden die Listeneinträge in die Rückgabe kopiert
	histptr = *histstart;
	for(i = 0; i < *histcount; i++) {
		if(i >= histoffset) {
			sprintf(histbuffer, "%d %s", i, histptr->eintrag);
			strcat(*rueckgabe, histbuffer);
		}
		histptr = histptr->next;
	}
}


// #####################################################################
// MAIN:

int main() {

	/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
		Variablen Deklaration:
	*/
	int i;                          // Zählvariabel
	int argcount;                   // Anzahl der übergebenen Argumente
	int histcount;                  // zählt die Anzahl der Einträge der history-Liste
	int histoffset = 0;             // wie in hist-Funktion
	int haspipe = 0;                // 2 = vor Pipe, 1 = nach Pipe, 0 = Pipe beenden
	int status = 0;                 // 0 = ok, 1 = no command, 2 = exit
	char inbuffer[BUFSIZ];          // speichert Eingaben
	char dirbuffer[BUFSIZ];         // speichert aktuelles Verzeichnis für history-file
	char workingbuffer[BUFSIZ];     // Arbeitspuffer zum Zwischenspeichern von Eingaben
	char cwdbuffer[BUFSIZ];     	  // Current Working Directory Puffer
	char* rueckgabe = NULL;        // speichert die Ausgabe der Funktionen
	char* vorgabe = NULL;          // speichert die Ausgabe der Funktion vor der Pipe zwischen
	char** argumente = NULL;       // dynamisches Zeiger-Array zum Verwalten der Argumente
	FILE* histfile = NULL;         // Zeiger auf den Datei-Stream zum histfile
	struct list* histstart = NULL; // Zeiger auf den Anfang der history-Liste
	struct list* histptr = NULL;   // Zeiger auf die nachfolgenden Elemente der history-Liste


	/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
		histfile lesen:
	*/
	// Auslesen des aktuellen Verzeichnisses:
	getdir(dirbuffer);
	strcpy(cwdbuffer, dirbuffer);
	strcat(dirbuffer, histfilename);

	histfile = fopen(dirbuffer, "r");

	// liest jede Zeile des history-files in einen Listeneintrag ein
	if(histfile != NULL) {
		while(fgets(inbuffer, BUFSIZ, histfile))
			histlist(&histstart, &histcount, inbuffer);
		fclose(histfile);
	}


	/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
		Programmschleife Anfang:
	*/
	while(1) {

		// Reset des Pipestatus
		haspipe = 0;

		// Bildschirmausgabe des aktuellen Verzeichnisses und Nutzereingabe (speichern in inbuffer):
		printf("%s $ ", cwdbuffer);
		fgets(inbuffer, BUFSIZ, stdin);

		// Eingabe Whitespace? Dann zurück zum Schleifenanfang.
		strcpy(workingbuffer, inbuffer);
		if(strtok(workingbuffer, "\n\t ") == NULL)
			continue;

		// History-Eintrag erstellen
		histlist(&histstart, &histcount, inbuffer);


		/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
			Eingabe splitten:
		*/
		// inbuffer wird bei Pipe (falls vorhanden) abgeschnitten. In workingbuffer wird die Eingabe 
		// nach der Pipe zwischengespeichert. Falls die Pipe am Anfang der Eingabe steht, wird sie 
		// durch ein Leerzeichen ersetzt. Das behebt Folgeprobleme. Es wäre aber auch eine 
		// Fehlermeldung denkbar.
		if(inbuffer[0] == '|')
			inbuffer[0] = ' ';
		if((strchr(inbuffer, '|')) != NULL) {
			strtok(inbuffer, "|");
			strcpy(workingbuffer, strtok(NULL, "|"));
			haspipe = 2;
		}


		/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
			Pipeschleife Anfang:
		*/
		do {

			// Dynamisch alloziertes Array für Zeiger auf Argumente anlegen
			argumente = malloc(sizeof(char*)*2);
			MCHECK(argumente)

			// Es werden Zeiger auf die vom Nutzer übergebenen Argumente erstellt. Diese werden im 
			// argumente-Array gespeichert, das dynamisch mit der Anzahl der Argumente mitwächst. 
			// Die Anzahl der Argumente wird in argcount mitgezählt.
			argumente[argcount=0] = strtok(inbuffer, "\n\t ");
			while((argumente[argcount+1] = strtok(NULL, "\n\t ")) != NULL) {
				argcount++;
				argumente = realloc(argumente, sizeof(char*)*(argcount+2));
			}


			/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
				Programmauswahl:
			*/
			if(argumente[0] == NULL);
			else if(strcmp(argumente[0], "exit"   ) == 0)
				exitshell(&rueckgabe, &argcount, &status, &haspipe);
			else if(strcmp(argumente[0], "date"   ) == 0)
				date(&rueckgabe, &argcount);
			else if(strcmp(argumente[0], "echo"   ) == 0)
				echo(&rueckgabe, &argcount, argumente);
			else if(strcmp(argumente[0], "ls"     ) == 0)
				ls  (&rueckgabe, &argcount, cwdbuffer);
			else if(strcmp(argumente[0], "cd"     ) == 0)
				cd  (&rueckgabe, &argcount, argumente, cwdbuffer);
			else if(strcmp(argumente[0], "grep"   ) == 0)
				grep(&rueckgabe, &argcount, argumente, &haspipe, &vorgabe);
			else if(strcmp(argumente[0], "history") == 0)
				hist(&rueckgabe, &argcount, argumente, &histstart, &histcount);
			else
				status = 1; // no command


			/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
				Pipe oder nicht Pipe
			*/
			// falls eine Pipe vorhanden ist, die noch nicht durchlaufen wurde, wird der 
			// workingbuffer in den inbuffer kopiert (um beim nächsten Durchlauf der Pipeschleife 
			// extrahiert zu werden).
			if(haspipe == 2) {
				strcpy(inbuffer, workingbuffer);

				// Falls die Programmfunktion vor der Pipe eine Rückgabe erstellt hat, wird diese 
				// als Vorgabe für den Befehl nach der Pipe zwischengespeichert. Der Speicher von 
				// Rückgabe wird dann freigegeben.
				if(rueckgabe != NULL) {
					vorgabe = malloc(sizeof(char)*(strlen(rueckgabe)+1));
					MCHECK(vorgabe)
					strcpy(vorgabe, rueckgabe);
					free(rueckgabe);
					rueckgabe = NULL;
				}
			}

			// durch die folgende Inkrementierung wird sichergestellt, dass die Pipeschleife nur 
			// dann fortgeführt wird, wenn eine Pipe vorhanden ist, und der Befehl nach der Pipe 
			// noch nicht ausgeführt wurde.
			haspipe--;

			/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
				Speicher freigeben
			*/
			// falls reserviert: Speicher von argumente freigeben
			if(argumente != NULL) {
				free(argumente);
				argumente = NULL;
			}

		}	while (haspipe > 0); // Pipeschleife Ende


		/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
			Ausgabe
		*/
		// no command ausgeben
		if(status == 1) {
			printf("%s", nocommand);
			status = 0;
		}
		// falls vorhanden Rückgabe ausgeben
		else {
			if(rueckgabe != NULL)
				printf("%s", rueckgabe);
		}


		/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
			Speicher freigeben
		*/
		// falls reserviert: Speicher von Vorgabe freigeben
		if(vorgabe != NULL) {
			free(vorgabe);
			vorgabe = NULL;
		}

		// falls reserviert: Speicher von Rückgabe freigeben
		if(rueckgabe != NULL) {
			free(rueckgabe);
			rueckgabe = NULL;
		}

		// falls Programmstatus = 2: Programmschleife beenden (Programm wird dann beendet)
		if(status == 2)
			break;

	} // Programmschleife Ende


	/*	-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -
		histfile scheiben
	*/
	histfile = fopen(dirbuffer, "w");

	// nur die letzten 1000 Einträge der history-Liste werden in das histfile geschrieben. 
	// Ähnliches Verfahren wie in hist-Funktion.
	histoffset = histcount-1000;
	if(histoffset < 0)
		histoffset = 0;
	histptr = histstart;
	for(i = 0; i < histcount; i++) {
		if(i >= histoffset)
			fprintf(histfile, "%s", histptr->eintrag);
		histptr = histptr->next;
	}

	fclose(histfile);

	// Speicher der history-Liste freigeben
	freehist(histstart);

	return EXIT_SUCCESS;
}
