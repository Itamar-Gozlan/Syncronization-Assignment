#include "ReceptionHour.h"


//========== Threads =========//

void* threadStudent(void *student_arg){
	StudentArgs *args = (StudentArgs*)student_arg;

	ReceptionHour *rh = args->_rh;
	unsigned int id = args->_id;
	free(student_arg);

	StudentStatus status = rh->waitForTeacher();
	rh->studentSetStatus(id,status);
	if (status != ENTERED) {
		return NULL;
	}
	pthread_mutex_lock(&rh->mutex_student);
	
	while(!rh->TA_is_waiting) {
		pthread_cond_wait(&rh->ta_ready, &rh->mutex_student);
	}
	
	rh->TA_is_waiting = false;
	pthread_cond_signal(&rh->student_arrived);

	rh->askQuestion();
	rh->waitForAnswer();

	pthread_mutex_unlock(&rh->mutex_student);

	return NULL;
	//will end - but still exist in map!
}

//during the entire run TA must be responsive
void* threadTA(void *_rh){

	ReceptionHour *rh = (ReceptionHour*)_rh;
	
	while(!rh->receptionHourEnd()){

		if(rh->waitForStudent()){
			rh->waitForQuestion();
			if (rh->isDoorClose()) break;
			rh->giveAnswer();
		}
	}
	return NULL;
}

//========== C'tor and D'tor =========//

ReceptionHour::ReceptionHour(unsigned int max_waiting_students) :
	TA_is_waiting(false),
	TA_wait_for_question(false),
	students_limit(max_waiting_students),
	free_seats(max_waiting_students),
	door_closed(false),
	student_wait_for_answer(false) {

	//map default constructor - no need for default c'tor

	pthread_mutex_init(&mutex_door_closed, NULL);
	pthread_mutex_init(&mutex_student, NULL);

	pthread_mutex_init(&mutex_map, NULL);
	pthread_mutex_init(&mutex_free_seats, NULL);

	pthread_mutex_init(&wait_for_student, NULL);
	pthread_mutex_init(&wait_for_ta, NULL);
	pthread_cond_init(&ta_ready, NULL);
	pthread_cond_init(&student_arrived, NULL);

	pthread_mutex_init(&wait_for_question, NULL);
	pthread_cond_init(&student_question, NULL);
	pthread_cond_init(&ta_ready_for_question, NULL);

	pthread_mutex_init(&wait_for_answer, NULL);
	pthread_cond_init(&ta_answer, NULL);

	pthread_create(&thread_ta,NULL,threadTA, this);
}


ReceptionHour::~ReceptionHour() {
	//map default dt'or - no need for default d'tor
	//notice that Student class does pthread_join
	pthread_join(thread_ta, NULL);

	pthread_mutex_destroy(&mutex_door_closed);
	pthread_mutex_destroy(&mutex_student);

	pthread_mutex_destroy(&mutex_map);
	pthread_mutex_destroy(&mutex_free_seats);

	pthread_mutex_destroy(&wait_for_student);
	pthread_mutex_destroy(&wait_for_ta);
	pthread_cond_destroy(&ta_ready);
	pthread_cond_destroy(&student_arrived);

	pthread_mutex_destroy(&wait_for_question);
	pthread_cond_destroy(&student_question);
	pthread_cond_destroy(&ta_ready_for_question);

	pthread_mutex_destroy(&wait_for_answer);
	pthread_cond_destroy(&ta_answer);
}


//========== RH methods - to be used by user =========//


//Q: what if we already have a thread with the same id? //assume we don't - by Idan
void ReceptionHour::startStudent(unsigned int id) {
	StudentArgs	*student_arg = (StudentArgs*)malloc(sizeof(StudentArgs)); //will be deleted by studentThread
	if(!student_arg){
		cout<<"ERROR - can't allocate memory!"<<endl;
		return;
	}
	student_arg->_rh = this;
	student_arg->_id = id;

	pthread_mutex_lock(&mutex_map);
	//status will be updated by studentThread
	this->students_data.emplace(id, Student(ENTERED));	
	std::unordered_map<unsigned int, Student>::iterator it = students_data.find(id);
	pthread_t *curr = it->second.getThread();
	pthread_mutex_unlock(&mutex_map);

	pthread_create(curr, NULL, threadStudent, (void*)student_arg);
}

StudentStatus ReceptionHour::finishStudent(unsigned int id) {
	

	pthread_mutex_lock(&mutex_map);
		std::unordered_map<unsigned int, Student>::iterator it = students_data.find(id);
		pthread_t *curr = it->second.getThread();
	pthread_mutex_unlock(&mutex_map);
		pthread_join(*(curr),NULL);

	pthread_mutex_lock(&mutex_map);
		StudentStatus status = it->second.getStatus();
		this->students_data.erase(it);
	pthread_mutex_unlock(&mutex_map);


	return status;
}

