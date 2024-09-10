/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <iostream>
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // TODO: add functionality
    // Create pipe
    //this array will hold the fild descriptors of the pipe
    int fd[2]; //pipefd[0] is for reading and pipefd[1] is for writing

    //check if pipe creation is successful
    if(pipe(fd) == -1){
        // cerr << "Pipe failed" << endl;
        // return 1; //returning 1 indicates an error
        perror("Pipe failed");
        exit(EXIT_FAILURE); 
    }

    // // Create child to run first command
    pid_t firstChildPid = fork(); //creates two processes
    if(firstChildPid < 0){
        // cerr << "Fork failed" << endl;
        // return 1; 
        perror("Fork failed");
        exit(EXIT_FAILURE); 
    }
    // In child, redirect output to write end of pipe
    if(firstChildPid > 0){ //forked child process sucessfully 
        dup2(fd[1], 1); 
    
        // Close the read end of the pipe on the child side.
        close(fd[0]); 
        // In child, execute the command
        execvp("ls", cmd1); 
    }
    // Create another child to run second command
    pid_t secondChildPid = fork(); 
    if(secondChildPid < 0){
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if(secondChildPid > 0){
        // In child, redirect input to the read end of the pipe
        dup2(fd[0], 0); 
        // Close the write end of the pipe on the child side.
        close(fd[1]); 
        // Execute the second command.
        execvp("tr", cmd2); 
    }
    // Reset the input and output file descriptors of the parent.
}
