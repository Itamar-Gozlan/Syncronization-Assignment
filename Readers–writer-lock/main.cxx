#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <string> 
#include <unistd.h>


using std::cout;
using std::cin;
using std::endl;
using std::getline;
using std::string;
using std::stoi;
#define SIZE 1024
#define NUM_THREADS 10


/* --- Shared variables --- */
// locks
pthread_mutex_t global_lock;
// global counters
volatile int number_of_writers;
volatile int number_of_waiting_writers;
volatile int number_of_readers;

pthread_cond_t readers_condition;
pthread_cond_t writers_condition;

struct thread_data_t
{
	int tid;
	int sleep_time;
};

class Reader {

public:
	Reader(){}

	~Reader(){}

	void read_lock() {
		pthread_mutex_lock(&global_lock);
		while ((number_of_writers > 0) ||
		(number_of_waiting_writers > 0))
		pthread_cond_wait(&readers_condition, &global_lock);
		number_of_readers++;
		pthread_mutex_unlock(&global_lock);
	}

	void read_unlock() {
		pthread_mutex_lock(&global_lock);
		number_of_readers--;
		if (number_of_readers == 0)
		pthread_cond_signal(&writers_condition);
		pthread_mutex_unlock(&global_lock);
	}

private:
	pthread_t *thread;
};

class Wrtier {
public:
	Wrtier(){}

	~Wrtier(){}

	void write_lock() {
		pthread_mutex_lock(&global_lock);
		number_of_waiting_writers++;
		while ((number_of_writers > 0) ||
		(number_of_readers > 0))
		pthread_cond_wait(&writers_condition, &global_lock);
		number_of_waiting_writers--;
		number_of_writers++;
		pthread_mutex_unlock(&global_lock);
	}

	void write_unlock() {
		pthread_mutex_lock(&global_lock);
		number_of_writers--;
		pthread_cond_broadcast(&readers_condition);
		pthread_cond_signal(&writers_condition);
		pthread_mutex_unlock(&global_lock);
	}

};

void *threadReader(void* thread_data){
	thread_data_t *data = (thread_data_t*)thread_data;
	sleep(data->sleep_time);
	Reader *r = new Reader();
	// cout << "Thread ID, " << tid << "is locking global" << endl;
	r->read_lock();
	cout << "Thread ID, " << data->tid << " is reading!" << endl;
	r->read_unlock();
	// cout << "Thread ID, " << tid << "is unlocking global" << endl;	
	delete r;
    pthread_exit(NULL);
}

void *threadWriter(void* thread_data){
	thread_data_t *data = (thread_data_t*)thread_data;
	sleep(data->sleep_time);
	Wrtier *w = new Wrtier();
	// cout << "Thread ID, " << tid << "is locking global" << endl;
	w->write_lock();
	cout << "Thread ID, " << data->tid << " is writing!" << endl;
	w->write_unlock();
	// cout << "Thread ID, " << tid << "is unlocking global" << endl;	
	delete w;
    pthread_exit(NULL);
}

// simulating a simple readers-writers 

int main(){

	pthread_mutex_init(&global_lock, NULL);
	pthread_cond_init(&readers_condition, NULL);
	pthread_cond_init(&writers_condition, NULL);

	pthread_t threads[NUM_THREADS];
	thread_data_t thread_data[NUM_THREADS];

	int s_time, count = 1;
	bool input_invalid = true;
	int status;
	string type, time;
	cout<<"Please type the commands you with to perform in the format <type> <sleep time>"<<endl;
	cout<<"when type is \"read\" or \"write\""<<endl;
	cout<<"and time is number between 0 to 5"<<endl;
	cout<<"to end the program type \"quit\""<<endl;

	
	while(1){
		cin >> type;
		if(type == "quit"){
			break;
		}		
		cin >> time;
		try{
			s_time = stoi(time);	
		}
		catch (...){
			cout <<"Error: Invalid input! please use the correct format: \"type sleep_time\""<<endl;
			continue;
		}
		if(s_time < 0 || s_time > 5) s_time = 1; 

		thread_data[count % 10].tid = count;
		thread_data[count % 10].sleep_time = s_time;

		if(type == "read"){
			input_invalid = false;
			status = pthread_create(&threads[count % 10], NULL, threadReader, (void *)&thread_data[count % 10]);
			if (status) {
				cout << "Error:unable to create thread," << status << endl;
				exit(-1);
			}
		}

		if(type == "write"){
			input_invalid = false;
			status = pthread_create(&threads[count % 10], NULL, threadWriter, (void *)&thread_data[count % 10]);
			if (status) {
				cout << "Error:unable to create thread," << status << endl;
				exit(-1);
			}
		}



		if(input_invalid){
			cout << "Invalid input: "<<type<<endl;
			cout <<"Error: Invalid input! only \"read\" \"write\" and \"quit\" qualify as valid input" <<endl;
		}
		input_invalid = true;
		count++;
	}

	pthread_mutex_destroy(&global_lock);
	pthread_cond_destroy(&readers_condition);
	pthread_cond_destroy(&writers_condition);

	return 0;
}