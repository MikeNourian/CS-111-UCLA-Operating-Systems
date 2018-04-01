#NAME: Milad Nourian
#ID: 004854226
#EMAIL: miladnourian@ucla.edu

build:
	gcc -Wextra -Wall -lprofiler -pthread -g SortedList.c lab2_list.c -o lab2_\
list

profile: clean build
	rm -f miladsprofile.out profile.out
	LDPRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=miladsprofile.out ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./miladsprofile.out > profile.out
	pprof --list=spinLock ./lab2_list ./miladsprofile.out >> profile.out
	rm -f miladsprofile.out


clean:
	rm -rf lab2_list  lab2b-004854226.tar.gz
dist: graphs profile
	tar -zcvf lab2b-004854226.tar.gz Makefile README lab2_list_new.gp *.png *.c\
sv SortedList.c lab2_list.c *.h generateGoodList.sh profile.out

graphs: tests
	gnuplot lab2_list_new.gp

tests: clean build
	./generateGoodList.sh
