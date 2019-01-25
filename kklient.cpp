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

#define BUF_SIZE 1024
#define NUM_THREADS     5

char kogo_runda;
char pionek_gracza;

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

///////////////////////////////////////////////////////////

struct gracz{
	int ilosc_pionkow;
	bool wygrany;
};

void zbuduj_plansze(char plansza[8][8])
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (j % 2 == 1 && i % 2 == 0)	//ustawienie '-' w nieparzystych wierszach
			{
				plansza[i][j] = '-';
				continue;
			}
			if (j % 2 == 0 && i % 2 == 1)	//ustawienie '-' w parzystych wierszach
			{
				plansza[i][j] = '-';
				continue;
			}
			if (i < 2)						//ustawienie pionkow gracza A - 'x'
			{
				plansza[i][j] = 'x';
				continue;
			}
			if (i > 5)						//ustawienie pionkow gracza B - 'o'
			{
				plansza[i][j] = 'o';
				continue;
			}
			plansza[i][j] = ' ';
		}
	}
}

void wyswietl_pomoc()
{
	cout << "-----------------------------------------------"<<endl;
	cout << "\tPOMOC:" << endl
		<< "Poruszanie: <wiersz><kolumna><kierunek>" << endl
		<< "Mozliwe kierunki:\t1 - lewo dol\t2 - prawo dol" << endl
		<< "\t\t\t3 - prawo gora\t4 - lewo gora" << endl
		<< "q - zakoncz gre" << endl;
	cout << "---------------------------------------------" << endl;
}

void wyswietl_plansze(char plansza[8][8])
{
	char literka_wiersza = 'A';	//literka ktora bedzie wyswietlac sie z lewej strony planszy oznajczajaca jej wiersz

	cout << "\tPLANSZA:" << endl;
	cout << "\t  +--------+" << endl;
	for (int i = 0; i < 8; i++)
	{
		cout << "\t"<<literka_wiersza<<" |";
		for (int j = 0; j < 8; j++)
		{
			cout << plansza[i][j];
		}
		cout << "|" << endl;
		literka_wiersza++;
	}
	cout << "\t  +--------+" << endl;
	cout << "\t   12345678" << endl;
	cout << "---------------------------------------------" << endl;
}

void wyswietl_kogo_runda(char kogo_runda,char pionek_gracza)
{
	cout << "\tRUNDA GRACZA " << kogo_runda << endl;
	cout << "Twoje pionki to " << pionek_gracza << endl;
	cout << "Twoj ruch:\t";
}

bool sprawdz_poprawnosc_ruchu(char plansza[8][8], char ruch_gracza[3], char kogo_runda,bool wymagane_bicie)
{
	if (ruch_gracza[0] >= '1'&&ruch_gracza[0] <= '8')			//sprawdzanie pierwszego znaku rozkazu ruchu (czy jest DUZA litera)
		if (ruch_gracza[1] >= '1'&&ruch_gracza[1] <= '8')		//sprawdzanie drugiego znaku rozkazu ruchu (czy jest cyfra)
			if (ruch_gracza[2] <= '4'&&ruch_gracza[2] >= '1')	//sprawdzanie trzeciego znaku czy jest z zakresu 1-4
			{
				if (((plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'x' && (( ruch_gracza[2] != '3' && ruch_gracza[2] != '4' ) || wymagane_bicie )) || plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'X') && kogo_runda == 'A')	//czy pionek nalezy do gracza, oraz czy male pionki nie chca sie "cofac", zezwalam na "cofanie" jesli jest bicie
					return true;
				if (((plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'o' && (( ruch_gracza[2] != '1' && ruch_gracza[2] != '2' ) || wymagane_bicie ))  || plansza[ruch_gracza[0] - '1'][ruch_gracza[1] - '1'] == 'O') && kogo_runda == 'B')
					return true;
			}
	return false;												//zwroc false jesli cokolwiek sie nie zgadzalo
}


