#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>

#define LOCAL_PORT 80
#define BACK_LOGG 10 // Størrelse på for kø ventende forespørsler 
#define BREV_STR 100

#define DEF_FILE "/index.html"  //Default fil som sendes om ingenting forespørres
#define MIMEFILE "/etc/mime.types"

void printErr(char *message);
void errorHandler(int errorCode, char code[]);
char *rmWS(char *string);
void *rmAfter(char *string, char *st);

void deamon() {

        if (fork()!=0) 
        {
                exit(0);
        }

        setsid();

        signal(SIGHUP, SIG_IGN);

        if (fork()!=0) 
        {
                exit(0);
        }

        close(0);
        close(1);       //Lukker stdinn, stdout og stderror
        close(2);
        int log = open("/var/log/debug.log", O_WRONLY|O_CREAT|O_APPEND, 0666);
        dup2(log, 2);
}

//Henter tidsstempel
char *getTime() {
        time_t elapsed;
        struct tm *now;

        elapsed = time(NULL);
        char *time = asctime(now);
        time[strcspn(time, "\n")] = 0;
        return time;
}

//Sjekker om filen eksisterer
int exists(const char *fname, char code[]) {
        int size;

        FILE *file;
        if ((file = fopen(fname, "r"))) {
                fseek(file, 0, SEEK_END);
                size = ftell(file);
                fseek(file, 0, SEEK_SET);

                strcat(code, "200 OK\n");

                return size;
        } else {
                errorHandler(404, code);
                return 0;
        }
}

void parseInp(int ny_sd, char fname[]) {
        int brv_len;
        char brev_buffer[BREV_STR];

        //Lagrer spørring
        brv_len = read(ny_sd, brev_buffer, BREV_STR);

        char *ret = strtok(brev_buffer, " ");
        ret = strtok(NULL, " ");

        if(strcmp(ret, "/") == 0) {
                strcpy(fname, DEF_FILE);
        } else
                strcpy(fname, ret);
}


//Sjekk om filtypen er en av de støttede filtypene
int supports(char *ftype, char *type) {
        char text[100];
        FILE *types;
        if (strcmp(ftype, "asis") == 0) {
                strcat(type, "text/plain");
                return 1;
        }

        types = fopen(MIMEFILE, "r");

        if(types != NULL) {
                while(fgets(text, 100, types)) {
                        if(text[0] != '#') {
                                char *t_name = strtok(text, "\t");
                                char *end = strtok(NULL, "");
                                if(end != NULL) {
                                        if(strstr(end, ftype)) {
                                                end = rmWS(end);
                                                char *tk = strtok(end, " ");

                                                while (tk != NULL) {
                                                        char cpy[50];
                                                        rmAfter(tk, cpy);

                                                        if(strcmp(ftype, cpy) == 0) {
                                                                strcat(type, t_name);
                                                                return 1;
                                                        }
                                                        tk = strtok(NULL, "\t");
                                                }
                                        }
                                }
                                fflush(stdout);
                        }
                }
                fclose(types);
        }

        printErr("etterspurte ugyldig filtype.");
        printErr(ftype);

        return 0;
}

char *rmWS(char *string) {
        for(int i = 0; i < sizeof(string); i++) {
                if(!isspace(string[i]))
                        return &string[i];
        }
}

void *rmAfter(char *string, char *st) {
        int count = 0;
        char copy[30];
        char ret[30];

        strcpy(copy, string);

        for(int i = 0; i < sizeof(copy); i++) {
                if(!isspace(copy[i]))
                        count++;
                else
                        break;
        }
        copy[count] = '\0';
        int a = count + 1;
        char *retur = strncpy(ret, copy, a);
        strcpy(st, retur);
}

void errorHandler(int errorCode, char code[])
{
        switch(errorCode)
        {
                case 415:
                        strcat(code, "415 Unsupported Media Type\n");
                        break;
                case 404:
                        strcat(code, "404 Not Found\n");
                        break;
                default:
                        strcat(code, "400 Bad Request");
        }
}

void writebody(char *fname) {
        int size;
        FILE *file;
        if ((file = fopen(fname, "r"))) {
                fseek(file, 0, SEEK_END);
                size = ftell(file);
                fseek(file, 0, SEEK_SET);

                char buffer[size];
                while(!feof(file)) {
                        fread(buffer, 1, sizeof(buffer), file);
                        write(1, buffer, sizeof(buffer));
                        bzero(buffer, sizeof(buffer));
                }
                fclose(file);
        }
}


void printErr(char *message) {
        fprintf(stderr, "%s     -       Prosess %d %s, forelder er %d\n", getTime(), getpid(), message, getppid());
}


int main ()
{
    struct sockaddr_in  lok_adr;
    int sd, ny_sd;

    deamon();

    int file = open("tjener.txt", O_WRONLY|O_CREAT, 0666);
    dup2(file, 3);

    char c;

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

    //Endrer webroten til /var/www/ i container
    chdir("./container/var/www/");
    chroot(".");

    //Setter bruker- og gruppeID til noe random
    setuid(55555);
    setgid(55555);


    // Venter på forespørsel om forbindelse
    listen(sd, BACK_LOGG);
    while(1)
    {

      // Aksepterer mottatt forespørsel
      ny_sd = accept(sd, NULL, NULL);


      if(0==fork()) 
      {

        //Setter opp HTTP-header
        char header[50] = "HTTP/1.1";   //HTTP version
        char code[50] = " ";            //Either OK or Error code
        char type[50] = "Content-Type: "; //Will contain the filetype
        char charset[30] = ";charset=UTF-8\n"; //Charset
        char fname[50];                 //Will contain filename
        char clength[30] = "Content-Length: "; //Content Length

        //printErr("Connected to client");

        dup2(ny_sd, 1); // redirigerer socket til standard utgang

        parseInp(ny_sd, fname);

        int size = 0;

        char temp[100];
        strcpy(temp, fname);


        char* buff = strtok(temp, ".");
        char* ftype = strtok(NULL, " ");

        fflush(stdout);
        close(2);

        //Sjekker om filtype støttes
        if (supports(ftype, type) == 1) {

               //Sjekker om filen eksisterer
                size = exists(fname, code);
        } else  {
                errorHandler(415, code);
        }

        //Skriv ut header og body for fil
        printf("%s", header);
        printf("%s", type);
        printf("%s%d\n", clength, size);
        printf("\n");
        fflush(stdout);
        writebody(fname);


        // Sørger for å stenge socket for skriving og lesing
        // NB! Frigjør ingen plass i fildeskriptortabellen
        shutdown(ny_sd, SHUT_RDWR);
        exit(0);
      }

      else
          {
          close(ny_sd);
      }
      signal(SIGCHLD, SIG_IGN);
    }
  return 0;
}
