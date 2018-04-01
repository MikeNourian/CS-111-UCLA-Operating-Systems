default:
	gcc -Wextra -Wall -g -lmraa -std=gnu99 main.c -o lab4b -lm
check: test
clean:
	rm -f lab4b lab4b-004854226.tar.gz
dist:
	tar -czvf lab4b-004854226.tar.gz main.c README Makefile
test:
	echo | ./lab4b --bogus &> /dev/null; \
	if [[ $? -ne 1 ]]; then \
	echo "Test FAILED: bad argument returns wrong exit code"; \
	else \
	echo "Test PASSED: bad argument returns correct exit code"; \
	fi
