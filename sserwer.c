#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define SERVER_PORT 1220
#define USER_QUEUE_SIZE 8 //user_number
#define ROOM_QUEUE_SIZE 4 //user_number

//struktura zawierajaca dane, ktore zostana przekazane do watku
struct thread_data_t
{
    int cfd;
    struct sockaddr_in caddr;
};

//funkcja opisujaca zachowanie watku - musi przyjmowac argument typu (void *) i zwracac (void *)
void *ThreadBehavior(void *t_data)
{
    //pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    
    MakeUser(th_data);
    MakeRoom(th_data);

    //dostep do pol struktury: (*th_data).pole
    //TODO (przy zadaniu 1) klawiatura -> wysylanie albo odbieranie -> wyswietlanie

    close(th_data->cfd);
    free(t_data);
    pthread_exit(NULL);
}

//funkcja obslugujaca polaczenie z nowym klientem
void handleConnection(int server_socket_descriptor) {
    //wynik funkcji tworzacej watek
    int create_result = 0;

    //uchwyt na watek
    pthread_t thread1;

    //dane, ktore zostana przekazane do watku
    //TODO dynamiczne utworzenie instancji struktury thread_data_t o nazwie t_data (+ w odpowiednim miejscu zwolnienie pamieci)
    //TODO wypelnienie pol struktury

    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    socklen_t len = (socklen_t) sizeof(t_data->caddr);
    
    t_data->cfd = accept(server_socket_descriptor, (struct sockaddr*) &t_data->caddr, &len);

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, t_data);
    if (create_result){
       printf("Blad przy probie utworzenia watku, kod bledu: %d\n", create_result);
       exit(-1);
    }
    pthread_detach(thread1);
}

void SendMessage(int fd, char * buffer)
//void SendMessage(int receiverFd, char buffer[50])
{
    write(fd, buffer, sizeof(&buffer));
    //write(receiverFd, buffer, sizeof(&buffer));
}

//struktura gracza
struct User
{
    char ip[15];
    int fd;
    int id;
};
struct User usersCount[USER_QUEUE_SIZE];
int usersCreated = 0;

void MakeUser(struct thread_data_t *thread_data)
{
    memset(usersCount[usersCreated].ip, 0, 15);
    memcpy(usersCount[usersCreated].ip, inet_ntoa((struct in_addr) thread_data->caddr.sin_addr), 15);
    usersCount[usersCreated].fd = thread_data->cfd;
    usersCreated++;
    usersCount[usersCreated].id = usersCreated;
printf("User zrobiony");
}

//struktura gry
struct Room
{
    int turn;
    int roomId;
    int users;
    struct User ingame[2];
};
struct Room roomsCount[ROOM_QUEUE_SIZE];
int roomsCreated = 0;

void MakeRoom(struct thread_data_t *thread_data)
{
    int i, j;

    for (i = 0; i < ROOM_QUEUE_SIZE; i++)
    {
        if (roomsCount[i].users == 1)
        {
            
            for (j = 0; j < USER_QUEUE_SIZE; j++)
                if (usersCount[j].fd == thread_data->cfd)
                    roomsCount[i].ingame[1] = usersCount[j];

            roomsCount[i].users++;
            SendMessage(thread_data->cfd, "s");
            SendMessage(thread_data->cfd, "c");
            return;
        }
        else if (roomsCount[i].users == 0)
        {
            roomsCount[i].roomId = roomsCreated;
            roomsCount[i].users++;

            for (j = 0; j < USER_QUEUE_SIZE; j++)
                if (usersCount[j].fd == thread_data->cfd)
                    roomsCount[i].ingame[0] = usersCount[j];

            roomsCreated++;
            SendMessage(thread_data->cfd, "j" );
            //printf("Room created");
            SendMessage(thread_data->cfd, "a");
            return;
        }
    }
    //SendMessage(thread_data->cfd, "n");
    //printf("Brak wolnych pokoi");
}

/*void MakeOpportunity(struct thread_data_t *thread_data){
int i, j;
    for (i = 0; i < ROOM_QUEUE_SIZE; i++)
    {
        for (j = 0; j< 2; j++)
            if (roomsCount[i].ingame[j].fd == thread_data->cfd)
            {
                if (roomsCount[i].turn == 1)
                {
                    SendMessage(roomsCount[i].ingame[1].fd, "b");
                    SendMessage(roomsCount[i].ingame[0].fd, "c");
                    roomsCount[i].turn = 2;
                }
                else if (roomsCount[i].turn == 2)
                {
                    SendMessage(roomsCount[i].ingame[0].fd, "a");
                    SendMessage(roomsCount[i].ingame[1].fd, "c");
                    roomsCount[i].turn = 1;
                }
            }
    }
}*/

int main(int argc, char* argv[])
{
   int server_socket_descriptor;
   int bind_result;
   int listen_result;
   char reuse_addr_val = 1;
   struct sockaddr_in server_address;
   int port = 1234;

   //inicjalizacja gniazda serwera
   memset(&roomsCount, 0, sizeof(struct Room));
    for (int i = 0; i < ROOM_QUEUE_SIZE; i++)
        roomsCount[i].turn = 1;
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = PF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(SERVER_PORT);

   server_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
   if (server_socket_descriptor < 0)
   {
       fprintf(stderr, "%s: Blad przy probie utworzenia gniazda..\n", argv[0]);
       exit(1);
   }else{printf("Gniazdo utworzone");}

   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));
printf("ok");

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(server_address));
   if (bind_result < 0)
   {
       fprintf(stderr, "%s: Blad przy probie dowiazania adresu IP i numeru portu do gniazda.\n", argv[0]);
       exit(1);
   }

   listen_result = listen(server_socket_descriptor, USER_QUEUE_SIZE);
   if (listen_result < 0) {
       fprintf(stderr, "%s: Blad przy probie ustawienia wielkosci kolejki.\n", argv[0]);
       exit(1);
   }

   while(1)
   {
       //connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
      // if (connection_socket_descriptor < 0)
      // {
      //     fprintf(stderr, "%s: Blad przy probie utworzenia gniazda dla polaczenia.\n", argv[0]);
      //     exit(1);
      // }

       handleConnection(server_socket_descriptor);
   }

   close(server_socket_descriptor);
   return(0);
}
