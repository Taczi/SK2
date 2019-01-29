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

#define USER_QUEUE_SIZE 200 //user_number
#define ROOM_QUEUE_SIZE 10 //user_number

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

char buffer[3];

//struktura zawierajaca dane, ktore zostana przekazane do watku
struct thread_data_t
{
    int cfd;
    struct sockaddr_in caddr;
};

//struktura gracza
struct User
{
    char ip[15];
    int fd;
    int id;
};

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

struct User usersCount[USER_QUEUE_SIZE];
int usersCreated = 0;

void add_user()
{
    pthread_mutex_lock(&count_mutex);
    usersCreated = usersCreated + 1; //nie chcemy aby ktoś miał to samo id co my
    pthread_mutex_unlock(&count_mutex);
}

void add_room()
{
    pthread_mutex_lock(&count_mutex);
    roomsCreated = roomsCreated + 1; //nie chcemy aby powstały dwa pokoje o tym samym id
    pthread_mutex_unlock(&count_mutex);
}

//funkcja tworzenia gracza
void MUser(struct thread_data_t *thread_data)
{
    memset(usersCount[usersCreated].ip, 0, 15);
    memcpy(usersCount[usersCreated].ip, inet_ntoa((struct in_addr) thread_data->caddr.sin_addr), 15);
    usersCount[usersCreated].fd = thread_data->cfd;
    add_user();
    usersCount[usersCreated].id = usersCreated;
}

//funkcja pokoju
int MRoom(struct thread_data_t *thread_data)
{
    int i, j;

    pthread_mutex_lock(&mutex);

    for (i = 0; i < ROOM_QUEUE_SIZE; i++)
    {
        if (roomsCount[i].users == 1) //jeśli ktoś jest w pokoju dołącze
        {
            
            for (j = 0; j < USER_QUEUE_SIZE; j++)
                if (usersCount[j].fd == thread_data->cfd)
                    roomsCount[i].ingame[1] = usersCount[j];

            roomsCount[i].users++;
	    roomsCount[i].turn++;
            pthread_mutex_unlock(&mutex);
            strcpy(buffer, "w"); //jestes graczem white
	    send(thread_data->cfd, buffer, strlen(buffer), 0);
            bzero(&buffer, sizeof buffer);
            return 0;
        }
	
        else if (roomsCount[i].users == 0) //nie ma niepełnych pokoi więc stworze swój
        {
	    pthread_mutex_unlock(&mutex);
            roomsCount[i].roomId = roomsCreated;
            roomsCount[i].users++;

            for (j = 0; j < USER_QUEUE_SIZE; j++)
                if (usersCount[j].fd == thread_data->cfd)
                    roomsCount[i].ingame[0] = usersCount[j];

            add_room();
	    roomsCount[i].s = 0;
            pthread_mutex_unlock(&mutex);
            strcpy(buffer, "b"); //jestes graczem black
	    send(thread_data->cfd, buffer, strlen(buffer), 0);
            bzero(&buffer, sizeof buffer);
            return 0;
        }
    }
    //nie ma miejsca na pokój dla mnie
    pthread_mutex_unlock(&mutex);
    strcpy(buffer, "o"); //brak gry
    send(thread_data->cfd, buffer, strlen(buffer), 0);
    bzero(&buffer, sizeof buffer);
    close(thread_data->cfd);
    return 1;
}

