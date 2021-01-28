#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define LOCAL_PORT 80
#define BACK_LOGG 10 // Størrelse på for kø ventende forespørsler 
#define BREV_STR 100

void deamon() {

	if (fork()!=0) {
		exit(0);
	}

	setsid();

	signal(SIGHUP, SIG_IGN);

	if (fork()!=0) {
		exit(0);
	}

	close(0);
	close(1);	//Lukker stdinn og stdout
	int file = open("logg.txt", O_WRONLY|O_CREAT|O_APPEND, 0666);
	dup2(file, 2);


	//printf("Daemonizing succesfull");
}

int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int main ()
{

  struct sockaddr_in  lok_adr;
  int sd, ny_sd;

  char brev_buffer[BREV_STR];
  int brv_len;

  deamon();

  int file = open("tjener.txt", O_WRONLY|O_CREAT, 0666);
  dup2(file, 3);

  char c;
//  FILE *fp = fopen("/var/www/index.asis", "r");

  // Setter opp socket-strukturen
  sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // For at operativsystemet ikke skal holde porten reservert etter tjenerens død
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

  // Initierer lokal adresse
  lok_adr.sin_family      = AF_INET;
  lok_adr.sin_port        = htons((u_short)LOCAL_PORT); 
  lok_adr.sin_addr.s_addr = htonl(         INADDR_ANY);

  // Kobler sammen socket og lokal adresse
  if ( 0==bind(sd, (struct sockaddr *)&lok_adr, sizeof(lok_adr)) )
    fprintf(stderr, "Prosess %d er knyttet til port %d.\n", getpid(), LOCAL_PORT);
  else
    exit(1);

  // Venter på forespørsel om forbindelse
  listen(sd, BACK_LOGG);
  while(1){

    // Aksepterer mottatt forespørsel
    ny_sd = accept(sd, NULL, NULL);

//    fp = fopen("/var/www/index.asis", "r");


    if(0==fork()) {

      dup2(ny_sd, 1); // redirigerer socket til standard utgang

      //Lagrer spørring
      brv_len = read(ny_sd, brev_buffer, BREV_STR);
      char* fname = strtok(brev_buffer, " ");
      fname = strtok(NULL, " ");

      if (fname[0] == '/')
	fname++;

      printf("%s\n", fname);
      fflush(stdout);
      close(2);

      //Endrer webroten til ./www/
      chdir("./www/");
      chroot(".");

      //Setter bruker- og gruppeID til noe random?
      setuid(55555);
      setgid(55555);

      //printf("HTTP/1.1 200 OK\n");
      //printf("Content-Type: text/plain\n");
      //printf("\n");
      //printf("Hallo klient!\n");

      //Sjekker om fil eksisterer
      if (exists(fname) == 1) {
        printf("Filen eksisterer!");

        //Åpner filen
        FILE *file = fopen(fname,"r");
        if(file != NULL) {
          char line[128];
          while(fgets( line, sizeof line, file) != NULL) {
            fputs ( line, stdout );
          }
          fclose ( file );
        }

      } else {
        printf("File not found");
      }



      fflush(stdout);

      // Sørger for å stenge socket for skriving og lesing
      // NB! Frigjør ingen plass i fildeskriptortabellen
      shutdown(ny_sd, SHUT_RDWR);
      exit(0);
    }

    else {
      close(ny_sd);
    }
  }
  return 0;
}
