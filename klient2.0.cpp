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
//#include <GL/glut.h>
#include <cassert>
#include <cstdlib>
#include <utility>
#include <vector>
#include <iostream>

using namespace std;

char kogo_runda;
char pionek_gracza;

char buffer[50];
int connection_socket_descriptor;
char sign;

struct gracz{
	bool wygrany;
};

void Make_Board(char arr[8][8])
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			if((i%2 != 0 && j%2 == 0) || (i%2 == 0 && j%2 != 0)){
				if(i<2)
					arr[i][j] = 'x';
				if(i>5)
					arr[i][j] = 'o';
				if(i>1 && i <6)
					arr[i][j] = ' ';}
			if((i%2 == 0 && j%2 == 0) || (i%2 != 0 && j%2 != 0))
				arr[i][j] = '.';	
		}		
}

void Display_Board(char arr[8][8])
{	
	cout<<"   12345678"<<endl;
	cout<<"   "<<endl;
	for (int i = 1; i < 9; i++){
		cout<<i<<"  ";
		for (int j = 1; j < 9; j++)
			cout << arr[i-1][j-1];
		cout<<' '<<endl;
	}
}

void Now_Player(char kogo_runda,char pionek_gracza){
	cout<<"\tGRACZ:   "<<kogo_runda<<endl;
	cout<<"\tPIONKI:  "<<pionek_gracza<<endl;
	if (kogo_runda == 'A')
		cout << "Twoj ruch:\t";
}

bool sprawdz_poprawnosc_ruchu(char plansza[8][8], char ruch_gracza[3])
{
	if (ruch_gracza[0] >= '1'&&ruch_gracza[0] <= '8')
		if (ruch_gracza[1] >= '1'&&ruch_gracza[1] <= '8')
			if (ruch_gracza[2] <= '4'&&ruch_gracza[2] >= '1')
				return true;
	return false;
}

int RunGame()
{
	gracz A;
	A.wygrany = false;
	gracz B;
	B.wygrany = false;

	int x, y;
	char ruch_gracza[3];
	char buff[3];
	bool koniec_gry = false;

	char plansza[8][8];
	Make_Board(plansza);

	while (true) {			
    
    		if (kogo_runda == 'A'){
		  kogo_runda='B';
		  pionek_gracza='o';}
	  	else{
		  kogo_runda='A';
		  pionek_gracza='x';}
					
		system("cls");
		Display_Board(plansza);

		Now_Player(kogo_runda, pionek_gracza);

		if (kogo_runda == 'A'){
			cin >> ruch_gracza;
			if (ruch_gracza[0] == 'q'){
				send(connection_socket_descriptor, "qqq", 3, 0);
            			break;}	
			while(!sprawdz_poprawnosc_ruchu(plansza, ruch_gracza)){
				cout << endl << "Niepoprawny ruch, sprobuj jeszcze raz:\t";
				bzero(&ruch_gracza, sizeof ruch_gracza);
				cin >> ruch_gracza;
			}
			send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);	
			while(recv(connection_socket_descriptor, buff, sizeof(buff), 0)!=10){
				if(buff[0] == 'x'){
					cout << endl << "Niepoprawny ruch, sprobuj jeszcze raz:\t";
					bzero(&buff, sizeof buff);
					cin >> ruch_gracza;
					if (ruch_gracza[0] == 'q'){
						send(connection_socket_descriptor, "qqq", 3, 0);
            					break;}	
					while(!sprawdz_poprawnosc_ruchu(plansza, ruch_gracza)){
						cout << endl << "Niepoprawny ruch, sprobuj jeszcze raz:\t";
						bzero(&ruch_gracza, sizeof ruch_gracza);
						cin >> ruch_gracza;
					}
					send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
				}
				else
					break;	
			}

		}
		if (kogo_runda == 'B'){
			while(recv(connection_socket_descriptor, buff, sizeof(buff), 0)!=3){};
				for (int i = 0 ; i < 3 ; i++) 
					ruch_gracza[i] = buff[i];

			if (buff[0] == 'e' || buff[0] == 'd' || buff[0] == 'q')
            			break;
			}

			x = ruch_gracza[0] - '1';
			y = ruch_gracza[1] - '1';

			if (ruch_gracza[2] == '1')
				{
					if (x + 1 <= 7 && y - 1 >= 0 && plansza[x + 1][y - 1] == ' ')
					{
						if (x + 1 == 7 && pionek_gracza == 'x')
							plansza[x + 1][y - 1] = 'X';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x + 1][y - 1] = pionek_gracza - 32;
							else
								plansza[x + 1][y - 1] = pionek_gracza;
						plansza[x][y] = ' ';
					}
				}
				if (ruch_gracza[2] == '2')
				{
					if (x + 1 <= 7 && y + 1 <= 7 && plansza[x + 1][y + 1] == ' ')
					{
						if (x + 1 == 7 && pionek_gracza == 'x')
							plansza[x + 1][y + 1] = 'X';
						else
							if(plansza[x][y]=='X'||plansza[x][y]=='O')
								plansza[x + 1][y + 1] = pionek_gracza-32;
							else
								plansza[x + 1][y + 1] = pionek_gracza;
						plansza[x][y] = ' ';

					}

				}
				if (ruch_gracza[2] == '3')
				{
					if (x - 1 <= 7 && y + 1 <= 7 && plansza[x - 1][y + 1] == ' ')
					{
						if (x - 1 == 0 && pionek_gracza == 'o')
							plansza[x - 1][y + 1] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 1][y + 1] = pionek_gracza - 32;
							else
								plansza[x - 1][y + 1] = pionek_gracza;
						plansza[x][y] = ' ';
					}
				}
				if (ruch_gracza[2] == '4')
				{
					if (x - 1 <= 7 && y - 1 <= 7 && plansza[x - 1][y - 1] == ' ')
					{
						if (x - 1 == 0 && pionek_gracza == 'o')
							plansza[x - 1][y - 1] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 1][y - 1] = pionek_gracza - 32;
							else
								plansza[x - 1][y - 1] = pionek_gracza;
						plansza[x][y] = ' ';

					}
        		      }
	}
	return 0;
}

void SendMessage(char sign){
    send(connection_socket_descriptor, &sign, sizeof(sign), 0);
}

void ReceiveMessage(){
   int message = recv(connection_socket_descriptor, buffer, sizeof(buffer), 0);
   sign = buffer[0];
   buffer[message] = '\x0';
}

int main (int argc, char *argv[])
{

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


   ReceiveMessage();
   if (sign == 'w'){
	kogo_runda = 'B';
	pionek_gracza = 'o';
	RunGame();
   }
   if (sign == 'b'){
	kogo_runda = 'A';
	pionek_gracza = 'x';
	RunGame();
  }
  if (sign == 'o'){
	cout<<"Bark wolnych pokoi."<<endl;
  }
  
  close(connection_socket_descriptor);
  return 0;

}
