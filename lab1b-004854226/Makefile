#NAME: Milad Nourian
#EMAIL: miladnourian@ucla.edu
#ID: 004854226
FILES= README server.c client.c Makefile

all:
	gcc -Wall -Wextra -g -lz server.c -o lab1b-server
	gcc -Wall -Wextra -g -lz client.c -o lab1b-client
clean:
	rm -f lab1b-server lab1b-client lab1b-004854226.tar.gz

dist: all
	tar -czf lab1b-004854226.tar.gz $(FILES)
