#ifndef BARRIER_H_
#define BARRIER_H_

#include <semaphore.h>
#include <iostream>
using std::cout;
using std::endl;

class Barrier {
public:
	Barrier(unsigned int n);

	void wait();
	~Barrier();

protected:
	sem_t counter_lock;
	sem_t stop1;
	sem_t stop2;

	//shared values - must be protected!
	unsigned int barrier_n;
	unsigned int threads_count;
	
};

#endif // BARRIER_H_

