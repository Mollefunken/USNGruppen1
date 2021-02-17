#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

#define LOKAL_PORT 80
#define BREV_STR 100
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 
#define DEF_FILE "/index.html" //setter default fil
#define R_DIR "/var/www" //setter rot-dir


void parseInp(int ny_sd, char filnavn[]);
char *rmWS(char *string);
void *rmAfter(char *string, char *st);
char *getTime();
void printErr(char *melding);
int fil_eksisterer(char *filnavn, char kode[]);
int filtype(char filnavn[], char *type);
void setCode(int feilKode, char kode[]);
void print_header();
void writebody(char *filnavn);

int main ()
{
  	chdir(R_DIR);
	chroot(".");

	struct sockaddr_in  lok_adr;
  	struct sockaddr_in fj_adr;

  	int sd, ny_sd;

  	socklen_t adr_len;

 	 // Setter opp socket-strukturen
	sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

 	 // For at operativsystemet ikke skal holde porten reservert etter tjenerens død
 	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

  	// Initierer lokal adresse
  	lok_adr.sin_family      = AF_INET;
  	lok_adr.sin_port        = htons((u_short)LOKAL_PORT);
  	lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);

  	// Kobler sammen socket og lokal adresse
  	if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
    		fprintf(stderr, "%s   -   Prosess %d er knyttet til port %d.\n", getTime(), getpid(), LOKAL_PORT);
  	else
    		exit(1);

  	setuid(1000);
  	setgid(1000);


  	// Venter på forespørsel om forbindelse
  	listen(sd, BAK_LOGG);
  	while(1)
	{
    		// Aksepterer mottatt forespørsel
    		ny_sd = accept(sd, (struct sockaddr *)&fj_adr, &adr_len);

    		if(0==fork())
		{
			char header[50] = "HTTP/1.1";
			char kode[50] = " ";
			char type[100] = "Content-Type: ";
			char charset[30] = ";charset=UTF-8\n";
			char filnavn[50];
			char content_length[30] = "Content-Length: ";

			printErr("koblet til klient");


      			dup2(ny_sd, 1); // redirigerer socket til standard utgang

//			print_header();

			parseInp(ny_sd, filnavn); // Hent sti til etterspurt fil.

			int size = 0;

      			if(filtype(filnavn, type) == 0)
			{
				size = fil_eksisterer(filnavn, kode);
      			}
			else
			{
      				setCode(422, kode);
			}

			strcat(header, kode);
			strcat(type, charset);

			printf("%s", header);

			if(size != 0)
			{
				printf("%s", type);
				printf("%s%d\n", content_length, size);
				printf("\n");
				fflush(stdout);
				writebody(filnavn);
			}
			else
				fflush(stdout);

			printErr("kobler fra klient");

      			close(2);

      			// Sørger for å stenge socket for skriving og lesing
      			// NB! Frigjør ingen plass i fildeskriptortabellen
      			shutdown(ny_sd, SHUT_RDWR);
      			exit(0);
			return 0;
    		}
    		else
		{
      			wait(NULL);
      			close(ny_sd);
    		}
  	}
  	return 0;
}

void parseInp(int ny_sd, char filnavn[])
{
	int brv_len;
	char brev_buffer[BREV_STR];

	brv_len = read(ny_sd, brev_buffer, BREV_STR);

	char *retur = strtok(brev_buffer, " ");
	retur = strtok(NULL, " ");

	if(strcmp(retur, "/") == 0)
	{
		strcpy(filnavn, DEF_FILE);
	}
	else
		strcpy(filnavn, retur);
}

char *getTime()
{
	time_t elapsed;
	struct tm *now;

	elapsed = time(NULL);

	now = localtime(&elapsed);

	char *tid = asctime(now);

	tid[strcspn(tid, "\n")] = 0;

	return tid;
}

void printErr(char *melding)
{
	fprintf(stderr, "%s   -   Prosess %d %s, forelder er %d\n", getTime(), getpid(), melding, getppid());
}

int fil_eksisterer(char *filnavn, char kode[])
{
	int size;

	FILE *file;
	if ((file = fopen(filnavn, "r")))
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);

		strcat(kode, "200 OK\n");

		return size;
	}
	else
	{
		printErr("etterspurte fil som ikke eksisterer");
		setCode(404, kode);
		return 0;
	}
}

void writebody(char *filnavn)
{
	int size;

	FILE *file;
	if ((file = fopen(filnavn, "r")))
	{
		fseek(file, 0, SEEK_END);
                size = ftell(file);
                fseek(file, 0, SEEK_SET);

                char buffer[size];

		while(!feof(file))
		{
                	fread(buffer, 1, sizeof(buffer), file);
                	write(1, buffer, sizeof(buffer));
			bzero(buffer, sizeof(buffer));
		}

		fclose(file);

	}
}

int filtype(char filnavn[], char *type)
{
	FILE *typer;
	char text[100];

	char strKopi[30];
	strcpy(strKopi, filnavn);

	char *token = strtok(strKopi, ".");
	char *filtype = strtok(NULL, " ");

	if(strcmp(filtype, "asis") == 0)
	{
		strcat(type, "text/plain");
		return 0;
	}

	typer = fopen("mime.types", "r");

	if(typer != NULL)
	{
		while(fgets(text, 100, typer))
		{
			if(text[0] != '#')
			{
				char *t_navn = strtok(text, "\t");
				char *endelse = strtok(NULL, "");

				if(endelse != NULL)
				{
					if(strstr(endelse, filtype))
					{
						endelse = rmWS(endelse);
						char *tk = strtok(endelse, " ");

						while(tk != NULL)
						{
							char cpy[50];
							rmAfter(tk, cpy);

							if(strcmp(filtype, cpy) == 0)
							{
								strcat(type, t_navn);
								printErr("etterspurte gyldig filtype");
								return 0;
							}
							tk = strtok(NULL, " \t");
						}
					}
				}
				fflush(stdout);
			}
		}

		fclose(typer);

	}
	else
		printErr("kunne ikke åpne mime.types.");

	printErr("etterspurte ugyldig filtype.");
	printErr(filtype);
	return 1;
}

char *rmWS(char *string)
{
	for(int i = 0; i < sizeof(string); i++)
	{
		if(!isspace(string[i]))
			return &string[i];

	}
}

void *rmAfter(char *string, char *st)
{
	int count = 0;
	char kopi[30];
	char retur[30];

	strcpy(kopi, string);

	for(int i = 0; i < sizeof(kopi); i++)
	{
		if(!isspace(kopi[i]))
			count++;
		else
			break;
	}

	kopi[count] = '\0';

	int a = count + 1;

	char *ret = strncpy(retur, kopi, a);

	strcpy(st, ret);
}


void setCode(int feilKode, char kode[])
{
	switch(feilKode)
	{
		case 422:
			strcat(kode, "422 Unprocessable Entity\n");
			break;
		case 404:
			strcat(kode, "404 Not Found\n");
			break;
		default:
			strcat(kode, "400 Bad Request\n");
	}
}

void print_header()
{
	printf("HTTP/1.1 200 OK\n");
	printf("Content-Type: text/plain\n");
	printf("\n");
	printf("%d\n", getpid());

	fflush(stdout);
}