char zmien_pionek(char kogo_runda)
{
	if (kogo_runda == 'A')										//decyzja jaki jest pionek aktualnego gracza
		return 'x';
	else
		return 'o';
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

void gratulacje(gracz *A, gracz *B)
{
	system("cls");
	if (A->wygrany)
		cout << "GRATULACJE DLA GRACZA A";
	else if (B->wygrany)
		cout << "GRATULACJE DLA GRACZA B";
	else
		cout << "GRA ZAKONCZONA BEZ ZWYCIEZCY :(";
}

bool czy_mozliwy_ruch(char plansza[8][8], char pionek_gracza)
{
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (plansza[i][j] == pionek_gracza)
			{
				if (pionek_gracza == 'x')
				{
					if (i < 7 && j > 0 && j <= 7 && plansza[i + 1][j - 1] == ' ')
						return true;
					if (i < 7 && j > 0 && j <= 7 && plansza[i + 1][j + 1] == ' ')
						return true;
				}
				if (pionek_gracza == 'o')
				{
					if (i > 0 && j > 0 && j <= 7 && plansza[i - 1][j - 1] == ' ')
						return true;
					if (i > 0 && j > 0 && j <= 7 && plansza[i - 1][j + 1] == ' ')
						return true;
				}
			}

		}
	}
	return false;
}

