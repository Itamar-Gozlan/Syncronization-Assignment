This program simulate a simple Readers–writer lock mechanism that is more "fair" towards the writers
Any reader that want to read get permission but when a writer wish to get access all the readers afterwards have to wait
This protocol might not solve starvation completly but it is a better solution for writers
The program uses main's data strucutre in round-robin, strucutre sizes can be changes by using NUM_THREADS define

The code was written independately to maintain and improve my pthreads and synchronization skills
Program code is inspired by "Operating System" course in the Technion, mainly from the tutoroal synchronization slides by Idan Yaniv

How to use the program:
each line takes 2 arguments
In each line, the user have to write 'quit' or specify 2 arguments '<type> <sleep time>'
when type is "read" or "write" wich indicated the activity the user want to specify
and sleeping time is number between 0 to 5
"to end the program type \"quit\"

compile it with:
g++ -std=c++11 -g -Wall main.cxx -o executable -lpthread

running example:
./executable
Please type the commands you with to perform in the format <type> <sleep time>
when type is "read" or "write"
and time is number between 0 to 5
to end the program type "quit"
read 2
read 2
write 1
write 1
read 3
read 3
write 1
Thread ID, 4 is writing!
Thread ID, 3 is writing!
Thread ID, 7 is writing!
Thread ID, 1 is reading!
Thread ID, 2 is reading!
Thread ID, 6 is reading!
Thread ID, 5 is reading!
quit