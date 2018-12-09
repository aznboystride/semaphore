#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "semaphore.h"

using namespace std;

const int MAXTIME = 5;
const int BUFFSIZE = 4;

enum {
    CHECKING,
    SAVING,
    CREDIT,
    SAFE
};

void deposit(SEMAPHORE&, double*, int);

void withdraw(SEMAPHORE&, double*, int);

void initsem(SEMAPHORE&);

void initshmBUF(double*);

void parent_cleanup(SEMAPHORE&, int shmid);

int main()
{
    srand(time(NULL));
    int shmid;
    double* shmBUF;

    SEMAPHORE sem(4);
    initsem(sem);
    

    shmid = shmget(IPC_PRIVATE, BUFFSIZE*sizeof(double), PERMS);
    shmBUF = (double *) shmat(shmid, 0, SHM_RND);
    initshmBUF(shmBUF);

    for(int i = 0; i < 10; i++) {
        if(!fork()) {
            deposit(sem, shmBUF, rand() % 100);
            //withdraw(sem, shmBUF, rand() % 100);
        }
    }

    return 0;
}

void initsem(SEMAPHORE& sem) {
    sem.V(CHECKING);
    sem.V(SAVING);
    sem.V(CREDIT);
    sem.V(SAFE);
}

void initshmBUF(double* shmBUF) {
    for(int i = 0; i < BUFFSIZE; i++)
        shmBUF[i] = 0;
}

void parent_cleanup (SEMAPHORE &sem, int shmid) {

	int status;			/* child status */
	wait(0);	/* wait for child to exit */
	shmctl(shmid, IPC_RMID, NULL);	/* cleaning up */
	sem.remove();
} // parent_cleanup

void deposit(SEMAPHORE& sem, double* shmBUF, int amt) {
    for(int k = 0; k < MAXTIME; k++) {
        sem.P(CHECKING);
        cout << "\nPID: " << getpid() << " is making a deposit to CHECKINGS of amt " << amt << ". Current balance is " << shmBUF[CHECKING] << endl;
        shmBUF[CHECKING] += amt;
        cout << "\nPID: " << getpid() << " sees new balance of CHECKINGS to be: " << shmBUF[CHECKING] << endl;
        sem.V(CHECKING);

        sem.P(SAVING);
        cout << "\nPID: " << getpid() << " is making a deposit to SAVING of amt " << amt << ". Current balance is " << shmBUF[SAVING] << endl;
        shmBUF[SAVING] += amt;
        cout << "\nPID: " << getpid() << " sees new balance of SAVING to be: " << shmBUF[SAVING] << endl;
        sem.V(SAVING);

        sem.P(CREDIT);
        cout << "\nPID: " << getpid() << " is making a deposit to CREDIT of amt " << amt << ". Current balance is " << shmBUF[CREDIT] << endl;
        shmBUF[CREDIT] += amt;
        cout << "\nPID: " << getpid() << " sees new balance of CREDIT to be: " << shmBUF[CREDIT] << endl;
        sem.V(CREDIT);

        sem.P(SAFE);
        cout << "\nPID: " << getpid() << " is making a deposit to SAFE of amt " << amt << ". Current balance is " << shmBUF[SAFE] << endl;
        shmBUF[SAFE] += amt;
        cout << "\nPID: " << getpid() << " sees new balance of SAFE to be: " << shmBUF[SAFE] << endl;
        sem.V(SAFE);
        
    }
}

void withdraw(SEMAPHORE& sem, double* shmBUF, int amt) {
    for(int k = 0; k < MAXTIME; k++) {
        sem.P(CHECKING);
        cout << "\nPID: " << getpid() << " is making a withdraw to CHECKINGS of amt " << amt << ". Current balance is " << shmBUF[CHECKING] << endl;
        shmBUF[CHECKING] -= amt;
        cout << "\nPID: " << getpid() << " sees new balance of CHECKINGS to be: " << shmBUF[CHECKING] << endl;
        sem.V(CHECKING);

        sem.P(SAVING);
        cout << "\nPID: " << getpid() << " is making a withdraw to SAVING of amt " << amt << ". Current balance is " << shmBUF[SAVING] << endl;
        shmBUF[SAVING] -= amt;
        cout << "\nPID: " << getpid() << " sees new balance of SAVING to be: " << shmBUF[SAVING] << endl;
        sem.V(SAVING);

        sem.P(CREDIT);
        cout << "\nPID: " << getpid() << " is making a withdraw to CREDIT of amt " << amt << ". Current balance is " << shmBUF[CREDIT] << endl;
        shmBUF[CREDIT] -= amt;
        cout << "\nPID: " << getpid() << " sees new balance of CREDIT to be: " << shmBUF[CREDIT] << endl;
        sem.V(CREDIT);

        sem.P(SAFE);
        cout << "\nPID: " << getpid() << " is making a withdraw to SAFE of amt " << amt << ". Current balance is " << shmBUF[SAFE] << endl;
        shmBUF[SAFE] -= amt;
        cout << "\nPID: " << getpid() << " sees new balance of SAFE to be: " << shmBUF[SAFE] << endl;
        sem.V(SAFE);
        
    }
}


