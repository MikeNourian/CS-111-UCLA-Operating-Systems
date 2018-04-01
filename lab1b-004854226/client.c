//NAME: Milad Nourian
//EMAIL: miladnourian@ucla.edu
//ID: 004854226

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include<stdio.h>
#include<stdlib.h>


#include <sys/stat.h> //for using open ()
#include <fcntl.h> //for using open

#include <assert.h>
#include "zlib.h" //for compression
//#include <zlib.h>

#define true 1
#define false 0



#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
 def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
 allocated for processing, Z_STREAM_ERROR if an invalid compression
 level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
 version of the library linked do not match, or Z_ERRNO if there is
 an error reading or writing the files. */




//on success, defBuffer returns number of output bytes

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

int def(FILE *source, FILE *dest, int level)
{
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK]; //massive input buffer
    unsigned char out[CHUNK]; //massive output buffer
    
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
        
        /*
         If the parameter flush is set to Z_SYNC_FLUSH, all pending output is flushed to the output buffer and the output is aligned on a byte boundary, so that the decompressor can get all input data available so far. (In particular avail_in is zero after the call if enough output space has been provided before the call.) Flushing may degrade compression for some compression algorithms and so it should be used only when necessary. This completes the current deflate block and follows it with an empty stored block that is three bits plus filler bits to the next byte, followed by four bytes (00 00 ff ff).
    
         
         */
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        //flush = feof(source) ? Z_FINISH : Z_SYNC_FLUSH; //see if this is the problem that can change the sync problem
//no need for this
        //and then flush = Z_FINISH;
        
        strm.next_in = in;
        
        /* run deflate() on input until output buffer not full, finish
         compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);     //FLUSH = Z_FINISH    /* no bad return value */
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



