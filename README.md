The code in this repository was written in my 4th semester as part of "Operating Systems" course
This assignment that was focused on synchronization was definitely my favorite in this course.

The assignment was composed of two separate parts "Barrier" and "Reception Hour".

Part I - Barrier
An N-process barrier is a synchronization mechanism that allows N threads, where N is a fixed
number, to wait until all of them have reached a certain point. Once all threads have reached
this certain point (the barrier), they may all continue

Implementation Requirements
- Using only standard POSIX threads 
- Implementation use only one synchronization primitive: semaphores

Part II - Reception Hour:
As you probably know, each TA in the operating systems course holds a weekly reception hour.
The TA sleeps as long as there are no students in his office. When a student arrives he wakes
up the TA and starts asking a question. To synchronize between the many students that may
arrive simultaneously, the following must hold:
(1) Only one student asks a question at any given moment.
(2) After asking a question, the student waits for the TA’s answer. The student leaves only
when the TA finished answering.
(3) A student may only ask a question after the TA finished answering the previous student’s
question (or the student was the first to arrive at the reception hour).
(4) The TA’s office is small and contains only N (will be given as a parameter) seats. If a
student arrives and can’t find a free seat, he leaves immediately.
(5) At a certain point, the TA decides to finish the reception hour by closing the door. After
closing the door, the TA finishes answering the students who are left in his room.
Students that arrive when the door is closed leave immediately.

Implementation Requirements
- you are not allowed to use semaphores or any synchronization primitives (besides pthread_mutex_t and pthread_cond_t)
- Use only standard POSIX threads and synchronization functions