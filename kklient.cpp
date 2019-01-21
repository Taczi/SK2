#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <cmath>
#include <GL/glut.h>
#include <cassert>
#include <cstdlib>
#include <utility>
#include <vector>

#define BUF_SIZE 1024
#define NUM_THREADS     5

char buffer[50];
int connection_socket_descriptor;
char sign;

void SendMessage(char sign){
    send(connection_socket_descriptor, &sign, sizeof(sign), 0);
}

void ReceiveMessage(){
   int message = recv(connection_socket_descriptor, buffer, sizeof(buffer), 0);
   sign = buffer[0];
   buffer[message] = '\x0';
}

void SendPosition(int array [8][8]){
   char charArray[8][8];
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
         		if (array[i][j] == 0)
				charArray[i][j] = '0';
			else if (array[i][j] == 1)
				charArray[i][j] = '1';
			else if (array[i][j] == 2)
				charArray[i][j] = '2';
			else if (array[i][j] == 9)
				charArray[i][j] = '9';
      }
   send(connection_socket_descriptor, charArray[i], 8, 0);
   }
}

void RunGame(){
printf("ruszył");
}

void RunGame1(){
printf("dolaczyl");
}


int main (int argc, char *argv[])
{

//networkConnection.SetIpAddress(argv);  ipAddress

   int connect_result;
   struct sockaddr_in server_address;
   struct hostent* server_host_entity;

   if (argc != 3)
   {
     fprintf(stderr, "Sposob uzycia: %s server_name port_number\n", argv[0]);
     exit(1);
   }else{printf("Coś ruszyło\n");}

   server_host_entity = gethostbyname(argv[1]);
   if (! server_host_entity)
   {
      fprintf(stderr, "%s: Nie mozna uzyskac adresu IP serwera.\n", argv[0]);
      exit(1);
   }else{printf("Uzyskano adress IP\n");}

   connection_socket_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (connection_socket_descriptor < 0)
   {
      fprintf(stderr, "%s: Blad przy probie utworzenia gniazda.\n", argv[0]);
      exit(1);
   }else{printf("Utworzono gniazdo\n");}

   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   memcpy(&server_address.sin_addr.s_addr, server_host_entity->h_addr, server_host_entity->h_length);
   server_address.sin_port = htons(atoi(argv[2]));

   connect_result = connect(connection_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (connect_result < 0)
   {
      fprintf(stderr, "%s: Blad przy probie polaczenia z serwerem (%s:%i).\n", argv[0], argv[1], atoi(argv[2]));
      exit(1);
   }else{printf("Połączono\n");}

   SendMessage('s');
   //while(1){
   ReceiveMessage();
   if (sign == 'j'){
//	//ReceiveMessage();
	RunGame1(); //Chess chess(window, networkConnection);
   }
   if (sign == 's'){
//	//ReceiveMessage();
	RunGame(); //Chess chess(window, networkConnection);
   }//}
  
   //close(connection_socket_descriptor);
   return 0;

}
