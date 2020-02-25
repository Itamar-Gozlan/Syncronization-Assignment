#include "Barrier.h"

Barrier::Barrier(unsigned int n) : barrier_n(n), threads_count(0) {
	sem_init(&counter_lock, 0, 1); // binary semaphore to protect counter updating
	sem_init(&stop1, 0, 0); // controlling in flow
	sem_init(&stop2, 0, 0); // controlling out flow
}

void Barrier::wait() {
	unsigned int i;
	sem_wait(&counter_lock);
	threads_count++;
	if (threads_count == barrier_n) { // when all processes arrived to the barrier
		for(i = 0; i < barrier_n; i++) sem_post(&stop1); // unlock stop1
	}
	sem_post(&counter_lock);

	sem_wait(&stop1); // when all processes arrive, stop1 will open

	// at this point stop1 is open and all processes are now in the barrier
	// and are now ready to exit the barrier.

	sem_wait(&counter_lock);
	threads_count--;
	if (threads_count == 0) { // when all processes are ready to exit the barrier.
		for(i = 0; i < barrier_n; i++) sem_post(&stop2); // unlock stop2
	}
	sem_post(&counter_lock);

	sem_wait(&stop2); // when all processes are ready to exit the barrier,
					  // stop2 will open, and all processes will exit.


}

Barrier::~Barrier() {
	sem_destroy(&counter_lock);
	sem_destroy(&stop1);
	sem_destroy(&stop2);
}


