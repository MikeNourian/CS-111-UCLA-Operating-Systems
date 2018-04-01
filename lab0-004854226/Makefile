R#NAME: Milad Nourian
#EMAIL: miladnourian@ucla.edu
#ID: 004854226

ID=004854226
FILES=lab0.c Makefile backtrace.png breakpoint.png README
.SILENT:
#SILENT allows to only showcase the error messages generated and not the code ran in the Makefile

all :
	gcc -Wall -Wextra -g lab0.c -o lab0

check: all runAllTest

runAllTest: checkSuccess checkOpenFailure checkCreatFailure checkSegFaultAndCatch checkUnrecognized

clean:
	rm -f  lab0 lab0-004854226.tar.gz

#put the smoke check cases here 
dist: all
	tar -czf lab0-004854226.tar.gz $(FILES)


checkSuccess:
	echo "Hello World!" > inputFile.txt;\
	./lab0 --input=inputFile.txt --output=outputFile.txt ; \
	if [ $$? -ne 0 ] ; \
	then\
		echo "checkSuccess failed" ;\
	fi;\
	rm *.txt 

checkUnrecognized:
	./lab0 --badargument ;\
	if [ $$? -ne 1 ];\
	then\
		echo "There was an error handling the unrecognized argument, exited" ;\
	fi ;\

checkOpenFailure:
	echo "Hello World!" > inputFile.txt;\
	chmod ugo-r inputFile.txt ;\
	./lab0 --input=inputFile.txt --output=outputFile.txt ; \
	if [ $$? -ne 2 ] ;\
	then\
		echo "checkOpenFailure faild with exit $$?" ;\
	fi;\
	chmod 777 inputFile.txt ;\
	rm *.txt

checkCreatFailure:
	echo "Hello World!" > inputFile.txt;\
	touch outputFile.txt;\
	chmod u-w outputFile.txt ;\
	./lab0 --input=inputFile.txt --output=outputFile.txt ; \
	if [ $$? -ne 3 ] ;\
	then\
		echo "checkCreatFailure failed with exit $$?";\
	fi;\
	chmod u+w outputFile.txt
	rm *.txt

checkSegFaultAndCatch:
	./lab0 --segfault --catch;\
	if [ $$? -ne 4 ];\
	then\
		echo "checkSegFaultAndCatch with exit $$?";\
	fi;\