int RunGame()
{
	gracz A;														//pojawienie zawodnikow i ustalenie ilosci ich pionkow na 8
	A.ilosc_pionkow = 8;
	A.wygrany = false;
	gracz B;
	B.ilosc_pionkow = 8;
	B.wygrany = false;

	int x, y;														//wspolrzedne wybranego pionka ktory bedzie poruszany
	char ruch_gracza[20];
	char buff[20];											//zmienna ruchu wykowynawenego , jest odpowiednio duzy aby dlugi rozkaz nie nadpisal koncowego nulla
	//char kogo_runda = 'B';											//zmienna ustalajaca ktory gracz teraz sie rusza
	//char pionek_gracza = 'o';										//zmienna ustalajaca ktory jest pionek aktualnego gracza
	bool poprawny_ruch=false;										//czy ruch byl wykonany(napisany) poprawnie
	bool koniec_gry = false;										//zmienna oznaczajaca zakonczenie rozgrywki
	bool wymagane_bicie = false;									//zmienna ustalajaca czy na graczu wymagane jest zbicie pionka przeciwnika
	bool mozliwy_ruch = true;										//zmienna sprawdzajaca czy gracz nie zostal zablokowany

	char plansza[8][8];												//pojawienie planczy i ustalenie pol nieuzywanych oraz x i o
	zbuduj_plansze(plansza);										//budowanie planszy

	while (true) {													//wlasciwa rozgrywka
		poprawny_ruch = false;
		wymagane_bicie = false;
		//kogo_runda=zamien_gracza(kogo_runda);						//decyzja kto wykonuje ruch
    
    		if (kogo_runda == 'A')								//decyzja kto teraz wykonuje swoj ruch
		  kogo_runda='B';
	  	else
		  kogo_runda='A';
    
		pionek_gracza=zmien_pionek(kogo_runda);						//decyzja jakie pionki poruszamy
		system("cls");												//czyszczenie ekranu
		wyswietl_pomoc();											//wyswietlenie pomocy
		wyswietl_plansze(plansza);									//wyswietlenie planszy
		wymagane_bicie = czy_wymagane_bicie(plansza,pionek_gracza);	//sprawdzenie czy na mapie sa wymagane bicia
		mozliwy_ruch = czy_mozliwy_ruch(plansza, pionek_gracza);	//sprawdzenie czy gracz moze wykonac poprawny ruch
		if (!mozliwy_ruch)							//jesli nie moze wykonac to gra powinna sie zakonczyc
		{
			koniec_gry = true;
			if (kogo_runda == 'A')
				B.wygrany = true;
			else
				A.wygrany = true;
		}
		wyswietl_kogo_runda(kogo_runda, pionek_gracza);				//wyswietlanie kogo ruch

		while (!poprawny_ruch)										//ruch gracza ze sprawdzaniem jego poprawnosci
		{
			if (kogo_runda == 'A')
				cin >> ruch_gracza;
			if (kogo_runda == 'B'){
				recv(connection_socket_descriptor, buff, sizeof(buff), 0);
				for (int i = 0 ; i < 3 ; i++) {
					ruch_gracza[i] = buff[i];}
					//cout << buff[i] << endl;}
				/*if(recv(connection_socket_descriptor, buff, sizeof(buff), 0)==3){
					for (int i = 0 ; i < 3 ; i++) {
					//ruch_gracza[i] = buff[i];
					cout << buff[i] << endl;}
					//ruch_gracza = buff ;*/
					
				}

			poprawny_ruch = sprawdz_poprawnosc_ruchu(plansza, ruch_gracza, kogo_runda, wymagane_bicie);	//sprawdzenie poprawnosci ruchu
			x = ruch_gracza[0] - '1';								//ktory wiersz na planszy
			y = ruch_gracza[1] - '1';								//ktora kolumna na planszy

			if (poprawny_ruch)										//ruch pionka na pole puste i ewentualne bicie
			{
				poprawny_ruch = false;
				if (ruch_gracza[2] == '1')
				{
					if (x + 1 <= 7 && y - 1 >= 0 && plansza[x + 1][y - 1] == ' ')		//warunki poruszania sie na puste pole
					{
						if (x + 1 == 7 && pionek_gracza == 'x')							//ruch bez bicia
							plansza[x + 1][y - 1] = 'X';
						else
							if (plansza[x][y] == 'X' || plansza[x][y] == 'O')
								plansza[x + 1][y - 1] = pionek_gracza - 32;
							else
								plansza[x + 1][y - 1] = pionek_gracza;
						plansza[x][y] = ' ';
						poprawny_ruch = true;
						if (kogo_runda == 'A')
							//B.wygrany = true;
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
					}
					if (x + 2 <= 7 && y - 2 >= 0 && plansza[x + 1][y - 1] != pionek_gracza && (plansza[x + 1][y - 1] != pionek_gracza + 32 || plansza[x + 1][y - 1] != pionek_gracza - 32) && plansza[x + 2][y - 2] == ' ')	//warunki bicia
					{
						if (x + 2 == 7 && pionek_gracza == 'x')							//ruch z biciem
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
							//B.wygrany = true;
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
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
							send(connection_socket_descriptor, &poprawny_ruch, sizeof(poprawny_ruch), 0);
					}
				}
			}
			if (ruch_gracza[0] == 'q')													//zwroc true jesli chcemy zakonczyc gre
				poprawny_ruch = true;
				if (kogo_runda == 'A')
					//B.wygrany = true;
					SendMessage('e');

			if(!poprawny_ruch)
				cout << endl << "Niepoprawny ruch, sprobuj jeszcze raz:\t";
		}
		if(!koniec_gry)
			koniec_gry=sprawdz_czy_koniec_gry(&A, &B);
		if (koniec_gry)
			break;

		if (ruch_gracza[0] == 'q'|| koniec_gry || ruch_gracza[0]=='e' )
			break;
	}
	gratulacje(&A, &B);
	return 0;
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

/////////////
   ReceiveMessage();
   if (sign == 'w'){
	//RunGame(); //Chess chess(window, networkConnection);
	kogo_runda = 'B';
	pionek_gracza = 'x';
	printf("polaczono");
	RunGame();

   }
   if (sign == 'b'){
	kogo_runda = 'A';
	pionek_gracza = 'o';
	printf("polaczono");
	RunGame();
  }//}
  
  //close(connection_socket_descriptor);
  return 0;

}
