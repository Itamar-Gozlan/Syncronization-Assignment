#ifndef RECEPTIONHOUR_H_
#define RECEPTIONHOUR_H_

#include <unordered_map>
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#define DEBUG_MODE 0
extern pthread_mutex_t print_debug;

using std::unordered_map;
using std::cout;
using std::endl;

enum StudentStatus {
	ENTERED = 0,
	LEFT_BECAUSE_NO_SEAT,
	LEFT_BECAUSE_DOOR_CLOSED
};

class Student {
public:
	Student(StudentStatus st) : status(st) {
		thread = (pthread_t*)malloc(sizeof(pthread_t));
	}
	//copy c'tor to be used by map - pthread_create will only be called with the mapped ptr
	Student(const Student &stu) : status(stu.status) {
		thread = (pthread_t*)malloc(sizeof(pthread_t));
	}
	~Student() {free(thread);}
	pthread_t* getThread(){ return thread; }
	StudentStatus getStatus() {return status; }
	void setStatus(StudentStatus s){ status = s;}

private:
	pthread_t *thread;
	StudentStatus status;
};


class ReceptionHour {
public:
	ReceptionHour(unsigned int max_waiting_students);
	~ReceptionHour();

	void startStudent(unsigned int id);
	StudentStatus finishStudent(unsigned int id);
	
	void closeTheDoor();

	bool waitForStudent();
	void waitForQuestion();
	void giveAnswer();

	StudentStatus waitForTeacher();
	void askQuestion();
	void waitForAnswer();

protected:	
	unordered_map<unsigned int, Student> students_data;
	
	bool TA_is_waiting;
	bool TA_wait_for_question;
	unsigned int students_limit;
	unsigned long free_seats;
	bool door_closed;
	bool student_wait_for_answer;
	
	pthread_mutex_t mutex_door_closed;
	pthread_mutex_t mutex_student;
	pthread_mutex_t mutex_map;
	pthread_mutex_t mutex_free_seats;
	pthread_mutex_t wait_for_student;
	pthread_mutex_t wait_for_ta;
	pthread_cond_t ta_ready;
	pthread_cond_t student_arrived;
	pthread_mutex_t wait_for_question;
	pthread_cond_t student_question;
	pthread_cond_t ta_ready_for_question;
	pthread_mutex_t wait_for_answer;
	pthread_cond_t ta_answer;

	pthread_t thread_ta;

	bool receptionHourEnd();
	bool isDoorClose();
	void studentSetStatus(unsigned int id, StudentStatus status);
	void atomic_free_seats_increase();
	bool atomic_free_seats_deacrease();
	unsigned long count_free_seats();
	bool room_is_empty();

	friend void* threadStudent(void *student_arg);
	friend void* threadTA(void *_rh);
};

typedef struct student_args_t{
	ReceptionHour* _rh;
	unsigned int _id;
} StudentArgs;

#endif // RECEPTIONHOUR_H_

