#include <iostream>
#include <unistd.h> // POSIX funkcije -> fork(), getpid()...
#include <sys/ipc.h> //IPC_PRIVATE...
#include <sys/shm.h> //shmget(), shmat()...
#include <csignal> // sigset();
#include <chrono> // milisekunde
#include <wait.h> // wait();
#include <iomanip>  //setprecision
#include <sys/shm.h> //shmctl
#include <sys/sem.h> //semctl
#include <sys/msg.h> //msgctl

using namespace std;

#define N 30

struct ZajednickoPolje{
    int polje[N];
} *info_polje;

ZajednickoPolje *p = new ZajednickoPolje[N]; //generiranje N polja

void PrviProces (int brojprocesa)
{
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count(); //dobivanje vremena u nanosekundama
    srand(seed + getpid() + 1);
    
    for (int i = 0; i < N; i++)
    {
        p[brojprocesa].polje[i] = rand() % 10000;
    }
}

void DrugiProces (int brojprocesa)
{
    int zbroj = 0;
    for (int i = 0; i < N; i++){
        zbroj += p[brojprocesa].polje[i];
    }
    cout << "Polje " << brojprocesa << " zbroj = " << zbroj << endl;
    sleep(1);
    cout << "Polje " << brojprocesa << " aritmeticka sredina = " << fixed << setprecision(6) << float(zbroj) / N << endl;
}


void Brisi(int sig) //funkcija za brisanje zajednicke memorije
{
    sighold(sig);

    for (int i = 1; i < 1000000; i++)
    {
        if (shmctl(i, IPC_RMID, NULL) != -1) cout << "Obrisao zajednicku memoriju " << i << endl;
        if (semctl(i, 0, IPC_RMID, 0) != -1) cout << "Obrisao skup semafora " << i << endl;
        if (msgctl(i, IPC_RMID, NULL) != -1) cout << "Obrisao red poruka " << i << endl;
    }
    sigrelse(sig);
    exit(0);
}

int main (int argc, char *argv[])
{
   
    if (argc != 2) // ako je uneseno više ili manje od 1 elementa (1 je default, 2 označava 1 element unesen)
    {
        cout << "1 argument -> broj procesa\n";
        return 0;
    }

    int M = atoi(argv[1]); // M poprima vrijednost koja je unesena prilikom pokretanja programa

    sigset (SIGINT, Brisi); //U slučaju da pošaljemo signal SIGINT, vodi nas na funkciju Brisi

    int ID_mem = shmget (IPC_PRIVATE, sizeof(ZajednickoPolje), 0600); //Stvaranje segmenta zajednicke memorije
    info_polje = (ZajednickoPolje*)shmat(ID_mem, NULL, 0); //Vezanje procesa za segment zajednicke memorije
    
    p = info_polje; // N polja povezujemo sa zajedničkom memorijom 

    //Početak stvaranja M procesa prvog tipa
    for (int i = 0; i < M; i++)
    {
        switch(fork())
        {
            case 0:
                PrviProces(i);
                exit(0);
            case -1:
            {
                cout << "Fatalna pogreška!\n";
                exit(1);
            }
        }
    }
    //roditelj čeka djecu kako ne bi bilo procesa zombija ni procesa siročića
    for (int i = 0; i < M; i++) wait(NULL); 

    //Početak stvaranja M procesa drugog tipa
    for (int i = 0; i < M; i++)
    {
        switch(fork())
        {
            case 0:
                DrugiProces(i);
                exit(0);
            case -1:
            {
                cout << "Fatalna pogreška!\n";
                exit(1);
            }
        }
    }
    //roditelj čeka djecu kako ne bi bilo procesa zombija ni procesa siročića
    for (int i = 0; i < M; i++) wait(NULL);

    if (shmdt(info_polje) == -1) //ako ne uspije odspojiti procese od segmenta vraća error poruku
    {
        perror("shmctl");
        exit(1);
    } 
    if (shmctl(ID_mem, IPC_RMID, NULL) == -1) //ako ne uspije obrisati zajedničku memoriju vraća error poruku
    {
        perror("shmctl");
        exit(1);
    }

    return 0;
} 