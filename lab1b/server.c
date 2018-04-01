//NAME: Milad Nourian
//EMAIL: miladnourian@ucla.edu
//ID: 004854226
#include <stdio.h>
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
#include <sys/types.h>
#include <sys/socket.h> //for supporting the socket API
#include <netinet/in.h> //for constants related to network
#include <stdlib.h> 
#include <sys/stat.h> //for using open ()
#include <fcntl.h> //for using open

#include <assert.h>
#include "zlib.h" //for compression

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

#define true 1
#define false 0


int defBuffer (void *src, int srcLen, void *dst, int dstLen, int level){
    //strm.next_in = input buffer
    //strm.avail_int = read(); //how much reas returns
    
    
    //intialize the strm.next_out = dst;
    //strm.avail_out= CHUNK; //size of the chunk
    z_stream strm ;
    strm.total_in  = strm.avail_in  = srcLen;
    strm.total_out = strm.avail_out = dstLen;
    strm.next_in   = (Bytef *) src;
    strm.next_out  = (Bytef *) dst;
    
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    int err = -1;
    int ret = -1;
    
    err = deflateInit(&strm, level);//initalize the stream with the level specified by the usr
    if (err == Z_OK){ //if initializing was fine, then actually perform
        err = deflate(&strm, Z_FINISH); //finish because we want to do it in one shot
        if (err == Z_STREAM_END){
            ret = (int)strm.total_out;
        }
        else{
            deflateEnd(&strm);
            return err;//when faced an error
        }
    }
    else{
        deflateEnd(&strm);
        return err;
    }
    
    deflateEnd(&strm);
    return ret;
}

int infBuffer (void *src, int srcLen, void *dst, int dstLen){
    z_stream strm ;
    strm.total_in  = strm.avail_in  = srcLen;
    strm.total_out = strm.avail_out = dstLen;
    strm.next_in   = (Bytef *) src;
    strm.next_out  = (Bytef *) dst;
    
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    int err = -1;
    int ret = -1;
    
    err = inflateInit(&strm);
    if (err == Z_OK){
        err = inflate(&strm, Z_FINISH); //to finish inflating right away
        if (err == Z_STREAM_END){ //if got to the end of the Z_STREAM_END
            ret = (int)strm.total_out; //total number of output bytes
        }
        else{
            inflateEnd(&strm);
            return err;
        }
    }
    else{
        inflateEnd(&strm);
        return err;
    }
    inflateEnd(&strm);
    return ret;
}



//ZLIB FUNCIONS
/* Compress from file source to file dest until EOF on source.
 def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 allocated for processing, Z_STREAM_ERROR if an invalid compression
 level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
 version of the library linked do not match, or Z_ERRNO if there is
 an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    
    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, level);
    if (ret != Z_OK)
        return ret;
    
    /* compress until end of file */
    do {
        strm.avail_in = (uint)fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return Z_ERRNO;
        }
        //original
        //flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        flush = feof(source) ? Z_FINISH : Z_SYNC_FLUSH;
        strm.next_in = in;
        
        /* run deflate() on input until output buffer not full, finish
         compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);     /* all input will be used */
        
        /* done when last data in file processed */
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);        /* stream will be complete */
    
    /* clean up and return */
    (void)deflateEnd(&strm);
    return Z_OK;
}

/* Decompress from file source to file dest until stream ends or EOF.
 inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 allocated for processing, Z_DATA_ERROR if the deflate data is
 invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
 the version of the library linked do not match, or Z_ERRNO if there
 is an error reading or writing the files. */
int inf(FILE *source, FILE *dest)
{
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];
    
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;
    
    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = (uint)fread(in, 1, CHUNK, source); //I ADDED THE uint to silent the error, Might need to change it
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;
        
        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                    (void)inflateEnd(&strm);
                    return ret;
                    break;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return ret;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
        } while (strm.avail_out == 0);
        
        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);
    
    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
        case Z_ERRNO:
            if (ferror(stdin))
                fputs("error reading stdin\n", stderr);
            if (ferror(stdout))
                fputs("error writing stdout\n", stderr);
            break;
        case Z_STREAM_ERROR:
            fputs("invalid compression level\n", stderr);
            break;
        case Z_DATA_ERROR:
            fputs("invalid or incomplete deflate data\n", stderr);
            break;
        case Z_MEM_ERROR:
            fputs("out of memory\n", stderr);
            break;
        case Z_VERSION_ERROR:
            fputs("zlib version mismatch!\n", stderr);
    }
}

