#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <iostream>
#include "pugixml.hpp"
#include <stdio.h>
extern int errno;
typedef struct thData
{
    int idThread;
    int cl;
} thData;
static void *treat(void *);
void StatusSosiri(void *);
void StatusPlecari(void *);
void Intarziere(void *);
void EstimareSosire(void *);
const char *actualizaretimp();
int compareOra(const char *timpP, const char *timp);
int transOra(const char *timp, const char *timptren);
const char *addTime(const char *value, int decalaj);
int main()
{   //De fiecare data cand deschidem serverul, baza de date va fi adusa la starea initiala
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("TrenuriInitial.xml");
    doc.save_file("Trenuri.xml");

    int PORT;
    int i = 0;
    printf("\nIntroduceti PORT-ul dorit pentru a porni serverul: ");
    scanf("%d", &PORT);
    struct sockaddr_in server;
    struct sockaddr_in from;
    int sd; //socket
    pthread_t th[100];

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server]Eroare la socket().\n");
        return errno;
    }
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server]Eroare la bind().\n");
        return errno;
    }
    if (listen(sd, 2) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    while (1)
    {
        int client;

        thData *td;
        socklen_t length = sizeof(from);
        printf("Port->%d\n", PORT);
        fflush(stdout);

        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        printf("S-a conectat:%s cu portul %d \n", inet_ntoa(from.sin_addr), ntohs(from.sin_port));

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept
        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        printf("\nClientul este %d\n", td->cl);
        //se va crea cate un thread pentru fiecare client, iar in thread se vor apela functiile aferente
        pthread_create(&th[i], NULL, &treat, td);

    }
}
static void *treat(void *arg)
{   //thread-ul unui client
    struct thData tdL;
    int nr;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    while (1)
    {
        tdL = *((struct thData *)arg);
        if (read(tdL.cl, &nr, sizeof(int)) <= 0)
        {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
        }
        //am citit comanda aleasa de client si o prelucram
        switch (nr)
        {
        case 1:
        {
            StatusSosiri((struct thData *)arg);
            break;
        }
        case 2:
        {
            StatusPlecari((struct thData *)arg);
            break;
        }
        case 4:
        {
            Intarziere((struct thData *)arg);
            break;
        }
        case 3:
        {
            EstimareSosire((struct thData *)arg);
            break;
        }
        case 5:
        {
            shutdown(tdL.cl, SHUT_RDWR);
            close((intptr_t)arg);
            return (NULL);
        }
        }
        printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, nr);
    }
    // am terminat cu acest client, inchidem conexiunea 
    close((intptr_t)arg);
    return (NULL);
};
void StatusSosiri(void *arg)
{   //functia Status Sosiri
    char ora[10];
    int ok = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    //struct cu cerintele clientului
    struct info
    {
        char statia[300];
        int cand;
    } primit;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("Trenuri.xml");
    if (read(tdL.cl, &primit, sizeof(info)) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }
    //mesajul ce va fi returnat
    char msg[300];
    memset(msg, 0, strlen(msg));
    strcat(msg, "\n");
    int ok2 = 0;
    char auxintarziere[300];

    //incepem sa cautam in baza de date
    if (primit.cand == 1) //toate trenurile din ziua respectiva care ajung in statia X
    {
        if (result)
        {
            for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren")) //cautam prin toate trenurile
            {
                ok = 0;//ok verifica daca s-a gasit macar o potrivire
                ok2 = 0; //ok2 verifica daca exista intarziere
                for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))//toate statiile
                    if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "first") != 0)
                    {
                        strcat(msg, "Tren ");
                        strcat(msg, tren.attribute("name").value());
                        ok = 1;
                        strcpy(ora, statie.attribute("oraS").value());
                        if (strcmp(statie.attribute("Bintarziere").value(), "true") == 0)
                        {
                            ok2 = 1;
                            strcpy(auxintarziere, statie.attribute("Tintarziere").value());
                        }
                    }
                if (ok == 1)
                {
                    strcat(msg, " : ");
                    strcat(msg, tren.child("Statii").child("Statie").attribute("name").value());
                    strcat(msg, "->");
                    strcat(msg, primit.statia);
                    strcat(msg, " ");
                    strcat(msg, ora);
                    if (ok2)
                    {
                        strcat(msg, "  Intarziere: ");
                        strcat(msg, auxintarziere);
                        strcat(msg, "min");
                    }
                    strcat(msg, "\n");
                }
            }
        }
        else
        {
            std::cout << "XML parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
            std::cout << "Error description: " << result.description() << "\n";
        }
    }
    else //toate trenurile care ajung in statia X in urmatoarea ora
    {
        const char *timph;
        timph = actualizaretimp();
        for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            ok = 0;
            ok2 = 0;
            for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
                if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "first") != 0 && transOra(timph, statie.attribute("oraS").value()))
                {
                    strcat(msg, "Tren ");
                    strcat(msg, tren.attribute("name").value());
                    ok = 1;
                    strcpy(ora, statie.attribute("oraS").value());
                    if (strcmp(statie.attribute("Bintarziere").value(), "true") == 0)
                    {
                        ok2 = 1;
                        strcpy(auxintarziere, statie.attribute("Tintarziere").value());
                    }
                }
            if (ok == 1)
            {
                strcat(msg, " : ");
                strcat(msg, tren.child("Statii").child("Statie").attribute("name").value());
                strcat(msg, "->");
                strcat(msg, primit.statia);
                strcat(msg, " ");
                strcat(msg, ora);
                if (ok2)
                {
                    strcat(msg, "  Intarziere: ");
                    strcat(msg, auxintarziere);
                    strcat(msg, "min");
                }
                strcat(msg, "\n");
            }
        }
    }
    if (msg[0] == '\n' && msg[1] == '\0')
        strcpy(msg, "\nNu s-a gasit nicio potrivire\n");
    //Trimitere mesaj txt catre client cu raspuns
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void StatusPlecari(void *arg)
{   //functia status plecari
    char ora[10];
    int ok = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    //struct cu informatii de la client
    struct info
    {
        char statia[300];
        int cand;
    } primit;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("Trenuri.xml");
    if (read(tdL.cl, &primit, sizeof(info)) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }
    char msg[300];
    memset(msg, 0, strlen(msg));
    strcat(msg, "\n");
    char ultimastatie[300];
    //PRELUARE DATE DIN BD
    if (primit.cand == 1) //toate trenurile din ziua respectiva care pleaca din statia X
    {
        if (result)
        {
            for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren"))
            {
                ok = 0;
                for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
                {
                    if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "last") != 0)
                    {
                        strcat(msg, "Tren ");
                        strcat(msg, tren.attribute("name").value());
                        ok = 1;
                        strcpy(ora, statie.attribute("oraP").value());
                    }
                    if (strcmp(statie.attribute("id").value(), "last") == 0 && ok == 1)
                        strcpy(ultimastatie, statie.attribute("name").value());
                }
                if (ok == 1)
                {
                    strcat(msg, " : ");
                    strcat(msg, primit.statia);
                    strcat(msg, "->");
                    strcat(msg, ultimastatie);
                    strcat(msg, " ");
                    strcat(msg, ora);
                    strcat(msg, "\n");
                }
            }
        }
        else
        {
            std::cout << "XML [ ] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
            std::cout << "Error description: " << result.description() << "\n";
        }
    }
    else //toate trenurile care pleaca din statia X in urmatoarea ora
    {
        const char *timph;
        timph = actualizaretimp();
        for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            ok = 0;
            for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
            {
                if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "last") != 0 && transOra(timph, statie.attribute("oraP").value()))
                {
                    strcat(msg, "Tren ");
                    strcat(msg, tren.attribute("name").value());
                    ok = 1;
                    strcpy(ora, statie.attribute("oraP").value());
                }
                if (strcmp(statie.attribute("id").value(), "last") == 0 && ok == 1)
                    strcpy(ultimastatie, statie.attribute("name").value());
            }
            if (ok == 1)
            {
                strcat(msg, " : ");
                strcat(msg, primit.statia);
                strcat(msg, "->");
                strcat(msg, ultimastatie);
                strcat(msg, " ");
                strcat(msg, ora);
                strcat(msg, "\n");
            }
        }
    }
    if (msg[0] == '\n' && msg[1] == '\0')
        strcpy(msg, "\nNu s-a gasit nicio potrivire\n");
    //Trimitere mesaj txt catre client cu raspuns
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void EstimareSosire(void *arg)
{   //functia estimare sosire
    char ora[10];
    int ok = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    struct info
    {
        char nrTren[2];
        char statia[300];
        char ora[300];
    } primit;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("Trenuri.xml");
    if (read(tdL.cl, &primit, sizeof(info)) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }
    char msg[300];
    memset(msg, 0, strlen(msg));
    strcat(msg, "\n");
    //PRELUARE DATE DIN BD
    if (result)
    {
        ok = 0;
        for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {
            int decalaj;

            if (strcmp(tren.attribute("name").value(), primit.nrTren) == 0)
            {
                for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
                {
                    if (ok == 2)
                    {
                        pugi::xml_attribute attr1 = statie.attribute("oraS");
                        pugi::xml_attribute attr2 = statie.attribute("oraP");
                        attr1.set_value(addTime(statie.attribute("oraS").value(), decalaj));
                        attr2.set_value(addTime(statie.attribute("oraP").value(), decalaj));
                        doc.save_file("Trenuri.xml");
                    }
                    if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "first") != 0)
                    {
                        if (compareOra(statie.attribute("oraP").value(), primit.ora) == 0)
                        {
                            pugi::xml_attribute attr = statie.attribute("oraS");
                            attr.set_value(primit.ora);
                            ok = 1;
                            doc.save_file("Trenuri.xml");
                        }
                        else
                        {
                            decalaj = compareOra(statie.attribute("oraP").value(), primit.ora);
                            pugi::xml_attribute attr1 = statie.attribute("oraS");
                            pugi::xml_attribute attr2 = statie.attribute("oraP");
                            attr1.set_value(primit.ora);
                            attr2.set_value(primit.ora);
                            doc.save_file("Trenuri.xml");
                            ok = 2;
                        }
                    }
                }
            }
        }
        if (ok == 0)
        {
            strcat(msg, "\nStatia introdusa nu se potriveste cu numarul trenului\n");
        }
        else
        {
            strcpy(msg, "\n UPDATE!\n");
        }
    }
    else
    {
        std::cout << "XML [ ] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
        std::cout << "Error description: " << result.description() << "\n";
    }

    if (msg[0] == '\n' && msg[1] == '\0')
        strcpy(msg, "\nNu s-a gasit nicio potrivire\n");
    //Trimitere mesaj txt catre client cu raspuns
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
void Intarziere(void *arg)
{   //functia de introducere intarziere
    char ora[10];
    int ok = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    struct info
    {
        char nrTren[2];
        char statia[300];
        char intarziere[300];
    } primit;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("Trenuri.xml");
    if (read(tdL.cl, &primit, sizeof(info)) <= 0)
    {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    }
    char msg[300];
    memset(msg, 0, strlen(msg));
    strcat(msg, "\n");
    //PRELUARE DATE DIN BD
    if (result)
    {
        ok = 0;
        for (pugi::xml_node tren = doc.child("Trenuri").child("Tren"); tren; tren = tren.next_sibling("Tren"))
        {

            if (strcmp(tren.attribute("name").value(), primit.nrTren) == 0)
            {
                for (pugi::xml_node statie = tren.child("Statii").child("Statie"); statie; statie = statie.next_sibling("Statie"))
                {
                    if (strcmp(statie.attribute("name").value(), primit.statia) == 0 && strcmp(statie.attribute("id").value(), "first") != 0)
                    {
                        pugi::xml_attribute attr1 = statie.attribute("Bintarziere");
                        pugi::xml_attribute attr2 = statie.attribute("Tintarziere");
                        attr1.set_value("true");
                        attr2.set_value(primit.intarziere);
                        ok = 1;
                        doc.save_file("Trenuri.xml");
                    }
                }
            }
        }
        if (ok == 0)
        {
            strcat(msg, "\nStatia introdusa nu se potriveste cu numarul trenului\n");
        }
        else
        {
            strcpy(msg, "\n UPDATE!\n");
        }
    }
    else
    {
        std::cout << "XML [ ] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
        std::cout << "Error description: " << result.description() << "\n";
    }

    if (msg[0] == '\n' && msg[1] == '\0')
        strcpy(msg, "\nNu s-a gasit nicio potrivire\n");
    //Trimitere mesaj txt catre client cu raspuns
    if (write(tdL.cl, msg, 300) <= 0)
    {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
    }
}
const char *actualizaretimp()
{   //functie ce actualizeaza timpul cu ora locala a computerului, pentru a vedea trenurile din urmatoarea ora
    int ok = 0;
    pugi::xml_document doc2;
    char ora[10];
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char timp[300];
    strcpy(timp, asctime(timeinfo));
    char timph[300];
    int j = 0;
    for (int i = 11; i <= 15; i++)
        timph[j++] = timp[i];
    timph[j] = '\0';
    char *timpreturn;
    timpreturn = timph;
    pugi::xml_parse_result result = doc2.load_file("Trenuri.xml");
    pugi::xml_node node = doc2.child("Trenuri");
    pugi::xml_attribute attr = node.attribute("time");
    attr = timph;
    doc2.save_file("Trenuri.xml");
    timpreturn = timph;
    return timpreturn;
}
int transOra(const char *timp, const char *timptren)
{   //functie ce returneaza 0 daca ora sosirii este mai mica decat ora plecarii sau decalajul daca trenul soseste dupa ce era programata plecarea
    char orat1[300];
    strcpy(orat1, timp);
    char orat2[300];
    strcpy(orat2, timptren);
    int ora1 = orat1[0] - '0';
    int ora2 = orat1[1] - '0';
    int ora3 = orat2[0] - '0';
    int ora4 = orat2[1] - '0';
    int min1 = orat1[3] - '0';
    int min2 = orat1[4] - '0';
    int min3 = orat2[3] - '0';
    int min4 = orat2[4] - '0';
    // HH:MM
    int orareturn1 = ora1 * 10 + ora2; // H1
    int orareturn2 = ora3 * 10 + ora4; // H2
    int minreturn1 = min1 * 10 + min2; // MIN1
    int minreturn2 = min3 * 10 + min4; //MIN2
    if ((orareturn2 <= (orareturn1 + 1)) && (orareturn2 > orareturn1))
    {
        return 1;
    }
    else if (orareturn2 > orareturn1 + 1)
        return 0;
    else if (orareturn2 == orareturn1 && minreturn1 < minreturn2)
        return 1;
    else
        return 0;
}
int compareOra(char const *timpP, char const *timp)
{
    char oraP[300];
    strcpy(oraP, timpP);
    char ora[300];
    strcpy(ora, timp);
    int ora2 = (oraP[0] - '0') * 10 + (oraP[1] - '0'); //ora plecare
    int ora3 = (ora[0] - '0') * 10 + (ora[1] - '0');   //cu ce schimbam ora sosirii
    int min2 = (oraP[3] - '0') * 10 + (oraP[4] - '0'); //min plecare
    int min3 = (ora[3] - '0') * 10 + (ora[4] - '0');   //cu ce schimbam min sosirii
    // 0-> Estimarea este inaintea plecarii
    // >0 -> Estimarea este dupa plecarea initiala, prin urmare toate sosirile si plecarile sunt decalate cu diferenta dintre timpul plecarii si estimarea sosirii
    int total1 = ora2 * 60 + min2;
    int total2 = ora3 * 60 + min3;
    int total = total2 - total1;
    if (ora2 == ora3 && min3 > min2)
        return (min3 - min2);
    else if (ora2 < ora3)
        return total;
    return 0;
}
const char *addTime(const char *value, int decalaj)
{   //functie care adauga decalajul la timpul sosirii si/sau cel al plecarii
    char ora[300];
    strcpy(ora, value);

    int ora1 = (ora[0] - '0') * 10 + (ora[1] - '0');
    int min1 = (ora[3] - '0') * 10 + (ora[4] - '0');
    int total = ora1 * 60 + min1 + decalaj;
    ora1 = (total / 60)%24;
    min1 = total % 60;
    char orareturn[300];
    memset(orareturn, 0, strlen(orareturn));
    if (ora1 >= 10)
        sprintf(orareturn, "%d", ora1);
    else
    {
        strcat(orareturn, "0");
        char aux[10];
        sprintf(aux, "%d", ora1);
        strcat(orareturn, aux);
    }
    if (min1 >= 10)
    {
        char aux[10];
        sprintf(aux, "%d", min1);
        strcat(orareturn, ":");
        strcat(orareturn, aux);
    }
    else
    {
        char aux[10];
        sprintf(aux, "%d", min1);
        strcat(orareturn, ":");
        strcat(orareturn, "0");
        strcat(orareturn, aux);
    }
    char *retfinal = orareturn;
    return retfinal;
}//