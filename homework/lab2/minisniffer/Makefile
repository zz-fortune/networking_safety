CC = gcc
midfile = *.o
finalfile_test = test
finalfile = client


.PHONY: test run

run:
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile); then \
		rm $(finalfile); \
	fi
	@$(CC) minisniffer.h minisniffer.c -pthread -Wall -o minisniffer -lpcap
	@sudo ./minisniffer


test: 
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile_test); then \
		rm $(finalfile_test); \
	fi
	@$(CC) test.c -pthread -Wall -o test -lpcap
	@./test