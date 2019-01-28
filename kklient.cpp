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
	int ilosc_pionkow;
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

bool sprawdz_poprawnosc_ruchu(char plansza[8][8], char ruch_gracza[3], char kogo_runda,bool wymagane_bicie)
{
	if (ruch_gracza[0] >= '1'&&ruch_gracza[0] <= '8')
		if (ruch_gracza[1] >= '1'&&ruch_gracza[1] <= '8')
			if (ruch_gracza[2] <= '4'&&ruch_gracza[2] >= '1')
			{
				if (((plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'x' && (( ruch_gracza[2] != '3' && ruch_gracza[2] != '4' ) || wymagane_bicie )) || plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'X') && kogo_runda == 'A')
					return true;
				if (((plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'o' && (( ruch_gracza[2] != '1' && ruch_gracza[2] != '2' ) || wymagane_bicie ))  || plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'O') && kogo_runda == 'B')
					return true;
			}
	return false;
}


bool czy_wymagane_bicie(char plansza[8][8], char pionek_gracza)
{
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
		{
			if (plansza[i][j] == pionek_gracza)
			{
				//sprawdzanie czy na drodze pionka stoi pionek rozny od niego, jego krolowej i spacji, oraz czy za nim jest puste pole
				if (i + 2 < 8 && j + 2 < 8 && plansza[i + 1][j + 1] != pionek_gracza && (plansza[i + 1][j + 1] != pionek_gracza + 32 || plansza[i + 1][j + 1] != pionek_gracza - 32 )&& plansza[i + 1][j + 1] != ' ' && plansza[i + 2][j + 2] == ' ')
					return true;
				if (i - 2 >= 0 && j + 2 < 8 && plansza[i - 1][j + 1] != pionek_gracza && (plansza[i - 1][j + 1] != pionek_gracza + 32 || plansza[i - 1][j + 1] != pionek_gracza - 32 ) && plansza[i - 1][j + 1] != ' ' && plansza[i - 2][j + 2] == ' ')
					return true;
				if (i + 2 < 8 && j - 2 >= 0 && plansza[i + 1][j - 1] != pionek_gracza && (plansza[i + 1][j - 1] != pionek_gracza + 32 || plansza[i + 1][j - 1] != pionek_gracza - 32 ) && plansza[i + 1][j - 1] != ' ' && plansza[i + 2][j - 2] == ' ')
					return true;
				if (i - 2 >= 0 && j - 2 >= 0 && plansza[i - 1][j - 1] != pionek_gracza && (plansza[i - 1][j - 1] != pionek_gracza + 32 || plansza[i - 1][j - 1] != pionek_gracza - 32 ) && plansza[i - 1][j - 1] != ' ' && plansza[i - 2][j - 2] == ' ')
					return true;
			}
		}
	return false;
}

void usun_pionek(gracz *A, gracz *B, char kogo_ruch)
{
	if (kogo_ruch == 'A')
		B->ilosc_pionkow--;
	else
		A->ilosc_pionkow--;
}

bool sprawdz_czy_koniec_gry(gracz *A, gracz *B)
{
	if (A->ilosc_pionkow == 0)
	{
		B->wygrany = true;
		return true;
	}
	if (B->ilosc_pionkow == 0)
	{
		A->wygrany = true;
		return true;
	}
	return false;
}

bool czy_mozliwy_ruch(char arr[8][8], char pionek_gracza)
{
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++)
		{
			if (arr[i][j] == pionek_gracza)
			{
				if (pionek_gracza == 'x')
				{
					if (i < 7 && j > 0 && j <= 7 && arr[i + 1][j - 1] == ' ')
						return true;
					if (i < 7 && j > 0 && j <= 7 && arr[i + 1][j + 1] == ' ')
						return true;
				}
				if (pionek_gracza == 'o')
				{
					if (i > 0 && j > 0 && j <= 7 && arr[i - 1][j - 1] == ' ')
						return true;
					if (i > 0 && j > 0 && j <= 7 && arr[i - 1][j + 1] == ' ')
						return true;
				}
			}

		}
  }
	return false;
}

