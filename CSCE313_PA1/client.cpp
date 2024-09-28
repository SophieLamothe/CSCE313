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
	bool use_new_channel = false; 

	//Add other arguments here
	while ((opt = getopt(argc, argv, "p:t:e:f:c")) != -1) {
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
			case 'c':
				//Used for Task 4 w/ cmd arg -c
				use_new_channel = true;
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
		FIFORequestChannel* new_channel = &chan; 
		if(use_new_channel){
			//send NEWCHANNEL_MSG to server
			MESSAGE_TYPE new_channel_msg = NEWCHANNEL_MSG;
			chan.cwrite(&new_channel_msg, sizeof(new_channel_msg));

			//receive name of new channel from server
			char new_channel_name[30];
			chan.cread(&new_channel_name, sizeof(new_channel_name)); 

			//create new FIFORequestChannel w/ received channel
			new_channel = new FIFORequestChannel(new_channel_name, FIFORequestChannel::CLIENT_SIDE);	
		}

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
		string fname = filename;
		
		int len = sizeof(filemsg) + (fname.size() + 1);
		char* buf2 = new char[len];
		//store file message in buf2
		memcpy(buf2, &fm, sizeof(filemsg));
		strcpy(buf2 + sizeof(filemsg), fname.c_str());
		chan.cwrite(buf2, len);

		
		__int64_t file_length;
		chan.cread(&file_length, sizeof(__int64_t));
		cout << "The length of " << fname << " is " << file_length << endl;

		string received_file = "received/" + fname;
		ofstream outFile(received_file, ios::binary); 
		if(!outFile.is_open()){
			//move delete operation here so that if the file cannot 
			//be opened it is deleted in here
			delete[] buf2;
			cerr << "Cannot open file" << endl; 
			exit(1); 
		}
		//send series of messages to get content of the file
		//calculate the number of requests to transfer the file
		__int64_t offset = 0; 
		const int chunk_len = MAX_MESSAGE;

		while(offset < file_length){
			int bytes_to_read = min(chunk_len, static_cast<int>(file_length - offset));
			//use current offset and chunk size for filemsg
			filemsg curr_fmsg(offset, bytes_to_read);
			memcpy(buf2, &curr_fmsg, sizeof(curr_fmsg)); 
			strcpy(buf2 + sizeof(filemsg), fname.c_str());

			//request the chunk
			chan.cwrite(buf2, len);
			char* file_data = new char[bytes_to_read];
			chan.cread(file_data, bytes_to_read); 
			//write to output file
			outFile.write(file_data, bytes_to_read); 
			//update offset
			offset += bytes_to_read;
			//deallocate memory of file data buffer after each use
			delete[] file_data; 
		}

		//close output file
		outFile.close();
		delete[] buf2; 
		// int num_requests = ceil(file_length/256);
		// for(int i = 0; i < num_requests; i++){
		// 	int curr_offset = i * 256;
		// 	int curr_len = 256; 
		// 	if(curr_offset + 256 > file_length){
		// 		int curr_len = file_length - curr_offset;  
		// 	}
		// 	filemsg curr_msg(curr_offset, curr_len);
		// 	chan.cwrite(&curr_msg, sizeof(curr_msg)); 
		// 	//store file message in buf2
		// memcpy(buf2, &fm, sizeof(filemsg));
		// strcpy(buf2 + sizeof(filemsg), fname.c_str());
		// 	char request; 
		// 	chan.cread()
		// }
		
		//Task 5:
		// Closing all the channels
		MESSAGE_TYPE m = QUIT_MSG;
		//always send quit_msg to control channel
		chan.cwrite(&m, sizeof(MESSAGE_TYPE));

		new_channel->cwrite(&m, sizeof(MESSAGE_TYPE));
		if(use_new_channel){
			delete new_channel; 
		}

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
