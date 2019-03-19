CC = g++
midfile = *.o
finalfile_test = test
finalfile_client = client
final_server = server

.PHONY: test client server

test: 
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile_test); then \
		rm $(finalfile_test); \
	fi
	@$(CC) FileTransfer.h test.c -pthread -o test
	@./test

client: 
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile_client); then \
		rm $(finalfile_client); \
	fi
	@$(CC) FileTransfer.h client.c -o client
	@./client

server: 
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(final_server); then \
		rm $(final_server); \
	fi
	@$(CC) FileTransfer.h server.c -pthread -o server
	@./server