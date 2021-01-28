#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define LOCAL_PORT 80
#define BACK_LOGG 10 // Størrelse på for kø ventende forespørsler

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
        close(1);       //Lukker stdinn og stdout
        int file = open("logg.txt", O_WRONLY|O_CREAT|O_APPEND, 0666);
        dup2(file, 2);

        //printf("Daemonizing succesfull");
}


int main ()
{

  struct sockaddr_in  lok_adr;
  int sd, ny_sd;

  deamon();

  char c;
  FILE *fp = fopen("/var/www/index.asis", "r");

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

    fp = fopen("/var/www/index.asis", "r");

    if(0==fork()) {

      dup2(ny_sd, 1); // redirigerer socket til standard utgang

      //Leser inn første linje og lagrer request
      char* line = NULL;
      FILE* request = fopen(ny_sd, "r");
      

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

        //Read contents of file
        c = fgetc(fp);
        while (c != EOF) {
                printf ("%c", c);
                c = fgetc(fp);
        }
        fclose(fp);
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
