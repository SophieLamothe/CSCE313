/*
	Author of the starter code
    Yifan Ren
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 9/15/2024
	
	Please include your Name, UIN, and the date below
	Name: Sophie Lamothe	
	UIN: 432009535
	Date: 09/29/2024
*/
#include "common.h"
#include "FIFORequestChannel.h" //for wait
#include <sys/wait.h>
#include <unistd.h> //for fork and execvp

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	string filename = "";

	//Add other arguments here
	while ((opt = getopt(argc, argv, "p:t:e:f:")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}

	//Task 1:
	//Run the server process as a child of the client process
	pid_t pid = fork(); 
	//child process: run the server
	if(pid == 0){
		//server has no arguments
		const char* args[] = {"./server", nullptr}; 
		//replace child process with server process
		execvp(args[0],  (char* const*)args); 
	}
	else if(pid > 0){
		//parent process: continue with client process 
		//read from and write to the server
    	FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
	

		//Task 4:
		//Request a new channel
		
		//Task 2:
		//Request data points
		char buf[MAX_MESSAGE];
		datamsg x(1, 0.0, 1);
		
		memcpy(buf, &x, sizeof(datamsg));
		chan.cwrite(buf, sizeof(datamsg));
		double reply;
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		
		//Task 3:
		//Request files
		filemsg fm(0, 0);
		string fname = "1.csv";
		
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);

		delete[] buf2;
		__int64_t file_length;
		chan.cread(&file_length, sizeof(__int64_t));
		cout << "The length of " << fname << " is " << file_length << endl;
		
		//Task 5:
		// Closing all the channels
		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));

		//wait for server process to terminate
		//store termination status of the child process (server)
		//status has info about whether server exited normally or due to a signal,
		//and its exit code
		int status;
		//use wait instead of waitpid b/c there is only one child process(the server)
		wait(&status);
	}
	else{
		//fork failed
		//terminate the program immediately, 1 is an exit status code that OS
		//can check to understand how the program ended
		exit(1); 
	}
}