void ReceptionHour::closeTheDoor() {
	pthread_mutex_lock(&mutex_door_closed);
	door_closed = true;

	// notify TA to stop waiting if needed
	if (count_free_seats() == students_limit) {
		if (TA_is_waiting) {
			pthread_mutex_lock(&wait_for_student);
			TA_is_waiting = false;
			pthread_cond_signal(&student_arrived);
			pthread_mutex_unlock(&wait_for_student);
		}
		if (TA_wait_for_question) {
			pthread_mutex_lock(&wait_for_question);
			TA_wait_for_question = false;
			pthread_cond_signal(&student_question);
			pthread_mutex_unlock(&wait_for_question);
		}
	}
	pthread_mutex_unlock(&mutex_door_closed);
}

bool ReceptionHour::receptionHourEnd(){

	bool res;
	pthread_mutex_lock(&mutex_door_closed);
	res = door_closed && room_is_empty();
	pthread_mutex_unlock(&mutex_door_closed);

	return res;
}


//========== RH methods - to be used by TA thread =========//

bool ReceptionHour::waitForStudent() {
	bool res = false;

	pthread_mutex_lock(&wait_for_student);

	//False if there are no more students and the door has closed.
	if(door_closed && (count_free_seats() == students_limit)){
		res = false;
	}
	else {
		//wait for student
		TA_is_waiting = true;
		pthread_cond_signal(&ta_ready);
		// printDebug("[TA is waiting for a student to enter the room...]");

		while(TA_is_waiting){
			pthread_cond_wait(&student_arrived, &wait_for_student); // signalled from arriving sudent OR closed door
		}
		if (count_free_seats() == students_limit && door_closed) {
			res = false;
		} else {
			res = true;	
			// printDebug("[TA noticed a student in the room...]");
		}
	}
	pthread_mutex_unlock(&wait_for_student);
	//True if a new student has arrived.
	return res;
}

void ReceptionHour::waitForQuestion() {

	pthread_mutex_lock(&wait_for_question);
	
	TA_wait_for_question = true;
	pthread_cond_signal(&ta_ready_for_question);

	while(TA_wait_for_question){
		pthread_cond_wait(&student_question, &wait_for_question);
	}

	pthread_mutex_unlock(&wait_for_question);
}

void ReceptionHour::giveAnswer() {

	pthread_mutex_lock(&wait_for_answer);
	
	while(!student_wait_for_answer){
		pthread_cond_wait(&ta_answer, &wait_for_answer);
	}
	student_wait_for_answer = false;	
	
	pthread_cond_signal(&ta_answer);

	pthread_mutex_unlock(&wait_for_answer);
}

//========== RH methods - to be used by student thread =========//

StudentStatus ReceptionHour::waitForTeacher() {

	if(isDoorClose()) return LEFT_BECAUSE_DOOR_CLOSED;

	if(!atomic_free_seats_deacrease()) return LEFT_BECAUSE_NO_SEAT;
	
	//Entered	
	return ENTERED;
}

void ReceptionHour::askQuestion() {

	//if we are here, student ENTERED
	pthread_mutex_lock(&wait_for_question);
	while(!TA_wait_for_question) {
		pthread_cond_wait(&ta_ready_for_question, &wait_for_question);
	}
	TA_wait_for_question = false;
	
	pthread_cond_signal(&student_question);

	pthread_mutex_unlock(&wait_for_question);
}

void ReceptionHour::waitForAnswer() {

	pthread_mutex_lock(&wait_for_answer);

	student_wait_for_answer = true; 
	pthread_cond_signal(&ta_answer);
	while(student_wait_for_answer){
		pthread_cond_wait(&ta_answer, &wait_for_answer);
	}

	atomic_free_seats_increase();	 //get up!

	pthread_mutex_unlock(&wait_for_answer);
}

void ReceptionHour::studentSetStatus(unsigned int id, StudentStatus status){

	pthread_mutex_lock(&mutex_map);
		std::unordered_map<unsigned int, Student>::iterator it = this->students_data.find(id);
		assert(it != students_data.end());
		it->second.setStatus(status);
	pthread_mutex_unlock(&mutex_map);
	//according to Idan - insert always succeed
}

//========== aux functions for atomic use of free_seats shared var =========//

void ReceptionHour::atomic_free_seats_increase(){

		pthread_mutex_lock(&mutex_free_seats);
	 	free_seats++;
	 	pthread_mutex_unlock(&mutex_free_seats);
}

bool ReceptionHour::atomic_free_seats_deacrease(){
		bool res;
		pthread_mutex_lock(&mutex_free_seats);
	 	if(free_seats == 0) res = false;
	 	else {
		 	free_seats--;
		 	res = true;
	 	}
	 	pthread_mutex_unlock(&mutex_free_seats);

	 	return res;
}

unsigned long ReceptionHour::count_free_seats(){

		unsigned long res;
		pthread_mutex_lock(&mutex_free_seats);
	 	res = free_seats;
	 	pthread_mutex_unlock(&mutex_free_seats);
	 return res; 
}

bool ReceptionHour::room_is_empty(){

		bool res;
		pthread_mutex_lock(&mutex_free_seats);
		res = (free_seats == students_limit);
	 	pthread_mutex_unlock(&mutex_free_seats);
	 	return res;
}

bool ReceptionHour::isDoorClose(){
	bool res = false;
	pthread_mutex_lock(&mutex_door_closed);
	res = door_closed;
	pthread_mutex_unlock(&mutex_door_closed);
	return res;
}



