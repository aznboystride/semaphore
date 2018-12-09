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
#include <random>
#include "semaphore.h"

using namespace std;

const int PROCESS_COUNT = 10;
const int ITERATIONS = 7;
const int BUFFSIZE = 4;

enum {
    CHECKING,
    SAVING,
    CREDIT,
    SAFE
};

void updateWithBeta(SEMAPHORE&, float*);

void initsem(SEMAPHORE&);

void initshmBUF(float*);

void parent_cleanup(SEMAPHORE&, int shmid);

int main()
{
    int shmid;
    float* shmBUF;

    SEMAPHORE sem(4);
    initsem(sem);
    

    shmid = shmget(IPC_PRIVATE, BUFFSIZE*sizeof(float), PERMS);
    shmBUF = (float *) shmat(shmid, 0, SHM_RND);
    initshmBUF(shmBUF);

    for(int i = 0; i < PROCESS_COUNT; i++) {
        if(!fork()) {
            updateWithBeta(sem, shmBUF);
        }
    }
    parent_cleanup(sem, shmid);

    return 0;
}

void initsem(SEMAPHORE& sem) {
    sem.V(CHECKING);
    sem.V(SAVING);
    sem.V(CREDIT);
    sem.V(SAFE);
}

void initshmBUF(float* shmBUF) {
    for(int i = 0; i < BUFFSIZE; i++)
        shmBUF[i] = 1;
}

void parent_cleanup (SEMAPHORE &sem, int shmid) {

	int status;			/* child status */
	wait(0);	/* wait for child to exit */
	shmctl(shmid, IPC_RMID, NULL);	/* cleaning up */
	sem.remove();
} // parent_cleanup

void updateWithBeta(SEMAPHORE& sem, float* shmBUF) {
    random_device rd;
    uniform_real_distribution<double> distribution(-0.5, 0.5);
    mt19937 generator(rd());
    float beta;
    for(int k = 0; k < ITERATIONS; k++) {
        beta = distribution(generator);
        sem.P(k % BUFFSIZE);
        cout << "\nPID: " << getpid() << " sees current value of buff[" << k % BUFFSIZE << "] to be: " << shmBUF[k % BUFFSIZE] << endl;
        cout << "\nPID: " << getpid() << " is updating buff[" << k % BUFFSIZE << "] of beta " << beta << endl;
        shmBUF[k % BUFFSIZE] += shmBUF[k % BUFFSIZE] * beta;
        cout << "\nPID: " << getpid() << " sees new value of buff[" << k % BUFFSIZE << "] to be: " << shmBUF[k % BUFFSIZE] << endl;
        sem.V(k % BUFFSIZE);
        
    }
}


