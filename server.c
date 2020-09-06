#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>

int new_socket;
extern char *optarg;

void *func(void *arg) //htop gibi komutlarda, clienti kapatmadan komutdan cikis yapmayi saglamak icin thread yapisi kullanildi.
{
    char my_line[1024];
    int bufsize = 1024;
    int *argsam = (int *)arg;
    int size = read(argsam[0], my_line, bufsize);
    my_line[size] = '\0';

    if (!strcmp(my_line, "iptal"))
    {
        argsam[1] = 0;
        // printf("\nExit command");
    }

    pthread_exit(NULL);
}

int login_system(char *user, char *pass) //Sisteme girisi kontrol eden fonksiyon.
{
    char buffer[1024];
    int size = read(new_socket, buffer, 1024);
    buffer[size] = '\0';
    // printf("\nGelen user : %s ", buffer);
    if (!strcmp(buffer, user))
    {
        send(new_socket, "sucess", strlen("sucess"), 0);

        strcpy(buffer, "\0");
        size = read(new_socket, buffer, 1024);
        buffer[size] = '\0';
        // printf("\nGelen pass : %s ", buffer);
        fflush(stdout);
        if (!strcmp(buffer, pass))
        {
            send(new_socket, "sucess", strlen("sucess"), 0);
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
    int server_fd;
    struct sockaddr_in address;
    int option;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    char *port;
    char *user;
    char *pass;

    /*
        komutlar icin
    */
    char my_line[1024]; // Direk satir okuma yapmak icin tutuyorum.
    int bufsize = 1024;
    /*
        komutlar icin
    */

    while ((option = getopt(argc, argv, "P:u:p:")) != -1) // Gelen parametreler ayrildi.
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
        default:
            printf("Usage: ./server -P port -u username -p password\n");
            break;
        }
    }     

    int PORT = atoi(port); // String olarak gelen port int degere cevriliyor.

    // Socket dosya tanimi olusturuldu
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
   
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

     // Socket gelen port bilgisine ayarlandigi kisim.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);


    if (bind(server_fd, (struct sockaddr *)&address,     //Yeni oluşturulan soket verilen IP baglandi.
             sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)   // Server su anda dinliyor.
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  // Clientten gelen datayi kabul ediyor.
                             (socklen_t *)&addrlen)) < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    //Socket olusturuldu ve baglantilar saglandi. Sistem kullanima hazir.
    printf("\n Server  is ready");
    if (login_system(user,pass)) //Sisteme giris kosullari kontrol ediliyor.
    {

        printf("\nAccept Login");
        while (1) //Client tarafi kapanana kadar calisacak.
        {
            int size = read(new_socket, my_line, bufsize);  //Socketden okuma yapiliyor.
            my_line[size] = '\0';
            strcat(my_line, " 2>&1"); // Hata mesajlarini da iletebilmek adina " 2>&1" eklendi.
            FILE *fp;
            int status;
            fp = popen(my_line, "r"); // Komutun islenmesi ve gelen sonucu dondurmek icin popen() kullanildi.
            if (fp == NULL)
            {
                printf("error in popen.");
            }
            else
            {
                pthread_t ptid; // thread gelen komutun iptalini yakalayabilmek icin kullanildi
                int argsam[2];
                argsam[0] = new_socket;
                argsam[1] = 1;
                if (pthread_create(&ptid, NULL, &func, (void *)argsam))// thread yaratiliyor.
                {
                    printf("Error in create thread\n Exit");
                    exit(-1);
                }
                while (fgets(buffer, 1024, fp) != NULL && argsam[1]) // Okuma bitesiye yada iptal komutu gelene kadar veriyi gondermeye devam ediyor.
                {

                    send(new_socket, buffer, strlen(buffer), 0); 
                }
                status = pclose(fp);
                if (status == -1)
                {
                    printf("error while closing popen!\n");
                }
                pthread_cancel(ptid); // Olusturulan client kapatiliyor.
                send(new_socket, "finished", 8, 0); // Gelen veri bittiginde client tarafina bittigine dair mesaj gonderiliyor.
            }
        }

        //
    }
    else
    {
        printf("\nError on login, Exit");
    }
    return 0;
}
