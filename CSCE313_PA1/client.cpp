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
#include "FIFORequestChannel.h" 
#include <sys/wait.h> //for wait
#include <unistd.h> //for fork and execvp

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = 1;
	double t = 0.00;
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
		//construct data message with the right parameters
		datamsg x(p, t, e);
		
		//copies data massage x to the buffer
		memcpy(buf, &x, sizeof(datamsg));
		//write request is sent to the server
		chan.cwrite(buf, sizeof(datamsg));
		double reply;
		//read response is stored in reply
		chan.cread(&reply, sizeof(double));
		cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		
		//Request 1000 data points for both ecg for given patient
		ofstream outputFile("x1.csv");
		if(!outputFile.is_open()){
			cerr << "Error opening file" << endl; 
			exit(1); //error with opening file
		}

		double time_interval = 0.004; //Calculated interval for each ecg data point (4ms)
		double time = 0.0; 

		for(int i = 0; i < 1000; i++){
			//request ecg1
			datamsg message1(p, time, 1);
			chan.cwrite(&message1, sizeof(datamsg));
			double ecg1;
			chan.cread(&ecg1, sizeof(double)); 
			//request ecg2
			datamsg message2(p,time,2);
			chan.cwrite(&message2, sizeof(message2));
			double ecg2; 
			chan.cread(&ecg2, sizeof(double)); 

			//write into data to the file
			outputFile << time << "," << ecg1 << "," << ecg2 << endl; 
			//increment to next time point
			time += time_interval; 
		}
		//close file
		outputFile.close();
		
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
