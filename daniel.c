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

int main ()
{

  struct sockaddr_in  lok_adr;
  struct sockaddr_in fj_adr;

  int sd, ny_sd, ja;

  char brev_buffer[BREV_STR];
  socklen_t adr_len;
  int brv_len;

  DIR *d;
  struct dirent *dir;
  d = opendir(".");

  int file = open("tjener.txt", O_WRONLY|O_CREAT, 0666);
  dup2(file, 3);

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
  while(1){ 

    // Aksepterer mottatt forespørsel
    ny_sd = accept(sd, (struct sockaddr *)&fj_adr, &adr_len);    

    if(0==fork()) {

      dup2(ny_sd, 1); // redirigerer socket til standard utgang

      printf("HTTP/1.1 200 OK\n");
      printf("Content-Type: text/plain\n");
      printf("\n");
      printf("Hallo klient!\n");
      printf("\nHer går skillet\n");

      fflush(stdout);

      brv_len = read(ny_sd, brev_buffer, BREV_STR);

      char *filnavn = strtok(brev_buffer, " ");
      filnavn = strtok(NULL, " ");

      printf("%s\n", filnavn);

      fflush(stdout);

      close(2);

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
