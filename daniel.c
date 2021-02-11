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

#define LOKAL_PORT 80
#define BREV_STR 100
#define BAK_LOGG 10 // Størrelse på for kø ventende forespørsler 
#define DEF_FILE "/index.html" //setter default fil


void parseInp(int ny_sd, char filnavn[]);
int fil_eksisterer(char *filnavn, char kode[]);
int filtype(char filnavn[], char *type);
void setCode(int feilKode, char kode[]);
void print_header();
void writebody(char *filnavn);

int main ()
{
  	chdir("./var/www");
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
    		fprintf(stderr, "Prosess %d er knyttet til port %d.\n", getpid(), LOKAL_PORT);
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

      			dup2(ny_sd, 1); // redirigerer socket til standard utgang

			//print_header();

			parseInp(ny_sd, filnavn); // Hent sti til etterspurt fil.

			int size = 0;


      			if(filtype(filnavn, type) == 0)
			{
				size = fil_eksisterer(filnavn, kode);
      			}
			else
      				setCode(415, kode);

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

      			close(2);

      			// Sørger for å stenge socket for skriving og lesing
      			// NB! Frigjør ingen plass i fildeskriptortabellen
      			shutdown(ny_sd, SHUT_RDWR);
      			exit(0);
    		}
    		else
		{
      			signal(SIGCHLD, SIG_IGN);
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

	char *allowed[8] = {"text/html", "text/plain", "image/png", "image/svg", "application/xml", "application/xslt+xml", "text/css", "application/json"};

	char strKopi[30];
	strcpy(strKopi, filnavn);

	char *token = strtok(strKopi, ".");
	char *filtype = strtok(NULL, " ");

	if(strcmp(filtype, "asis") == 0)
	{
		strcat(type, "text/plain\n");
		return 0;
	}

	for(int i = 0; i < 8; i++)
	{
		typer = fopen("mime.types", "r");
		if(typer != NULL)
		{
			while(fgets(text, 100, typer))
			{
				if(text[0] != '#')
				{
					char *t_navn = strtok(text, "\t");
					char *endelse = strtok(NULL, "");

					if(strstr(allowed[i], t_navn))
					{
						char *hei = strstr(endelse, filtype);
						char *tk = strtok(hei, " ");

						while(tk != NULL)
						{
							if(strcmp(filtype, tk))
							{
								strcat(type, t_navn);
								return 0;
							}
							tk = strtok(NULL, " ");
						}
					}
				}
			}

			fclose(typer);

		}
		else
				printf("Kunne ikke åpne\n");
	}
	return 1;
}

void setCode(int feilKode, char kode[])
{
	switch(feilKode)
	{
		case 415:
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

	fflush(stdout);
}