int RunGame()
{
	gracz A;
	A.ilosc_pionkow = 8;
	A.wygrany = false;
	gracz B;
	B.ilosc_pionkow = 8;
	B.wygrany = false;

	int x, y;
	char ruch_gracza[3];
	char buff[3];
	bool poprawny_ruch=false;
	bool koniec_gry = false;
	bool wymagane_bicie = false;
	bool mozliwy_ruch = true;

	char plansza[8][8];
	Make_Board(plansza);

	while (true) {			
		poprawny_ruch = false;
		wymagane_bicie = false;
    
    		if (kogo_runda == 'A'){
		  kogo_runda='B';
		  pionek_gracza='o';}
	  	else{
		  kogo_runda='A';
		  pionek_gracza='x';}
					
		system("cls");
		Display_Board(plansza);
		wymagane_bicie = czy_wymagane_bicie(plansza,pionek_gracza);
		mozliwy_ruch = czy_mozliwy_ruch(plansza, pionek_gracza);
		if (!mozliwy_ruch)
		{
			koniec_gry = true;
			if (kogo_runda == 'A')
				B.wygrany = true;
			else
				A.wygrany = true;
		}
		Now_Player(kogo_runda, pionek_gracza);

		while (!poprawny_ruch)
		{
			if (kogo_runda == 'A')
				cin >> ruch_gracza;
			if (kogo_runda == 'B'){
				while(recv(connection_socket_descriptor, buff, sizeof(buff), 0)!=3){};
				for (int i = 0 ; i < 3 ; i++) {
					ruch_gracza[i] = buff[i];}
				}

			poprawny_ruch = sprawdz_poprawnosc_ruchu(plansza, ruch_gracza, kogo_runda, wymagane_bicie);
			x = ruch_gracza[0] - '1';
			y = ruch_gracza[1] - '1';

			if (poprawny_ruch)
			{
				poprawny_ruch = false;
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
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
					if (x + 2 <= 7 && y - 2 >= 0 && plansza[x + 1][y - 1] != pionek_gracza && (plansza[x + 1][y - 1] != pionek_gracza + 32 || plansza[x + 1][y - 1] != pionek_gracza - 32) && plansza[x + 2][y - 2] == ' ')
					{
						if (x + 2 == 7 && pionek_gracza == 'x')
							plansza[x + 2][y - 2] = 'X';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x + 2][y - 2] = pionek_gracza - 32;
							else
								plansza[x + 2][y - 2] = pionek_gracza;
						plansza[x][y] = ' ';
						plansza[x + 1][y - 1] = ' ';
						usun_pionek(&A, &B, kogo_runda);
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
				}
				if (ruch_gracza[2] == '2')
				{
					if (x + 1 <= 7 && y + 1 <= 7 && plansza[x + 1][y + 1] == ' ')		//warunki poruszania sie na puste pole
					{
						if (x + 1 == 7 && pionek_gracza == 'x')							//ruch bez biciem
							plansza[x + 1][y + 1] = 'X';
						else
							if(plansza[x][y]=='X'||plansza[x][y]=='O')
								plansza[x + 1][y + 1] = pionek_gracza-32;
							else
								plansza[x + 1][y + 1] = pionek_gracza;
						plansza[x][y] = ' ';
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
					if (x + 2 <= 7 && y + 2 <= 7 && plansza[x + 1][y + 1] != pionek_gracza && (plansza[x + 1][y + 1] != pionek_gracza + 32 || plansza[x + 1][y + 1] != pionek_gracza - 32) && plansza[x + 2][y + 2] == ' ')	//warunki bicia
					{
						if (x + 2 == 7 && pionek_gracza == 'x')							//ruch z biciem
							plansza[x + 2][y + 2] = 'X';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x + 2][y + 2] = pionek_gracza - 32;
							else
								plansza[x + 2][y + 2] = pionek_gracza;
						plansza[x][y] = ' ';
						plansza[x + 1][y + 1] = ' ';
						usun_pionek(&A, &B, kogo_runda);
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
				}
				if (ruch_gracza[2] == '3')
				{
					if (x - 1 <= 7 && y + 1 <= 7 && plansza[x - 1][y + 1] == ' ')		//warunki poruszania sie na puste pole
					{
						/*if (wymagane_bicie)												//jesli gracz chce sie ruszyc na wolne pole ale moze bic, to wymuszam na nim ruch bijacy
						{
							poprawny_ruch = false;
							continue;
						}*/
						if (x - 1 == 0 && pionek_gracza == 'o')							//ruch bez biciem
							plansza[x - 1][y + 1] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 1][y + 1] = pionek_gracza - 32;
							else
								plansza[x - 1][y + 1] = pionek_gracza;
						plansza[x][y] = ' ';
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
					if (x - 2 >= 0 && y + 2 <= 7 && plansza[x - 1][y + 1] != pionek_gracza && (plansza[x - 1][y + 1] != pionek_gracza + 32 || plansza[x - 1][y + 1] != pionek_gracza - 32) && plansza[x - 2][y + 2] == ' ')	//warunki bicia
					{
						if (x - 2 == 0 && pionek_gracza == 'o')							//ruch z biciem
							plansza[x - 2][y + 2] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 2][y + 2] = pionek_gracza - 32;
							else
								plansza[x - 2][y + 2] = pionek_gracza;
						plansza[x][y] = ' ';
						plansza[x - 1][y + 1] = ' ';
						usun_pionek(&A, &B, kogo_runda);
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
				}
				if (ruch_gracza[2] == '4')
				{
					if (x - 1 <= 7 && y - 1 <= 7 && plansza[x - 1][y - 1] == ' ')		//warunki poruszania sie na puste pole
					{
						if (x - 1 == 0 && pionek_gracza == 'o')							//ruch bez biciem
							plansza[x - 1][y - 1] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 1][y - 1] = pionek_gracza - 32;
							else
								plansza[x - 1][y - 1] = pionek_gracza;
						plansza[x][y] = ' ';
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
					if (x - 2 >= 0 && y - 2 >= 0 && plansza[x - 1][y - 1] != pionek_gracza && (plansza[x - 1][y - 1] != pionek_gracza + 32 || plansza[x - 1][y - 1] != pionek_gracza - 32) && plansza[x - 2][y - 2] == ' ')	//warunki bicia
					{
						if (x - 2 == 0 && pionek_gracza == 'o')							//ruch z biciem
							plansza[x - 2][y - 2] = 'O';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x - 2][y - 2] = pionek_gracza - 32;
							else
								plansza[x - 2][y - 2] = pionek_gracza;
						plansza[x][y] = ' ';
						plansza[x - 1][y - 1] = ' ';
						usun_pionek(&A, &B, kogo_runda);
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
					}
				}
			}
			if (ruch_gracza[0] == 'q' || buff == "qqq"){
				poprawny_ruch = true;
				if (kogo_runda == 'A')
					send(connection_socket_descriptor, "qqq", 3, 0);}

			if(!poprawny_ruch)
				cout << endl << "Niepoprawny ruch, sprobuj jeszcze raz:\t";
		}
		if(!koniec_gry)
			koniec_gry=sprawdz_czy_koniec_gry(&A, &B);
		if (koniec_gry){
			if (kogo_runda == 'A'){
				send(connection_socket_descriptor, "eee", 3, 0);}
			break;}

		if (ruch_gracza[0] == 'q'|| koniec_gry)
			break;
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
	//cin >> ruch_gracza;
	//send(connection_socket_descriptor, ruch_gracza, sizeof(ruch_gracza), 0);
	//cout <<sizeof(ruch_gracza);

   }
   if (sign == 'b'){
	kogo_runda = 'A';
	pionek_gracza = 'x';
	RunGame();
	//int ile = recv(connection_socket_descriptor, buff, sizeof(buff), 0);
	//cout <<ile;
  }
  if (sign == 'o'){
	cout<<"Bark wolnych pokoi."<<endl;
  }
  
  close(connection_socket_descriptor);
  return 0;

}
