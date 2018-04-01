ID=004854226
default:
	gcc -Wextra -Wall -g -lmraa -std=gnu99 4c_tcp.c -o lab4c_tcp -lm
	gcc -Wextra -Wall -g -lmraa -std=gnu99 -lssl -lcrypto 4c_tls.c -o lab4c_tls -lm
tcp_test:
	./lab4c_tcp --id=124567893 --host=131.179.192.136 --log=tcp.txt 18000
tls_test:
	./lab4c_tls --id=124567893 --host=131.179.192.136 --log=tls_log.txt 19000
clean:
	rm -rf lab4c_tls lab4c_tcp lab4c-$(ID).tar.gz
dist:
	tar -czvf lab4c-$(ID).tar.gz 4c_tls.c 4c_tcp.c README Makefile

