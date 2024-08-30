#include <iostream>
#include <csignal> // sigset();
#include <pthread.h> //
#include <unistd.h> // sleep()
#include <atomic> // std::atomic;

using namespace std;

atomic<int> pravo;
atomic<int> zastavica[2] = {0}; // 0 ili 1

pthread_t dretva1, dretva2; //šifre dretvi

void brisi(int sig)
{
   pthread_cancel (dretva1); //prekid izvršavanja dretve 1
   pthread_cancel (dretva2); //prekid izvršavanja dretve 2
   exit(0); //izlaz iz programa
}

void ulaz_u_KO(int i, int j) {
   zastavica[i] = 1; //postavi zastavicu na 1
   while (zastavica[j] != 0){ //dok je zastavica druge dretve 1
      if (pravo == j) // ako druga dretva ima pravo
      {
         zastavica[i] = 0; //postavi zastavicu na 0
         while (pravo == j) {  //dok druga dretva ima pravo čekaj
            //čekaj
         }
         zastavica[i] = 1; //postavi zastavicu na 1
      }
   }
   //ako zastavica druge dretve nije 1, dretva ulazi u K.O.
}
 
void izlaz_iz_KO(int i, int j) 
{
   pravo = j; //druga dretva dobiva pravo
   zastavica[i] = 0; //postavi zastavicu na 0
}

void *proces(void *broj) {

   int i = *((int*)broj); //castanje i dereferenciranje i kako bi dobili vrijednost
   int j;
   if (i == 0) j = 1; // u slučaju da je i = 0, j postaje 1
   if (i == 1) j = 0; // u slučaju da je i = 1, j postaje 0
   int k, m;

   for (k = 1; k <= 5; k++)
   {
      ulaz_u_KO(i, j); //funkcija ulaska u kritični odsječak
      for (m = 1; m <= 5; m++){
          cout << "Dretva:" << i+1
          << ", K.O br: " << k
          << " (" << m << "/5)" << endl;
          usleep(500000); // spavanje pola sekunde
      } 
      izlaz_iz_KO(i, j); //funkcija izlaska iz kritičnog odsječka
   }
}


int main()
{
   sigset(SIGINT, brisi); //pozivanjem signala SIGINT idemo u funkciju brisi

   int ID1 = 0, ID2 = 1; 
   
   pthread_create (&dretva1, NULL, proces, &ID1); //kreiranje dretve broj 1
   usleep(100);
   pthread_create (&dretva2, NULL, proces, &ID2); //kreiranje dretve broj 2
   
   pthread_join (dretva1, NULL); //čekanje glavnog programa da dretva 1 završi
   pthread_join (dretva2, NULL); //čekanje glavnog programa da dretva 2 završi

   return 0;
}