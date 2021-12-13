#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"
#define REZERWA 500
#define TOR 1
#define START 2
#define JAZDA 3
#define KONIEC_JAZDY 4
#define KATASTROFA 5
#define TANKUJ 5000
int paliwo = 5000;
int ZJEZDZAJ=1, NIE_ZJEZDZAJ=0;
int liczba_procesow;
int nr_procesu;
int ilosc_aut;
int ilosc_torow=4;
int ilosc_zajetych_torow=0;
int tag=1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;
void Wyslij(int nr_auta, int stan)
{
    wyslij[0]=nr_auta;
    wyslij[1]=stan;
    MPI_Send(&wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}
void Tor(int liczba_procesow)
{
    int nr_auta,status;
    ilosc_aut = liczba_procesow -1;

    printf("Dysponujemy %d torami\n", ilosc_torow);
    sleep(2);
    while(ilosc_torow<=ilosc_aut)
    {
        MPI_Recv(&odbierz,2,MPI_INT,MPI_ANY_SOURCE,tag,MPI_COMM_WORLD, &mpi_status);//odbieram od kogokolwiek
        nr_auta=odbierz[0];
        status=odbierz[1];
        if(status==1)
        {
            printf("Auto o numerze %d nie wyruszylo z toru\n", nr_auta);
        }
        if(status==2)
        {
            printf("Auto o numerze %d wyjezdza z toru %d \n", nr_auta, ilosc_zajetych_torow);
            ilosc_zajetych_torow--;
        }
        if(status==3)
        {
            printf("Auto o numerze %d jest w trasie \n", nr_auta);
        }
        if(status==4)
        {
            if(ilosc_zajetych_torow<ilosc_torow)
            {
                ilosc_zajetych_torow++;
                MPI_Send(&ZJEZDZAJ, 1, MPI_INT, nr_auta, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&NIE_ZJEZDZAJ, 1, MPI_INT, nr_auta, tag, MPI_COMM_WORLD);
            }
        }
        if(status==5)
        {
            ilosc_aut--;
            printf("Ilosc aut %d\n", ilosc_aut);
        }
    }
    printf("Program zakonczyl dzialanie:)\n");
}
void Auto()
{
    int stan,suma,i;
    stan=JAZDA;
    while(1)
    {
        if(rand()%40 == 5) {
			stan=KOLIZJA;
			Wyslij(nr_procesu,stan);
			printf("Auto po ostatniej trasie nie nadaje sie do jazdy \n");
			return;
		}
        if(stan==1)
        {
            if(rand()%2==1)
            {
                stan=START;
                paliwo=TANKUJ;
                printf("Do wyjazdu przygotowuje sie auto o numerze %d\n",nr_procesu);
                Wyslij(nr_procesu,stan);
            }
            else
            {
                Wyslij(nr_procesu,stan);
            }
        }
        else if(stan==2)
        {
            printf("Wyruszyl samochod o numerze %d\n",nr_procesu);
            stan=JAZDA;
            Wyslij(nr_procesu,stan);
        }
        else if(stan==3)
        {
            paliwo-=rand()%500;
            if(paliwo<=REZERWA)
            {
                stan=KONIEC_JAZDY;
                printf("Auto o numerze %d potrzebuje zatankowac \n",nr_procesu);
                Wyslij(nr_procesu,stan);
            }
            else
            {
                for(i=0; rand()%10000; i++);
            }
        }
        else if(stan==4)
        {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp==ZJEZDZAJ)
            {
                stan=TOR;
                printf("Skonczylem trase %d\n", nr_procesu);
            }
            else
            {
                paliwo-=rand()%500;
                if(paliwo>0)
                {
                    Wyslij(nr_procesu,stan);
                }
                else
                {
                    stan=KATASTROFA;
                    printf("rozbilem sie\n");
                    Wyslij(nr_procesu,stan);
                    return;
                }
            }
        }
    }
}
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD,&liczba_procesow);
    srand(time(NULL));
    if(nr_procesu == 0)
        Tor(liczba_procesow);
    else
        Auto();
    MPI_Finalize();
    return 0;
}