//END OF ZLIB


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
    //fprintf(stderr, "But we are not exiting\n")
    exit(EXIT_SUCCESS);
    
    
}


void handleUnrecognizedArguments (){
    fprintf(stderr, "Unrecognized argument was provided to the program, terminating with signal 1\n");
    exit(EXIT_FAILURE);
}

//THIS IS THE SERVER CODE
//THE server gets the input from the socket from the terminal process and then uses pipes to write and read from the shell process that it creates
int main(int argc, char ** argv) {
    // insert code here...
    //at this point, create 2 pipes since we have to pass data in 2 directions
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"compress", no_argument, 0, 'c'},
        {0,         0,                 0,  0 }
    };
    int portNumber =0;
    int portFlag = 0;
    int c;
    
    int compressFlag =0;
    
    signal(SIGPIPE, signalHandlerForSIGPIPE);
    
    
    while (1) {
        c = getopt_long(argc, argv, "", long_options, 0);
        if (c == -1){
            //if no input is given
            break;
        }
        switch (c) {
            case 'p':
                portFlag = 1;
                portNumber = atoi(optarg); //to get the port number
                break;
            case 'c':
                compressFlag = 1;
                break;
            case '?':
            default:
                //this means that the user input unrecognized argument to the program
                handleUnrecognizedArguments();
                fprintf(stderr, "This should not print after the unrecognized input from the user\n");
                break;
        }
    }
    
    if (portFlag ==0){
        fprintf(stderr, "The user did not provide a required port number\n");
        exit(EXIT_FAILURE);
    }
    
    //fprintf(stderr, "It gets to before socket settup\n");
    int socketfd, newsocketfd;
    struct sockaddr_in server_address, client_address; //this is for binding (giving the server address) and accepting the connection to the user
    socklen_t clientLength;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd< 0){
        //restore the terminal
        fprintf(stderr, "There was an error with socket in server code\n");
        exit(EXIT_FAILURE);
    }
    bzero((char*)&server_address, sizeof(server_address));//zero it out
    //now set the address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(portNumber);
    server_address.sin_addr.s_addr = INADDR_ANY; //this is to set the IP address of the server
    if (bind(socketfd, (struct sockaddr *) &server_address, sizeof(server_address))< 0) //bind gives the socket that we have created, its address
    {
        fprintf(stderr, "There was an error with binding of the server, exiting\n");
        exit(EXIT_FAILURE);
    }
    listen(socketfd, 5);//listen on a connection
    clientLength = sizeof(client_address);
    newsocketfd = accept(socketfd, (struct sockaddr*) &client_address, &clientLength);
    if (newsocketfd < 0)
    {
        fprintf(stderr, "There was an error getting the new socketfd\n");
        exit(EXIT_FAILURE);
    }
    
    
    fprintf(stderr, "We have accepted the connection\n");
    //so once a connection is made, now fork() and create the shell process
    
    //the servers STDIN should be the socketfd, since we wont be working with the server directly
    close(STDIN_FILENO);
    
    //if (dup2(socketfd, STDIN_FILENO) == -1){
    if (dup2(newsocketfd, STDIN_FILENO) == -1){
        fprintf(stderr, "There was a problem with dup2 on the server\n");
        exit(EXIT_FAILURE);
    }
    //fprintf(stderr, "we just closed the STDIN_FILENO and replaced by socketfd\n");
    
    int pipefdFromServerToShell[2]; //change the name
    int pipefdFromShellToServer[2];
    if (pipe(pipefdFromServerToShell) == -1 ){
        fprintf(stderr, "Unsuccessful in creating the pipe pipefdFromTerminalToShell\n");
        exit(EXIT_FAILURE);
    }
    
    if (pipe(pipefdFromShellToServer) == -1 )
    {
        fprintf(stderr, "Unsuccessful in creating the pipe pipefdFromShellToTerminal\n");
        exit(EXIT_FAILURE);
    }
    
    //fprintf(stderr,"Finish setting up the pipes in the server, about to fork\n");
    
    int childPID = fork();
    if (childPID == 0){ //child process
        //for input
        close(pipefdFromServerToShell[1]);//we are not writing to this pipe from the shell
        close(STDIN_FILENO);//now we have closed the stdin
        if (dup2(pipefdFromServerToShell[0], STDIN_FILENO) == -1){
            fprintf(stderr, "There was an error with dup2-STDIN in pipefdFromTerminalToShell\n");
            exit(EXIT_FAILURE);
        }
        //end of stdin
        close(pipefdFromShellToServer[0]); //close since we are not reading from this pipe, we are just writing to it
        close(STDOUT_FILENO);
        if (dup2(pipefdFromShellToServer[1], STDOUT_FILENO) == -1){
            fprintf(stderr, "There was an error with dup2-STDOUT in pipefdFromTerminalToShell\n");
            exit(EXIT_FAILURE);
        }
        close(STDERR_FILENO);
        if (dup2(pipefdFromShellToServer[1], STDERR_FILENO)== -1){
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
        
        //fprintf(stderr,"In the parent process\n");
        
        struct pollfd pollfds[2];
        pollfds[0].fd= newsocketfd; //poll on the input from socket
        pollfds[0].events = POLLIN | POLLHUP | POLLERR; //waiting for this event to occur
        
        pollfds[1].fd = pipefdFromShellToServer[0]; //wait on the input from the shell
        pollfds[1].events = POLLIN | POLLHUP | POLLERR;
        
        
        if (close(pipefdFromServerToShell[0]) == -1){
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (close(pipefdFromShellToServer[1]) == -1){ //close the writing from Shelltotermial pipe
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        //fprintf (stderr,"After setting up the pipes in the parent\n");
        
        char socketBuffer [255]; //replace keyboard buffer with socketBuffer
        
        
        //char modifiedBuffer [510];
        //int modifiedIndex = 0; //was not used in the code
        long readcount = 0;
        
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
            
            //        fprintf(stderr,"In the while and after the poll()");
            
            //now check what has happened on the polling
            if (pollfds[0].revents & POLLIN){//if the input from the socket is available
                //fprintf (stderr,"we are on pollfd[0] & POLLIN and received input\n");
                
                fprintf(stderr, "There was POLLIN from the socket\n");
                
                //set my socket buffer to zeroa
                //char someZeroBuf [255] = {0};
                //sizeof arr or 20 * sizeof (int)
                //memset(arr, 0, sizeof arr);
                
                
                memset(socketBuffer, 0, 255 * sizeof(char)); //use this
                readcount = read(newsocketfd, socketBuffer, sizeof(socketBuffer));
                checkReadCount(readcount);
                
                //Data from the socket is available
//                fprintf(stderr, "Data from the socket is available for the server  pollfds[0].revents & POLLIN\n");
//                fprintf(stderr, "read data is: %s\n", socketBuffer);
                
                
                //now the input from the socket is available, decompress it
                
                if (compressFlag){
                    //now for decompressing, just use a massive buffer
                    char dest[CHUNK];
                    //ORIGINAL MIGHT NEED TO GO BACK
                    //int bytes = infBuffer(socketBuffer, 255, dest, CHUNK); //size of the buffers
                    int bytes = infBuffer(socketBuffer, (int)readcount, dest, CHUNK);
                    //TESTED: the dest actually has what we want, but bytes is 38 == incorrect
                    
                    if (bytes < 0){
                        fprintf(stderr, "There was problem with decompressing, but continuing\n");
                    }
                    
                    //use dest with bytes
                    
                    //ORIGINAL CODE
//                    if (stringContainsEscapeD(dest, bytes)){ //bytes in 255 and might contain some characters
//                        close(pipefdFromServerToShell[1]); //closing the pipe from terminal to shell used for writing
//                    }
//                    if (stringContainsEscapeC(dest, bytes)){
//                        //send the kill signal
//                        kill(childPID, SIGINT);//all done when the ^C is entered
//                    }
                    if (stringContainsEscapeD(dest, 1)){ //bytes in 255 and might contain some characters
                        close(pipefdFromServerToShell[1]); //closing the pipe from terminal to shell used for writing
                    }
                    if (stringContainsEscapeC(dest, 1)){
                        //send the kill signal
                        kill(childPID, SIGINT);//all done when the ^C is entered
                    }
                    
                    //READ THIS FORSURE:
                    //when we call inflate (to decompress), even when it returns a negative number, it still returns the correct character
                    if (dest[0] == '\r'){
                        dest[0] = '\n';
                    }
                    
                    
                    //Input received from the shell pipes (which receive both stdout and stderr from the shell) should be forwarded out to the network socket.
                    //now give that input to the shell process
                    //if EOF from the socket
                    if (bytes == 0){ //reach EOF
                        fprintf(stderr, "We read 0 bytes from the socket, EOF and closing the pipe\n");
                        close(pipefdFromServerToShell[1]); //close writing
                    }
                    
                    //use socketBuffer
                    //make the translation needed for the shell
                    //ORIGINAL
                    /*
                    shellIndex = 0;
                    int j;
                    for (j =0; j < bytes; j++){
                        if ((dest[j] == '\r' && dest[j+1] == '\n') || (dest[j] == '\r'))//509 is the last index of modifiedBuffer, I am checking that we dont go out of bounds
                        {
                            shellBuffer[shellIndex] = '\n';
                            shellIndex++;
                        }
                        else{
                            shellBuffer[shellIndex] = dest[j];
                            shellIndex++;
                        }
                    }
                    */
                    //END ORIGINAL
                    
                    //2 changes in the codse
                    //pass size 1 for the shell index
                    //translation to \n
                    //ORIGINAL CODE ==> MIGHT HAVE TO GO BACK
                    //write(pipefdFromServerToShell[1], shellBuffer, shellIndex);
                    //BEGIN ORIGINAL
                    //write(pipefdFromServerToShell[1], shellBuffer, 1);
                    //END ORIGINAL
                    write(pipefdFromServerToShell[1], dest, 1);

                    
                    //the problem is sending \n
                    
                    
                    
                    //now write that command given from the user to the shell
                    //now we are writing reading
                    

                    
                    //read the content that the user has entered via keyboard to the shell process
                    //write(pipefdFromServerToShell[1], shellBuffer, 1); //the reason I am writing 1 byte to the shell is becuase shellIndex =255, bytes =255
                    //did not solve the issue
                    
                    //now write that command given from the user to the shell
                    //write(pipefdFromServerToShell[1], shellBuffer, shellIndex); //read the content that the user has entered via keyboard to the shell process
                    
                    
                    
                    //START COMMENT
//                    int ret;
//                    //logFileDescriptor = open(logFileName,O_RDWR | O_APPEND | O_CREAT ,0666);
//                    int tempCompressedfd = open("temp.txt", O_RDWR | O_APPEND | O_CREAT ,0666);
//                    if (write(tempCompressedfd, socketBuffer, readcount) < 0){
//                        fprintf(stderr, "There was a problem opening/creating the tempfile on the clien\n");
//                        exit(EXIT_FAILURE);
//                    }//CHECK IF a+ IS WORKING PROPERLY
//
//
//                    fprintf(stderr, "Succesfully wrote wrote to tempCompressedfd, got FIlE* for compressedPtr\n");
//                    //temp has the compressed data
//
//                    FILE * compressedPtr = fdopen(tempCompressedfd, "a+");
//                    if (compressedPtr == NULL){
//                        fprintf(stderr, "Error getting compressedPtr in the clinet\n");
//                        exit(EXIT_FAILURE);
//                    }
//                    int tempDecompressedfd = open("temp2.txt", O_RDWR | O_APPEND | O_CREAT ,0666);
//                    FILE * decompressedPtr =fdopen(tempDecompressedfd, "a+");
//                    if (decompressedPtr == NULL){
//                        fprintf(stderr, "Error getting decompressedptr in the clinet\n");
//                        exit(EXIT_FAILURE);
//                    }
//                    SET_BINARY_MODE(compressedPtr);
//                    SET_BINARY_MODE(decompressedPtr);
//                    lseek(tempCompressedfd, 0, SEEK_SET);
//                    lseek(tempDecompressedfd, 0, SEEK_SET);
//                    fprintf(stderr, "Got to before decompressing");
//
//                    //now write to the decompressed
//                    ret = inf(compressedPtr, decompressedPtr); //this is to decompress
//                    if (ret != Z_OK){
//                        zerr(ret);
//                        fprintf(stderr, "There was a problem writing the decompressed data to the stdout\n");
//                    }
//
//
//                    lseek(tempDecompressedfd, 0, SEEK_SET);
//                    //read the data from decompressed into a buff
//                    char decompressedBuffer [255];
//                    long byteCount = read(tempDecompressedfd, decompressedBuffer, sizeof(decompressedBuffer));
//                    if (byteCount < 0){
//                        fprintf(stderr, "There was an error reading the bytes into the decompressed buff\n");
//                        exit(EXIT_FAILURE);
//                    }
//
//                    fprintf(stderr, "Read the data that is supposed to decompressed in the decompressedBuf, DATA: %s \n",decompressedBuffer);
//
//                    //decompressed has the string now, we can check for all that we need
//                    if (stringContainsEscapeD(decompressedBuffer, byteCount)){
//                        close(pipefdFromServerToShell[1]); //closing the pipe from terminal to shell used for writing
//                    }
//                    if (stringContainsEscapeC(decompressedBuffer, byteCount)){
//                        //send the kill signal
//                        kill(childPID, SIGINT);//all done when the ^C is entered
//                    }
//                    //Input received from the shell pipes (which receive both stdout and stderr from the shell) should be forwarded out to the network socket.
//                    //now give that input to the shell process
//                    //if EOF from the socket
//                    if (byteCount == 0){ //reach EOF
//                        close(pipefdFromServerToShell[1]); //close writing
//                    }
//
//                    if (remove("temp.txt") < 0){
//                        fprintf(stderr, "There was an error deleting temp.txt in the server\n");
//                        exit(EXIT_FAILURE);
//                    }
//                    if (remove("temp2.txt") < 0){
//                        fprintf(stderr, "There was an error deleting temp2.txt in the server\n");
//                        exit(EXIT_FAILURE);
//                    }
//
//                    //use socketBuffer
//                    shellIndex = 0;
//                    int j;
//                    for (j =0; j < byteCount; j++){
//                        if ((decompressedBuffer[j] == '\r' && decompressedBuffer[j+1] == '\n'))//509 is the last index of modifiedBuffer, I am checking that we dont go out of bounds
//                        {
//                            shellBuffer[shellIndex] = '\n';
//                            shellIndex++;
//                        }
//                        else{
//                            shellBuffer[shellIndex] = decompressedBuffer[j];
//                            shellIndex++;
//                        }
//                    }
//                    //now write that command given from the user to the shell
//                    write(pipefdFromServerToShell[1], shellBuffer, shellIndex); //read the content that the user has entered via keyboard to the shell process
                    //END COMMENT
                }

                //END OF DECOMPRESS
                else{ //if no compress
                    if (stringContainsEscapeD(socketBuffer, readcount)){
                        close(pipefdFromServerToShell[1]); //closing the pipe from terminal to shell used for writing
                    }
                    if (stringContainsEscapeC(socketBuffer, readcount)){
                        //send the kill signal
                        kill(childPID, SIGINT);//all done when the ^C is entered
                    }
                    //Input received from the shell pipes (which receive both stdout and stderr from the shell) should be forwarded out to the network socket.
                    //now give that input to the shell process
                    //if EOF from the socket
                    if (readcount == 0){ //reach EOF
                        close(pipefdFromServerToShell[1]); //close writing
                    }
                    
                    
                    //use socketBuffer
                    shellIndex = 0;
                    int j;
                    for (j =0; j < readcount; j++){
                        if ((socketBuffer[j] == '\r' && socketBuffer[j+1] == '\n'))//509 is the last index of modifiedBuffer, I am checking that we dont go out of bounds
                        {
                            shellBuffer[shellIndex] = '\n';
                            shellIndex++;
                        }
                        else{
                            shellBuffer[shellIndex] = socketBuffer[j];
                            shellIndex++;
                        }
                    }
                    //now write that command given from the user to the shell
                    write(pipefdFromServerToShell[1], shellBuffer, shellIndex); //read the content that the user has entered via keyboard to the shell process
                }
            }
            if (pollfds[1].revents & POLLIN){
                //read from the pipe that comes from the shell pipe
                //        pollfds[1].fd = pipefdFromShellToServer[0]; //wait on the input from the shell
                
                //compress it and send it
                
                fprintf (stderr,"pollfd[1] and POLLIN\n");
                fprintf(stderr, "before read from pipefdFromShellToServer[0]\n");
                readcount = read(pipefdFromShellToServer[0], socketBuffer, sizeof(socketBuffer)); //read into the keyboardBuff
                fprintf(stderr, "after read from pipefdFromShellToServer[0]\n");
                checkReadCount(readcount);
                
                //If the server gets an EOF or SIGPIPE from the shell pipes (e.g., the shell exits), harvest the shell's completion status, log it to stderr (as you did in project 1A), and exit with a return code of 0.
                
                //I AM NOT HANDLING SIGPIPE
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
                        //3. theservercollectsandreportstheshell'sterminationstatus.
                        //4. theserverclosesthenetworksockettotheclient,andexits.
                        close(socketfd);
                        close (newsocketfd);
                        exit(0);
                    }
                    else if(WIFSIGNALED(status)){
                        fprintf(stderr, "Program exited with signal, %d\n",WTERMSIG(status));
                        close(socketfd);
                        close(newsocketfd);
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr, "The status of the shell process could not be found from exit status or signal, exiting failure\n");
                    close(socketfd);
                    close(newsocketfd);
                    exit(EXIT_FAILURE);
                }
                
                if (compressFlag){
                //just write to a temp file and
                    //newSOCKET
                    
                    //if the compressFlag is set, we must then compress before sending over the socket
                    
                    //the data is in socketBuffer and readcount
                    
                    
                    
                    unsigned char dest[CHUNK]; //massive output buffer
                    //the data we read is in keyboard buf
                    int bytes = defBuffer(socketBuffer, 255, dest, CHUNK, Z_DEFAULT_COMPRESSION);
                    if (bytes < 0){
                        fprintf(stderr, "ret in defBuffer returned -1, error\n");
                        exit(EXIT_FAILURE);
                    }
                    
                    
                    //now the dest Buff must have some compressed data that we can write out
                    write(newsocketfd, dest,bytes); //if the user did not specify the --compress option, just write the buffer to the socketfd
                    
//                    //sockeBuff came from the pipe, readcount
//                    int ret;
//                    //        logFileDescriptor = open(logFileName,O_RDWR | O_APPEND | O_CREAT ,0666);
//                    int tempDecompressedfd = open("temp.txt", O_RDWR | O_APPEND | O_CREAT ,0666);
//                    if (write(tempDecompressedfd, socketBuffer, readcount) < 0){
//                        fprintf(stderr, "There was a problem opening/creating the tempfile on the clien\n");
//                        exit(EXIT_FAILURE);
//                    }//CHECK IF a+ IS WORKING PROPERLY
//
//                    //now compress the data
//                    FILE * decompressedPtr = fdopen(tempDecompressedfd, "a+");
//                    if (decompressedPtr == NULL){
//                        fprintf(stderr, "Error getting tempDecompressedfd in the server\n");
//                        exit(EXIT_FAILURE);
//                    }
//                    FILE * socketPtr = fdopen(newsocketfd, "a+");
//                    if (socketPtr == NULL){
//                        fprintf(stderr, "Error getting socketPtr in the server\n");
//                        exit(EXIT_FAILURE);
//                    }
//
//                    SET_BINARY_MODE(socketPtr);
//                    SET_BINARY_MODE(decompressedPtr);
//                    lseek(tempDecompressedfd, 0, SEEK_SET);
//
//                    ret = def(decompressedPtr, socketPtr, Z_DEFAULT_COMPRESSION); //compress it and send over the socket
//                    if (ret != Z_OK){
//                        zerr(ret);
//                        fprintf(stderr, "There was an error to compress to the socket in the server\n");
//                    }
//
//                    if (remove("temp.txt") < 0){
//                        fprintf(stderr, "Unable to remove the file temp.txt in the server\n");
//                        exit(EXIT_FAILURE);
//                    }
                
                }
                else{
                    write(newsocketfd, socketBuffer, readcount); //write to the screen
                }
            }
            //do the same for POLLHUP when the socket is closed
            if ((pollfds[0].revents & POLLHUP) || (pollfds[0].revents & POLLERR)){
                //see if there is anything available in the socket fd
                //even if there is, there is no point in it, since we would not be able to send the data back since the socket is closed
                close(pipefdFromServerToShell[1]);
            }
            
            
            //if the shell closed
            if ((pollfds[1].revents & POLLHUP) || (pollfds[1].revents & POLLERR)){ //change this to see if the exit status is for the POLLERR
                //check that there is no more input in the pipe from the shell to the terminal
                //CHECK THE REMAINING OUTPUT
                readcount = read(pipefdFromShellToServer[0], socketBuffer, sizeof(socketBuffer)); //read into the keyboardBuff
                checkReadCount(readcount);
                //turn <\n> from the shell to <\r><\n> to display on the screen
                //if the shell has closed, get the last input in the pipe, and write it to the socket
                write(newsocketfd, socketBuffer, readcount); //write to the screen
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
                    exit(0);
                }
                else if(WIFSIGNALED(status)){
                    fprintf(stderr, "Program exited with signal, %d\n",WTERMSIG(status));
                    exit(EXIT_FAILURE);
                }
                fprintf(stderr, "The status of the shell process could not be found from exit status or signal, exiting failure\n");
                exit(EXIT_FAILURE);
            }
            
        }
    }
    
    
    
    
    
    
}

