#include <iostream>
#include <csignal>
#include <ctime>
#include <unistd.h>

using namespace std;

#define N 11 //definirano 10 razina prekida, zato jer nikad nećemo unesti prekid 0

int cekanje[N];
int prioritet[N];
int tekuci_prioritet;

//funkcija u kojoj obrađujemo prekid
void obradba_prekida(int j) {
   cout << "\nPoceo obradbu prekida " << j << endl;
   for (int i=1; i<=5; i++)
   {
    cout << "Obradba prekida " << j << ": " << i << "/5\n"; //obrada prekida od 5 sekundi
    sleep(1);
   }
  cout << "Zavrsio obradbu prekida " << j << endl;
}

void prekidna_rutina(int sig) {
   int i, x;
   time_t t;

   time(&t);
   cout << "\nPrekidna rutina pozvana u: " << ctime(&t);

   cout << "Upiši razinu prekida: ";
   cin >> i;
   
   if (i > N-1)
   {
      cout << "Ne postoji ta razina prekida, najveća razina prekida iznosi " << N - 1 << endl;
      sleep(1);
      return;
   }

   cekanje[i] = 1; 

   do {
      // odredi prekid najvećega prioriteta koji čeka obradbu 
     x = 0;
      for (int j = tekuci_prioritet + 1; j < N; j++) {
         if (cekanje[j] != 0)
         {
            x = j; //pronalazi trenutno najveci prioritet koji ceka obradu
         } 
      }

      // ako postoji prekid koji čeka i prioritetniji je od trenutačnoga posla, idi u obradbu 
      if (x > 0){
         cekanje[x] = 0;
         prioritet[x] = tekuci_prioritet;
         tekuci_prioritet = x;
         sigrelse(sig); //omogući prekidanje;
         obradba_prekida(x);
         sighold(sig);  //zabrani prekidanje;
         tekuci_prioritet = prioritet[x];
      }
   } while (x > 0);
}

int main() {

   sigset(SIGINT, prekidna_rutina);
   
   cout << "Poceo osnovni program\n";
   for (int i=1; i<=10; i++)
   {
    cout << "Osnovni program: " << i << "/10\n";
    sleep(1);
   }
   cout << "Zavrsio osnovni program\n";

   return 0;
}