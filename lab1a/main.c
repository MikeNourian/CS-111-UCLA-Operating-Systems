//NAME: Milad Nourian
//EMAIL: miladnourian@ucla.edu
//ID: 004854226
//
//  main.c
//  lab1A
//
//  Created by Mike Nourian on 1/16/18.
//  Copyright © 2018 Mike Nourian. All rights reserved.
//
//int tcsetattr(int fildes, int optional_actions, const struct termios *termios_p);
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <signal.h> //for kill()
#include <sys/types.h> //for kill ()
#include <sys/types.h> //for waitpid
#include <sys/wait.h> //for waitpid
#define true 1
#define false 0

int stringContainsEscapeD(char * buff, long size){
    //1 is true (contains), 0 is false
    char * ptr = buff;
    long i =0;
    for (i = 0; i < size; i++){
        if (*(ptr + i) == 4){ //if the character string contains a ^D, return true
            return true;
        }
    }
    return false;
}
int stringContainsEscapeC(char * buff, long size){
    //1 is true (contains), 0 is false
    char * ptr = buff;
    long i =0;
    for (i = 0; i < size; i++){
        if (*(ptr + i) == 3){ //if the character string contains a ^C, return true
            return true;
        }
    }
    return false;
}
void checkReadCount(long readcount){
    if (readcount<0){
        fprintf(stderr, "Error reading the STDIN file, REASON: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


void signalHandlerForSIGPIPE (int sigNum){
    //this means that we are writing to something pipe that is closed already
    
    //what should I do
  fprintf(stderr, "The signal handler is called but not doing anything, exiting with signal %d\n", sigNum);
  //  fprintf(stderr,"SHELL EXIT SIGNAL=9 STATUS=9");
  exit(EXIT_SUCCESS);
    
    
}

void handleUnrecognizedArguments (){
    fprintf(stderr, "Unrecognized argument was provided to the program, terminating with signal 1\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv) {
    static struct option long_options[] = {
        {"shell",  no_argument, 0, 's'},
        {0,         0,                 0,  0 }
    };
    int shellFlag =0;
    int c;
    signal(SIGPIPE, signalHandlerForSIGPIPE);
    
    
    
    while (1) {
        c = getopt_long(argc, argv, "", long_options, 0);
        if (c == -1){
            //if no input is given
            break;
        }
        switch (c) {
            case 's'://set the shellflag
                shellFlag = 1;
                break;
            case '?':
            default:
                //this means that the user input unrecognized argument to the program
                handleUnrecognizedArguments();
                fprintf(stderr, "This should not print after the unrecognized input from the user\n");
                break;
        }
    }
    
    
    //if shellFlag is set then we have to do the operations
    
    
    
    //at this point, create 2 pipes since we have to pass data in 2 directions
    int pipefdFromTerminalToShell[2];
    int pipefdFromShellToTerminal[2];
    if (pipe(pipefdFromTerminalToShell) == -1 ){
        fprintf(stderr, "Unsuccessful in creating the pipe pipefdFromTerminalToShell\n");
        exit(EXIT_FAILURE);
    }
    
    if (pipe(pipefdFromShellToTerminal) == -1 )
    {
        fprintf(stderr, "Unsuccessful in creating the pipe pipefdFromShellToTerminal\n");
        exit(EXIT_FAILURE);
    }
    //for both with and without --shell option, we need to setup the stdin
    struct termios oldtio, newtio;
    char keyboardBuff[255]; //this is the buffer to read the input to the keyboard to STDIN
    //right away get the current state of the terminal (stdin, or the keyboard as specified in the specs)
    tcgetattr(STDIN_FILENO, &oldtio); //to get the old attribute
    newtio = oldtio;
    newtio.c_iflag = ISTRIP; //strip of the 8th bit
    //newtio.c_iflag = ISTRIP; This is what I was using originally, just to translate NL to <CR><NL>
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; //this is for non-canonical -- character by character
    
    newtio.c_cc[VTIME]    = 0;   //do the reading of the input character to the terminal by the keyboard immediately
    newtio.c_cc[VMIN]     = 1/*0*/; //to wait for one char   //get the characters types in as soon as they appear
    
    //use the setattribute with this TCSANOW so that it applies immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &newtio);
    
    //insert here
    
    //forking goes here
    if (shellFlag){
        int childPID = fork();
        if (childPID == 0){ //child process
            //       int execve(const char *filename, char *const argv[],
            //char *const envp[]);
            //argv[0] must be the name of the program that we want to execute
            //The argv and envp arrays must each include a null pointer at the end of the array.
            
            //now have to set up the file descriptor and the stdin and stdout of the file system
            /*
             whose standard input is a pipe from the terminal process, and whose standard output and standard error are (dups of) a pipe to the terminal process.
             */
            //for input
            close(pipefdFromTerminalToShell[1]);//we are not writing to this pipe from the shell
            close(STDIN_FILENO);//now we have closed the stdin
            if (dup2(pipefdFromTerminalToShell[0], STDIN_FILENO) == -1){
                fprintf(stderr, "There was an error with dup2-STDIN in pipefdFromTerminalToShell\n");
                exit(EXIT_FAILURE);
            }
            //end of stdin
            close(pipefdFromShellToTerminal[0]); //close since we are not reading from this pipe, we are just writing to it
            close(STDOUT_FILENO);
            if (dup2(pipefdFromShellToTerminal[1], STDOUT_FILENO) == -1){
                fprintf(stderr, "There was an error with dup2-STDOUT in pipefdFromTerminalToShell\n");
                exit(EXIT_FAILURE);
            }
            close(STDERR_FILENO);
            if (dup2(pipefdFromShellToTerminal[1], STDERR_FILENO)== -1){
                fprintf(stderr, "There was an error with dup2- STDERR in pipefdFromTerminalToShell\n");
                exit(EXIT_FAILURE);
            }
            char * fileName = "/bin/bash";
            char * args[]={fileName, NULL};
            char * env[] = {NULL};
            execve(fileName, args, env);
            fprintf(stderr, "Exceve was not successful, exiting the program\n");
            exit(EXIT_FAILURE);
        }
        
        else{//this is the parent
            //childPID is available
            //the parent process is the terminal process
            //what it does is it takes the keyboard entries and give it through a pipe to the shell process that is running so that the shell runs the commands
            //settup the file descriptors
            //close the reading from the terminaltoshell pipe
            
            struct pollfd pollfds[2];
            pollfds[0].fd= STDIN_FILENO;
            pollfds[0].events = POLLIN | POLLHUP | POLLERR; //waiting for this event to occur
            
            pollfds[1].fd = pipefdFromShellToTerminal[0];
            pollfds[1].events = POLLIN | POLLHUP | POLLERR;
            
            
            if (close(pipefdFromTerminalToShell[0]) == -1){
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (close(pipefdFromShellToTerminal[1]) == -1){ //close the writing from Shelltotermial pipe
                fprintf(stderr, "ERROR: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            char modifiedBuffer [510];
            int modifiedIndex = 0;
            long readcount = 0;
            int i = 0;
            
            char shellBuffer[510]; //this is the buffer we used when passing values to the shell
            int shellIndex = 0;
            while (1){
                //write(STDOUT_FILENO, keyboardBuff, readcount); //write all the bytes that were count to the display
                //write(STDOUT_FILENO, modifiedBuffer, modifiedIndex);
                //(1) now we have to pipe this string to the shell process
                //get shellBuffer
                //poll to wait for the input from the user
                int polled = poll(pollfds, 2, 0); //time out is zero
                if (polled == -1){
                    fprintf(stderr, "There was a problem with polling, REASON: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                //now check what has happened on the polling
                if (pollfds[0].revents & POLLIN){//if the input from the STDIN is available
                    readcount = read(STDIN_FILENO, keyboardBuff, sizeof(keyboardBuff));
                    checkReadCount(readcount);
                    if (stringContainsEscapeD(keyboardBuff, readcount)){
                        close(pipefdFromTerminalToShell[1]); //closing the pipe from terminal to shell used for writing
                    }
                    if (stringContainsEscapeC(keyboardBuff, readcount)){
                        //send the kill signal
                        kill(childPID, SIGINT);//all done when the ^C is entered
                    }
                    //check for conversion of <cr> and <fl> to <cr><fl> when echoing to the display
                    modifiedIndex = 0;
                    for (i =0; i <  readcount; i++) {
		      if ((keyboardBuff[i] == '\n') | (keyboardBuff[i] == '\r')){
                            modifiedBuffer[modifiedIndex] = '\r';
                            modifiedBuffer[modifiedIndex + 1] = '\n';
                            modifiedIndex = modifiedIndex + 2;
                        }
                        else{
                            modifiedBuffer[modifiedIndex] = keyboardBuff[i];
                            modifiedIndex = modifiedIndex + 1;
                        }
                    }
                    write(STDOUT_FILENO, modifiedBuffer, modifiedIndex);//first write the input given by the user to the display
                    //now give that input to the shell process
                    shellIndex = 0;
                    int j;
                    for (j =0; j < modifiedIndex; j++){
                        if ((j != 509) && (modifiedBuffer[j] == '\r' && modifiedBuffer[j+1] == '\n'))//509 is the last index of modifiedBuffer, I am checking that we dont go out of bounds
                        {
                            shellBuffer[shellIndex] = '\n';
                            shellIndex++;
                        }
                        else{
                            shellBuffer[shellIndex] = modifiedBuffer[j];
                            shellIndex++;
                        }
                    }
                    //now write that command given from the user to the shell
                    write(pipefdFromTerminalToShell[1], shellBuffer, shellIndex); //read the content that the user has entered via keyboard to the shell process
                }
                if (pollfds[1].revents & POLLIN){
                    //read from the pipe that comes from the shell pipe
                    readcount = read(pipefdFromShellToTerminal[0], keyboardBuff, sizeof(keyboardBuff)); //read into the keyboardBuff
                    checkReadCount(readcount);
                    modifiedIndex = 0;
                    //turn <\n> from the shell to <\r><\n> to display on the screen
                    for (i =0; i <  readcount; i++) {
                        if (keyboardBuff[i] == '\n'){
                            modifiedBuffer[modifiedIndex] = '\r';
                            modifiedBuffer[modifiedIndex + 1] = '\n';
                            modifiedIndex = modifiedIndex + 2;
                        }
                        else{
                            modifiedBuffer[modifiedIndex] = keyboardBuff[i];
                            modifiedIndex = modifiedIndex + 1;
                        }
                    }
                    if (readcount == 0){ //it means the pipe from shell to the terminal is at EOF so the
                        //now you have to waitpid to get the shell's exit status
                        pid_t waitReturn;
                        int status;
                        waitReturn = waitpid(childPID, &status, WUNTRACED | WCONTINUED);
                        if (waitReturn == -1){
                            fprintf(stderr, "The return process failed, exiting\n");
                            exit(EXIT_FAILURE);
                        }
                        if (WIFEXITED(status)){
                            int exitstatus = WEXITSTATUS(status);
                            //int signal = exitstatus & 0x007f;
                            //stat = exitstatus & 0xff00;
			    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",exitstatus, exitstatus);   
			      //fprintf (stderr, "The EXIT IS FOUND IN pollfds[1].revents & POLLIN");
			    //fprintf(stderr, "SHELL EXIT SIGNAL=9 STATUS=9");
                            //fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",signal,stat);
                            //and the we can exit successfully
                            //******************
                            //close the file fromShellToTerminal if not closed yet
                            //******************
			    //restore the terminal mode
			        //last thing before the shut down restore the STDIN to oldtio so it goes back to the previous mode

			    tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                            exit(EXIT_SUCCESS);
                        }
                        else if(WIFSIGNALED(status)){
                            fprintf(stderr, "Program exited with signal, %d\n",WTERMSIG(status));
			    tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
			    exit(EXIT_FAILURE);
                        }
                        fprintf(stderr, "The status of the shell process could not be found from exit status or signal, exiting failure\n");
			tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                        exit(EXIT_FAILURE);
                    }
                    write(STDOUT_FILENO, modifiedBuffer, modifiedIndex); //write to the screen
                }
                
		//                if (pollfds[1].revents & POLLHUP){
		               // if (pollfds[1].revents & POLLHUP){
                if ((pollfds[1].revents & POLLHUP) || (pollfds[1].revents & POLLERR)){ //change this to see if the exit status is for the POLLERR
                    //check that there is no more input in the pipe from the shell to the terminal
                    //CHECK THE REMAINING OUTPUT
                    readcount = read(pipefdFromShellToTerminal[0], keyboardBuff, sizeof(keyboardBuff)); //read into the keyboardBuff
                    checkReadCount(readcount);
                    modifiedIndex = 0;
                    //turn <\n> from the shell to <\r><\n> to display on the screen
                    for (i =0; i <  readcount; i++) {
                        if (keyboardBuff[i] == '\n'){
                            modifiedBuffer[modifiedIndex] = '\r';
                            modifiedBuffer[modifiedIndex + 1] = '\n';
                            modifiedIndex = modifiedIndex + 2;
                        }
                        else{
                            modifiedBuffer[modifiedIndex] = keyboardBuff[i];
                            modifiedIndex = modifiedIndex + 1;
                        }
                    }
                    write(STDOUT_FILENO, modifiedBuffer, modifiedIndex); //write to the screen
                    ///END OF CHECKING THE FINAL REMAINING OUTPUT
                    //now you have to waitpid to get the shell's exit status
                    pid_t waitReturn;
                    int status;
                    waitReturn = waitpid(childPID, &status,0);
                    if (waitReturn == -1){
                        fprintf(stderr, "The return process failed, exiting\n");
                        exit(EXIT_FAILURE);
                    }
                    if (WIFEXITED(status)){
                        int exitstatus = WEXITSTATUS(status);
                        //int signal = exitstatus & 0x007f;
                        //int stat = exitstatus & 0xff00;
			fprintf (stderr, "Shell is exiting because of the POLLHUP");
			//Change this line back again but check for the exit status now
			//fprintf(stderr, "SHELL EXIT SIGNAL=9 STATUS=9");
			fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n",exitstatus, exitstatus);
			//fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d",signal,stat);
                        //and the we can exit successfully
                        //******************
                        //close the file fromShellToTerminal if not closed yet
                        //******************
                        tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                        exit(EXIT_SUCCESS);
                    }
                    else if(WIFSIGNALED(status)){
                        fprintf(stderr, "Program exited with signal, %d\n",WTERMSIG(status));
			tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr, "The status of the shell process could not be found from exit status or signal, exiting failure\n");
		    tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                    exit(EXIT_FAILURE);
                }
                
            }
        }
    }
    
    
    //THIS IS NOT IN THE SHELLFLAG CODE
    else{
    
    long readcount = read(STDIN_FILENO, keyboardBuff, sizeof(keyboardBuff));
    checkReadCount(readcount);
    //this is to find out if the string contains any escape characters
    if (stringContainsEscapeD(keyboardBuff, readcount)){
        tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
        fprintf(stderr, "The user entered ^D and exit the program\n");
        exit(EXIT_SUCCESS);
    }
    char modifiedBuffer [510];
    int modifiedIndex = 0;
    //    char shellBuffer[510]; //this is the buffer we used when passing values to the shell
    //int shellIndex = 0;
    //pass in modifiedBuffer to the write, modify <cr> or <lf> to <cr><lf>
    int i = 0;
    for (i =0; i <  readcount; i++) {
      if ((keyboardBuff[i] == '\n') | (keyboardBuff[i] == '\r')){
            modifiedBuffer[modifiedIndex] = '\r';
            modifiedBuffer[modifiedIndex + 1] = '\n';
            modifiedIndex = modifiedIndex + 2;
        }
        else{
            modifiedBuffer[modifiedIndex] = keyboardBuff[i];
            modifiedIndex = modifiedIndex + 1;
        }
    }
    while (readcount != 0){
        //write(STDOUT_FILENO, keyboardBuff, readcount); //write all the bytes that were count to the display
        write(STDOUT_FILENO, modifiedBuffer, modifiedIndex);
        
        //The reason for the commented out section is that I dont need real from the shell when --shell is not specified
	/*
        shellIndex = 0;
        int j;
        for (j =0; j < modifiedIndex; j++){
            if ((j != 509) && (modifiedBuffer[j] == '\r' && modifiedBuffer[j+1] == '\n'))//509 is the last index of modifiedBuffer, I am checking that we dont go out of bounds
            {
                shellBuffer[shellIndex] = '\n';
                shellIndex++;
            }
            else{
                shellBuffer[shellIndex] = modifiedBuffer[j];
                shellIndex++;
            }
        }
        
        //now we are reading to pipe this input to the shell
        write(pipefdFromTerminalToShell[1], shellBuffer, shellIndex); //read the content that the user has entered via keyboard to the shell process
        
        */
        //pass this input through the pipe to the shell process
        
        readcount = read(STDIN_FILENO, keyboardBuff, sizeof(keyboardBuff));
        checkReadCount(readcount);
        if (stringContainsEscapeD(keyboardBuff, readcount)){
            tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
            fprintf(stderr, "The user entered ^D and exit the program\n");
            exit(EXIT_SUCCESS);
        }
        //check for conversion of <cr> and <fl> to <cr><fl>
        modifiedIndex = 0;
        for (i =0; i <  readcount; i++) {
	  if ((keyboardBuff[i] == '\n') | (keyboardBuff[i] == '\r')){
                modifiedBuffer[modifiedIndex] = '\r';
                modifiedBuffer[modifiedIndex + 1] = '\n';
                modifiedIndex = modifiedIndex + 2;
            }
            else{
                modifiedBuffer[modifiedIndex] = keyboardBuff[i];
                modifiedIndex++;
            }
        }
    }
    fprintf(stderr,"We got to the end\n");
    
    //the spec says to read input from the STDIN
    
    
    
    
    
    
    
    //last thing before the shut down restore the STDIN to oldtio so it goes back to the previous mode
    tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
    exit (0);
    }
}

