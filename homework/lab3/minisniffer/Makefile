CC = gcc
midfile = *.o
finalfile_test = test
finalfile = minisniffer

.PHONY: test run

run:
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile); then \
		rm $(finalfile); \
	fi
	@$(CC) minisniffer.c -lnet -o minisniffer
	@sudo ./minisniffer

test: 
	@if test -f $(midfile); then \
		rm $(midfile);\
	elif test -f $(finalfile_test); then \
		rm $(finalfile_test); \
	fi
	@$(CC) test.c -lnet -o test
	@sudo ./test