//THIS IS THE CODE FOR THE CLIENT
int main(int argc, char ** argv) {
    static struct option long_options[] = {
        {"port", required_argument, 0, 'p'},
        {"log", required_argument, 0, 'l'},
        {"compress", no_argument, 0, 'c'},
        {0,         0,                 0,  0 }
    };
    
    //int shellFlag =0;
    int portFlag = 0;
    int portNumber =0;
    int logFlag = 0;
    char * logFileName;
    int compressFlag = 0;
    
    
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
                //shellFlag = 1;
                break;
            case 'p':
                portFlag = 1;
                portNumber = atoi(optarg); //to get the port number
                break;
            case 'l':
                logFlag = 1;
                logFileName = optarg;
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
    
    
    if (portFlag != 1){
        fprintf(stderr, "The client program needs a port number, exiting with failure\n");
        exit(EXIT_FAILURE);
    }
    
    //if shellFlag is set then we have to do the operations
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
    
    //code for setting up the client
    //START CLIENT CODE HERE
    //we already have the port number from the user input
    
    int socketfd;
    struct sockaddr_in server_address;
    struct hostent *server;
    
    socketfd =socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0){
        fprintf(stderr, "There was an error creating the socket for the client, exiting with failure \n");
        tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
        exit(EXIT_FAILURE);
    }
    
    server = gethostbyname("localhost"); //use the local host name
    if (server == NULL){
        fprintf(stderr, "Unable to get the host name of the server, exiting\n");
        tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
        exit(EXIT_FAILURE);
    }
    //void *memset(void *s, int c, size_t n);

    memset((char *)  &server_address, 0, sizeof(server_address));
    //bzero((char *) &server_address, sizeof(server_address));//to clean the bits of the server_address
    server_address.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&server_address.sin_addr.s_addr,
          server->h_length);
    server_address.sin_port = htons(portNumber);
    if (connect(socketfd,(struct sockaddr *)&server_address,sizeof(server_address)) < 0)
    {
        fprintf(stderr, "There was an error connecting on the client side.\n");
        tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
        exit(EXIT_FAILURE);
    }
    
    //now we are ready to read the input from the keyboard, echo it and send it to the server through a shell
    //END CLIENT SETTUP
    
    struct pollfd pollfds[2];
    pollfds[0].fd= STDIN_FILENO;
    pollfds[0].events = POLLIN | POLLHUP | POLLERR; //waiting for this event to occur
    
    pollfds[1].fd = socketfd; //CHANGE THIS LINE TO SOCKETFD
    pollfds[1].events = POLLIN | POLLHUP | POLLERR;
    
    
    //this is where I open the logFile if the option was specified by the user
    int logFileDescriptor = -1;
    if (logFlag){
        //logFileDescriptor = open(logFileName, O_RDWR | O_APPEND | O_CREAT);
        //open(filename, O_RDWR|O_CREAT, 0666)
        logFileDescriptor = open(logFileName,O_RDWR | O_APPEND | O_CREAT ,0666);
        if (logFileDescriptor < 0){
            fprintf(stderr, "There was an error opening the file, exiting\n");
            tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
            exit(EXIT_FAILURE);
        }
    }
    
    
    char modifiedBuffer [510];
    int modifiedIndex = 0;
    long readcount = 0;
    int i = 0;
    
    //char socketBuffer[510]; //this is the buffer we used when passing values to the shell
    //int socketIndex = 0;
    while (1){
        //write(STDOUT_FILENO, keyboardBuff, readcount); //write all the bytes that were count to the display
        //write(STDOUT_FILENO, modifiedBuffer, modifiedIndex);
        //(1) now we have to pipe this string to the shell process
        //get shellBuffer
        //poll to wait for the input from the user
        int polled = poll(pollfds, 2, 0); //time out is zero
        if (polled == -1){
            fprintf(stderr, "There was a problem with polling, REASON: %s\n", strerror(errno));
            tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
            exit(EXIT_FAILURE);
        }
        
        //now check what has happened on the polling
        if (pollfds[0].revents & POLLIN){//if the input from the STDIN is available
            
            memset(keyboardBuff, 0, sizeof(keyboardBuff));
            //fprintf(stderr, "User entered and poll recognized it");
            readcount = read(STDIN_FILENO, keyboardBuff, sizeof(keyboardBuff));
            
            
            
            //fprintf(stderr, "We have read the value that we copied to keyboardBuff and the value is\n");
            //fprintf(stderr, "value: %s\n", keyboardBuff);
            checkReadCount(readcount);
            if (logFlag == 1){//we have the open file descriptor so write to it
                if (compressFlag == 0){ //if we dont have to compress
                    //fprintf(stderr, "logFlag = 1 and compressFlag = 0\n");
                    char beginString[14] = "SENT x bytes: ";
                    beginString[5] = '0' + readcount;
                    write(logFileDescriptor, beginString, 14);
                    write(logFileDescriptor, keyboardBuff, readcount);
                    write(logFileDescriptor, "\n", 1);
                }
            }
            
            if (compressFlag == 1){
                //should compress the
                //START COMMENT HERE
                
                //create a source and a dest buffer -- both massive
                //unsigned char src[CHUNK]; //massive input buffer
                unsigned char dest[CHUNK]; //massive output buffer
                //the data we read is in keyboard buf
                int bytes = defBuffer(keyboardBuff, 255, dest, CHUNK, Z_DEFAULT_COMPRESSION);
                if (bytes < 0){
                    fprintf(stderr, "ret in defBuffer returned -1, error\n");
                    tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                    exit(EXIT_FAILURE);
                }
                
                //now the dest Buff must have some compressed data that we can write out
                write(socketfd, dest,bytes); //write to the socketfd the compressed data and continue
                if (logFlag){ //if have to keep log of the bytes
                    char beginString[14] = "SENT x bytes: ";
                    beginString[5] = '0' + bytes;
                    write(logFileDescriptor, beginString, 14);
                    write(logFileDescriptor, dest, bytes);
                    write(logFileDescriptor, "\n", 1);
                }
                
                //FINISH HERE
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
            //now write that keyboard input to the socket to the server
            //now we are sending the value over the socket so if the --log flag is given
            if (!compressFlag){
                write(socketfd, modifiedBuffer,modifiedIndex); //if the user did not specify the --compress option, just write the buffer to the socketfd
            }
            
        }
        if (pollfds[1].revents & POLLIN){ //this is the input from the socket
            
            //Client has received some data over the socket from the server
            
            readcount = read(socketfd, keyboardBuff, sizeof(keyboardBuff)); //read into the keyboardBuff
            checkReadCount(readcount);
            //continue processing until EOF
            //OR UNTIL THE fd[1] POLLUP becomes available
            if (readcount == 0){
                tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                exit(EXIT_SUCCESS);
            }
            
            /*
             char log_string[18] = "RECEIVED x bytes: ";
             log_string[9] = '0' + num_bytes;
             write(file_fd, log_string, 18);
             write(file_fd, buffer+byte_offset, 1);
             write(file_fd, "\n",1);
             */
            if (logFlag == 1){//we have the open file descriptor so write to it
                //char * buf [modifiedIndex + 20]; //check if WE SHOULD USE MODIFIEDINDEX OR READCOUNT, readcount does not have the translation to <cr><fl>
                //use malloc
                //ORIGINALLY there was an if statment here, but there is no need for one
                //if (!compressFlag){
                
                
                //this passes the RECEIVED TEST
                //reduce the buf size by 4 or 5
                
                
                char buffer[20]; //failed on 21
                sprintf(buffer, "RECEIVED %d bytes: ", (int)readcount);
                write(logFileDescriptor, buffer, sizeof(buffer));
                write(logFileDescriptor, keyboardBuff, readcount);
                write(logFileDescriptor, "\n", 1);
                //}
                
            }
            //now we have the content compressed data from the socket that we have read into the before | keyboardBuff
            if (compressFlag){
                
                //decompress
                //we have read from the keyboardBuff and readcount
                char dest[CHUNK];
                
                int bytes = infBuffer(keyboardBuff, 255, dest, CHUNK); //size of the buffers
                if (bytes < 0){
                    fprintf(stderr, "There was problem with decompressing, but continuing\n");
                }
                
                //now dest has the data we want
                
                
                
                //change keyboardBuff to dest
                modifiedIndex = 0;
                //turn <\n> from the shell to <\r><\n> to display on the screen
                for (i =0; i <  bytes; i++) {
                    if (dest[i] == '\n'){
                        modifiedBuffer[modifiedIndex] = '\r';
                        modifiedBuffer[modifiedIndex + 1] = '\n';
                        modifiedIndex = modifiedIndex + 2;
                    }
                    else{
                        modifiedBuffer[modifiedIndex] = dest[i];
                        modifiedIndex = modifiedIndex + 1;
                    }
                }
                
                //write to std out
                if (write(STDOUT_FILENO, modifiedBuffer, modifiedIndex)<0){
                    fprintf(stderr, "Writing modifiedBuffer to the buffer e\n");
                }
            }
            if (!compressFlag){
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
            }
            
        }
        //This occurs when the server program exits and the socket is closed
        if ((pollfds[1].revents & POLLHUP) || (pollfds[1].revents & POLLERR)){ //change this to see if the exit status is for the POLLERR
            //check that there is no more input in the pipe from the shell to the terminal
            //CHECK THE REMAINING TO SEE IF ANYTHING LEFT IN THE SOCKET
            readcount = read(socketfd, keyboardBuff, sizeof(keyboardBuff)); //read into the keyboardBuff
            if (readcount == 0){
                tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
                exit(EXIT_SUCCESS);
            }
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
            tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
            exit(EXIT_FAILURE);
        }
        
    }
}

