#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_PORT 1234
#define QUEUE_SIZE 5

int main(int argc, char* argv[])
{
   int nSocket, nClientSocket; 
   int nBind, nListen;
   int nFoo = 1; 
   socklen_t nTmp; 
   struct sockaddr_in stAddr, stClientAddr; 

   /* address structure */
   memset(&stAddr, 0, sizeof(struct sockaddr)); //bzero(&server_addr, sizeof server_addr);
   stAddr.sin_family = AF_INET;
   stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   stAddr.sin_port = htons(SERVER_PORT);

   /* create a socket */
   nSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (nSocket < 0)
   {
       perror(stderr, "%s: Can't create a socket.\n", argv[0]);
       exit(1);
   }
  
   setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &nFoo, sizeof(nFoo));

   /* bind a name to a socket */
   nBind = bind(nSocket, (struct sockaddr*)&stAddr, sizeof sockaddr);
   if (nBind < 0)
   {
       perror(stderr, "%s: Can't bind a name to a socket.\n", argv[0]);
       exit(1);
   }
  
   /* specify queue size */
   nListen = listen(nSocket, QUEUE_SIZE);
   if (nListen < 0)
   {
       perror(stderr, "%s: Can't set queue size.\n", argv[0]);
       exit(1);
   }
  
  /*wait for connection*/
   while(1)
   {     
       /* block for connection request */
       nTmp = sizeof(struct sockaddr); 
       nClientSocket = accept(nSocket, (struct sockaddr*)&stClientAddr, &nTmp); 
       if (nClientSocket < 0)
       {
           perror(stderr, "%s: Can't create a connection's socket.\n", argv[0]);
           exit(1);
       }
     
       printf("Client connected.\n"); 
       //printf("%s: [connection from %s]\n", argv[0], inet_ntoa((struct in_addr)stClientAddr.sin_addr));
       }

   close(nSocket);
   return(0);
}
