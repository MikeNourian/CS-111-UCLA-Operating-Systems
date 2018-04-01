//NAME: Milad Nourian
//EMAIL: miladnourian@ucla.edu
//ID: 004854226


#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h> //for exit status
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h> //for registering handlers
//prototype
//int getopt_long(int argc, char * const argv[], const char *optstring,const struct option *longopts, int *longindex);
//int dup2(int oldfd, int newfd);


void signal_handle_for_segmentation (int signalNumber){
  fprintf(stderr,"SEGSEV was generated, signal number: %d\n", signalNumber);
  exit(4);
}

void segmentation_fault_generator() {
  char * ptr = NULL;
  *ptr = 'A';
}

int main(int argc, char **argv) {
  int c;
  int segGiven = 0;
  int catchGiven = 0;
  int fd0 = 0; //just to initialize in the case the user did not enter the input file, we will use fd0 as stdin
  int fd1 = 1; //just to initialize, stdout as fd1 as default
    
  static struct option long_options[] = {
    {"input",     required_argument, 0, 'i' },
    {"output",  required_argument, 0,  'o' },
    {"segfault", no_argument,       0,  's' },
    {"catch",  no_argument, 0, 'c'},
    {0,         0,                 0,  0 }
  };
    
  while (1) //a while loop that does not end
    {
      c = getopt_long(argc, argv, "", long_options, 0); //we leave optString since we have no small options
      if (c==-1){
	break; //to break out of the loop
      }
      switch(c){
      case 'i':
	fd0 = open(optarg, O_RDONLY);
	//see if there we any issues opening the file
	if (fd0 == -1){
	  //there is an error
	  fprintf(stderr, "There was an error opening the input file : %s *** Reason: %s \n" ,optarg, strerror(errno));
	  exit(2); //given in the specs
	}
	else if (fd0 >= 0){//we successfully opened the file and ready to redirect the input to fd0
	  close(0);
	  if (dup2(fd0, STDIN_FILENO) == -1){
	    fprintf(stderr, "**Error: The process of setting the file descriptor and redirecting input file failed** Reason: %s\n",strerror(errno)); //we can use the errno for the error
	    exit(2);
	  }
	  //now the input file has the stderror
	}
	break;
      case 'o':
	fd1 = creat(optarg, 0666); //to create the file with name optarg
	if (fd1 == -1){
	  fprintf(stderr, "There was an error opening the output file : %s *** Reason: %s \n" ,optarg, strerror(errno));
	  exit(3);
	}
	//now we have successfully opened the file
	else if (fd1 >= 0){
	  close(1);
	  if (dup2(fd1, STDOUT_FILENO) == -1){
	    fprintf(stderr, "**Error: The process of setting the file descriptors and redirecting output file failed** Reason: %s\n",strerror(errno)); //we can use the errno for the error
	    exit(2);
	  }
	}
	break;
      case 's':
	segGiven = 1; //so means that we do
	break;
      case 'c':
	catchGiven = 1;
	break;
      case '?':
      default:
	fprintf(stderr, "Error: Unrecognized option *%s* entered\n", argv[optind-1]);
	fprintf(stderr,"Usage: ./lab0 --input=inputFile --output=outputFile --segfault --catch\n");
	  return (1);
      }
    }
  //if --input, --output are not entered, the program will use the stdin and stdout
    
    
  //register the signal if the user gave the option in the command line
  if (catchGiven){
    signal(SIGSEGV, signal_handle_for_segmentation);
  }
    
  if (segGiven){
    segmentation_fault_generator();
  }
    
  //for this part, we read AND write one character at a time
  char * buf = (char*) malloc(sizeof(char));//this is the buffer
  ssize_t byteCount;
  byteCount = read(fd0, buf, 1);
 
  while (byteCount>0) //it means that we have read more than 0 bytes, so we keep looping until we are at the end or get an error
    {
      write(fd1, buf, 1);
      byteCount = read(fd0, buf, 1);
    }  
  free(buf);
  exit (0); //successful in copying
}

