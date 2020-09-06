#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>

extern char *optarg;

int sock;

void handle_sigint(int sig) // Komut iptali sinyalini yakalayip server tarafina bilgi gonderen fonksiyon.
{
    printf("Caught signal %d\n", sig);
    send(sock, "iptal", 5, 0);
}

int login_system(char *user, char *pass) //Sisteme girisi kontrol eden fonksiyon.
{
    char buffer[1024];
    send(sock, user, strlen(user), 0);
    int size = read(sock, buffer, 1024);
    buffer[size] = '\0';
    if (!strcmp(buffer, "sucess"))
    {
        send(sock, pass, strlen(pass), 0);
        memset(buffer, '\0', 1024);
        size = read(sock, buffer, 1024);
        buffer[size] = '\0';
        if (!strcmp(buffer, "sucess")) //User ve password ayniysa girise onay veriyor, aksi takdirde vermiyor.
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    signal(SIGTSTP, handle_sigint);
    char *port;
    char *user;
    char *pass;
    char *host;

    int option;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    while ((option = getopt(argc, argv, "P:u:p:h:")) != -1) // Gelen parametreler ayrildi.
    {
        switch (option)
        {
        case 'P':
            port = optarg;
            // printf("port is %s\n", port);
            break;
        case 'u':
            user = optarg;
            // printf("The user is %s\n", user);
            break;
        case 'p':
            pass = optarg;
            // printf("password is %s\n", pass);
            break;
        case 'h':
            host = optarg;
            // printf("Host is %s\n", host);
            break;
        default:
            printf("Usage: ./server -P port -u username -p password\n");
            break;
        }
    }

    int PORT = atoi(port); // String olarak gelen port int degere cevriliyor.

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Socket dosya tanimi olusturuldu
    {
        printf("\n Socket creation error \n");
        return -1;
    }
        // printf("\n Socket creation Succesful \n");

    // Socket gelen port bilgisine ayarlandigi kisim.
    struct hostent *he;
    if ((he = gethostbyname(host)) == NULL)
    {
        printf("gethostbyname");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr = *((struct in_addr *)he->h_addr_list[0]);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // Client server socketine baglandi.
    {
        printf("\nConnection Failed \n");
        return -1;
    }
        // printf("\nConnection Succesful \n");
    if (login_system(user, pass)) //Sisteme giris kosullari kontrol ediliyor.
    {
        printf("\nAccept Login");
        while (1)
        {
            printf("\n\n --> ");
            fgets(buffer, 1024, stdin);
            if (strlen(buffer) > 2) //Enter girilirse verilen komut islenmiyor. Sistemin kitlenmemesi icin
            {
                buffer[strlen(buffer) - 1] = '\0';
                send(sock, buffer, strlen(buffer), 0);
                int flag = 0;
                do
                {
                    int size = read(sock, buffer, 1024);
                    buffer[size] = '\0';

                    if (strcmp(buffer, "finished")) // Eger serverden finished degerini okursa, bayragi degistirip cikiyor.
                    {
                        printf("%s ", buffer);
                    }
                    else
                    {
                        flag = 1;
                    }

                } while (!flag);
                strcpy(buffer, "\0");
            }
        }
    }
    else
    {
        printf("Error on login, Exit");
    }

    return 0;
}
