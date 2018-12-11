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

const int PROCESS_COUNT = 10;   // Number of processes to be created
const int ITERATIONS = 7;       // Number of times each process will attempt to manipulate shared memory
const int BUFFSIZE = 4;         // Shared memory contains 4 elements

enum {
    CHECKING,
    SAVING,
    CREDIT,
    SAFE
};

void updateWithBeta(SEMAPHORE&, float*); // Accesses and increments a random element in the shared memory

void initsem(SEMAPHORE&); // Inititalizes all of the semaphores to 1

void initshmBUF(float*); // Initializes all elements of the memory shared buffer to 1.

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
    uniform_real_distribution<double> realDistribution(-0.5, 0.5); // For beta
    uniform_int_distribution<int> intDistribution(0, INT_MAX); // To randomize shared memory buffer index
    mt19937 generator(rd());
    float beta;
    int randomIndex;
    float oldValue;
    for(int k = 0; k < ITERATIONS; k++) {
        beta = realDistribution(generator);
        randomIndex = intDistribution(generator) % BUFFSIZE; // random index with range [0, BUFFSIZE-1]
        sem.P(randomIndex); // P operation to prevent other processes from accessing this shared buffer index.
        oldValue = shmBUF[randomIndex];
        shmBUF[randomIndex] = shmBUF[randomIndex] * (1 + beta); // increments shared memory element by a fraction of itself base on beta
        cout << "\nPID: " << getpid() 
            << " | \u03b2\: " << beta 
            << " | Old Value BUFF[" << randomIndex << "] = " << oldValue 
            << " | New Value: (BUFF[" << randomIndex << "] += BUFF[" << randomIndex << "] * \u03b2\) = " << shmBUF[randomIndex]
            << endl;
        sem.V(randomIndex);
        
    }
}


