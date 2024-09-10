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
    //pipefd[0] is for reading and pipefd[1] is for writing
    int fd[2]; 

    //check if pipe creation is successful
    if(pipe(fd) == -1){
        // cerr << "Pipe failed" << endl;
        // return 1; //returning 1 indicates an error
        perror("Pipe failed");
        exit(EXIT_FAILURE); 
    }

    // Create child to run first command
    pid_t firstChildPid = fork(); //creates two processes
    if(firstChildPid < 0){
        // cerr << "Fork failed" << endl;
        // return 1; 
        perror("Fork failed");
        exit(EXIT_FAILURE); 
    }
    // In child, redirect output to write end of pipe
    //forked child process sucessfully 
    if(firstChildPid == 0){ 
        //stdout is redirected to fd[1] which means any data sent to stdout will 
        //now go into the pipe instead of the terminal
        //enables redirection of output from ls to the pipe to be processed by the 
        //next command
        //you want to send data to fd[1] and 1 is the fd for standard output
        dup2(fd[1], 1); //STDOUT_FILENO = 1
    
        // Close the read end of the pipe on the child side.
        close(fd[0]); 
        //fd[1] is no longer needed after redirection
        //prevent accidental writes
        close(fd[1]); 
        // In child, execute the command
        execvp("ls", cmd1); // "ls" == cmd1[0]

        //error message if exexcvp fails
        // cerr << "Exec failed" << endl;
        // return 1; 
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
// return is 0 you are in child
// positive, you are sitll in the parent process