//funkcja wymiany wiadomosci
int RGame(struct thread_data_t *thread_data){
    int i, j;

    for (i = 0; i < ROOM_QUEUE_SIZE; i++)
    {
        for (j = 0; j< 2; j++){
            if (roomsCount[i].ingame[j].fd == thread_data->cfd) //szukam siebie wśród innych
            {
                if (roomsCount[i].turn == 0) //jestem graczem który stworzył pokój
                {
		    bzero(&buffer, sizeof buffer);
		    while(recv(thread_data->cfd, buffer, 3, 0) > 0) //wysyłam przeciwnikowi moja wiadomość
		    {
			send(roomsCount[i].ingame[1].fd, buffer, strlen(buffer), 0);
			bzero(&buffer, sizeof buffer);
		    }
		    if(recv(thread_data->cfd, buffer, 3, 0) == 0) //rozłączam się 
		    {
			strcpy(buffer, "d"); 
	    		send(roomsCount[i].ingame[1].fd, buffer, strlen(buffer), 0);
            		bzero(&buffer, sizeof buffer);
			roomsCount[i].users = 0; //zamykam pokój
			close(thread_data->cfd); //koniec wątku
			return 1;
		    }
		    if(recv(thread_data->cfd, buffer, 3, 0) < 0) //nie mogę się połaczyć
		    {
			strcpy(buffer, "e");
	    		send(roomsCount[i].ingame[1].fd, buffer, strlen(buffer), 0);
            		bzero(&buffer, sizeof buffer);
			roomsCount[i].users = 0; //zamykam pokój
			close(thread_data->cfd); //koniec wątku
			return 1;
			
		    } 
                }
		if (roomsCount[i].turn == 1) //jestem graczem który dołączył do pokoju i robie to samo co ten drugi :)
                {
		    bzero(&buffer, sizeof buffer);
		    while(recv(thread_data->cfd, buffer, 3, 0) > 0)
		    {
			send(roomsCount[i].ingame[0].fd, buffer, strlen(buffer), 0);
			bzero(&buffer, sizeof buffer);
                    }
		    if(recv(thread_data->cfd, buffer, 3, 0) == 0)
		    {
			strcpy(buffer, "d");
	    		send(roomsCount[i].ingame[0].fd, buffer, strlen(buffer), 0);
            		bzero(&buffer, sizeof buffer);
			close(thread_data->cfd);
			roomsCount[i].users = 0;
			return 1;
		    }
		    if(recv(thread_data->cfd, buffer, 3, 0) < 0)
		    {
			strcpy(buffer, "e");
	    		send(roomsCount[i].ingame[0].fd, buffer, strlen(buffer), 0);
            		bzero(&buffer, sizeof buffer);
			close(thread_data->cfd);
			roomsCount[i].users = 0;
			return 1;
		    } 
                }
	
         }
    
      }
   }
return 0;
}

//funkcja opisujaca zachowanie watku - musi przyjmowac argument typu (void *) i zwracac (void *)
void *ThreadBehavior(void *t_data)
{
    //pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;

    pthread_mutex_lock(&mutex);
    MUser(th_data);
    pthread_mutex_unlock(&mutex);

    if (MRoom(th_data) != 1){
    	while(RGame(th_data) != 1){
    	RGame(th_data);}
    }

    //close(th_data->cfd);
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
    struct thread_data_t *t_data = malloc(sizeof(struct thread_data_t));
    socklen_t len = (socklen_t) sizeof(t_data->caddr);

    t_data->cfd = accept(server_socket_descriptor, (struct sockaddr*) &t_data->caddr, &len);

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, t_data);
    if (create_result <0 ){
       printf("Blad przy probie utworzenia watku, kod bledu: %d\n", create_result);
       exit(-1);
    }

    pthread_detach(thread1);
}


int main(int argc, char* argv[])
{
   int server_socket_descriptor;
   int bind_result;
   int listen_result;
   char reuse_addr_val = 1;
   struct sockaddr_in server_address;

   //inicjalizacja gniazda serwera
   memset(&roomsCount, 0, sizeof(struct Room));
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = PF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(atoi(argv[1]));

   server_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0);
   if (server_socket_descriptor < 0)
   {
       fprintf(stderr, "%s: Blad przy probie utworzenia gniazda..\n", argv[0]);
       exit(1);
   }

   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

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

	handleConnection(server_socket_descriptor);

   }

   close(server_socket_descriptor);
   return(0);
}
