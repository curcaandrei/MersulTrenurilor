#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <stdbool.h>
int port;
int main(int argc, char *argv[])
{
  int sd;
  struct sockaddr_in server;

  int comanda1 = 0;
  char buf[10];
  bool isRunning = true;

  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  port = atoi(argv[2]);

  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  while (isRunning)
  {

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
      perror("[client]Eroare la connect().\n");
      return errno;
    }

    while (isRunning)
    {
      //MENIU
      printf("\n1.Status Sosiri");
      printf("\n2.Status Plecari");
      printf("\n3.Estimare Sosire");
      printf("\n4.Intarziere");
      printf("\n5.Deconectare\n");
      printf("Introduceti o comanda: ");
      fflush(stdout);

      //Citim ce doreste utilizatorul si trimitem catre server
      scanf("%d", &comanda1);
      if (write(sd, &comanda1, sizeof(int)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
      }
      switch (comanda1)
      {
                  case 5:
                  {
                            isRunning = false;
                            break;
                  }
                  case 1:
                  { // Status sosiri
                              struct info
                              {
                                char statia[300];
                                int cand;
                              } trimit;
                              printf("\n--------INFORMATII SOSIRI--------");
                                  printf("\n Sosiri in stiatia: ");
                              bzero(trimit.statia, sizeof(trimit.statia));
                              scanf("%s", trimit.statia);
                              printf("\n1.Toata ziua");
                              printf("\n2.Urmatoarea ora");
                              printf("\nIntroduceti comanda: ");
                              scanf("%d", &trimit.cand);
                              fflush(stdout);
                              //trimitere catre server

                              if (write(sd, &trimit, sizeof(info)) <= 0)
                              {
                                perror("[client]Eroare la write() spre server.\n");
                                return errno;
                              }

                              char primit[300];
                              bzero(primit, sizeof(primit));
                              if (read(sd, primit, 300) < 0)
                              {
                                perror("[client]Eroare la read() de la server.\n");
                                return errno;
                              }
                              printf("%s", primit);
                              break;
                        }
                        case 2:
                        {// STATUS PLECARI
                                  struct info
                                  {
                                    char statia[300];
                                    int cand;
                                  } trimit;
                                  printf("\n--------INFORMATII PLECARI--------");
                                      printf("\n Plecari din statia: ");
                                  bzero(trimit.statia, sizeof(trimit.statia));
                                  scanf("%s", trimit.statia);
                                  printf("\n1.Toata ziua");
                                  printf("\n2.Urmatoarea ora");
                                  printf("\nIntroduceti comanda: ");
                                  scanf("%d", &trimit.cand);
                                  fflush(stdout);
                                  //trimitere catre server
                                  if (write(sd, &trimit, sizeof(info)) <= 0)
                                  {
                                    perror("[client]Eroare la write() spre server.\n");
                                    return errno;
                                  }

                                  char primit[300];
                                  bzero(primit, sizeof(primit));
                                  if (read(sd, primit, 300) < 0)
                                  {
                                    perror("[client]Eroare la read() de la server.\n");
                                    return errno;
                                  }
                                  printf("%s", primit);
                                  break;
                  }
                  case 3:
                  {// ESTIMARE SOSIRE
                          struct info
                          {
                            char nrTren[2];
                            char statia[300];
                            char ora[300];
                          } trimit;
                          printf("\n--------ESTIMARE SOSIRE--------");
                          printf("\nIntroduceti numarul trenului:");
                          scanf("%s", trimit.nrTren);
                          printf("\nIntroduceti statia in care ajungeti: ");
                          bzero(trimit.statia, sizeof(trimit.statia));
                          scanf("%s", trimit.statia);
                          printf("\nIntroduceti ora la care ajungeti[ ORA:MINUTE ]: ");
                          bzero(trimit.ora, sizeof(trimit.ora));
                          scanf("%s",trimit.ora);
                          fflush(stdout);
                          //trimitere catre server
                          if (write(sd, &trimit, sizeof(info)) <= 0)
                          {
                            perror("[client]Eroare la write() spre server.\n");
                            return errno;
                          }

                          char primit[300];
                          bzero(primit, sizeof(primit));
                          if (read(sd, &primit, 300) < 0)
                          {
                            perror("[client]Eroare la read() de la server.\n");
                            return errno;
                          }
                          printf("%s", primit);
                          break;
                  }
                  case 4:
                  { // INTARZIERE
                          struct info
                          {
                            char nrTren[2];
                            char statia[300];
                            char intarziere[300];
                          } trimit;
                          printf("\n--------INTARZIERE--------\n");
                          printf("Introduceti numarul trenului:");
                          bzero(trimit.nrTren, sizeof(trimit.nrTren));
                          scanf("%s", trimit.nrTren);
                          printf("Introduceti statia in care ajungeti: ");
                          bzero(trimit.statia, sizeof(trimit.statia));
                          scanf("%s", trimit.statia);
                          printf("Introduceti numarul de minute intarziate: ");
                          bzero(trimit.intarziere, sizeof(trimit.intarziere));
                          scanf("%s", trimit.intarziere);
                          fflush(stdout);
                          //trimitere catre server

                          if (write(sd, &trimit, sizeof(info)) <= 0)
                          {
                            perror("[client]Eroare la write() spre server.\n");
                            return errno;
                          }

                          char primit[300];
                          bzero(primit, sizeof(primit));
                          if (read(sd, primit, 300) < 0)
                          {
                            perror("[client]Eroare la read() de la server.\n");
                            return errno;
                          }
                          printf("%s", primit);
                          break;
                    }
                  default:{
                    printf("\n--Comanda gresita--\n");
                  }
            }
    }
    /* inchidem conexiunea, am terminat */
    close(sd);
  }
  exit(0);
}//
