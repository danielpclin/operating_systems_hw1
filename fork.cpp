#include <iostream>
#include <unistd.h>

int main(){
    pid_t PID = fork();

    switch(PID){
        case -1:
            std::cerr << "fork() failed";
            exit(-1);
        // child process
        case 0:
            sleep(2);
            std::cout << "I'm Child process\n";
            std::cout << "Child Process pid from fork(): " << PID << "\n";
            std::cout << "Child's PID is " << getpid() << "\n";
            break;

        // parent process
        default:
            int exit_status;
            wait(&exit_status);
            std::cout << "I'm Parent process\n";
            std::cout << "Parent Process pid from fork(): " << PID << "\n";
            std::cout << "Parent's PID is " << getpid() << "\n";
    }

    return 0;
}