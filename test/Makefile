# Makefile for hash set test program
# 
# Feb 2013 - Bryant Moscon


hash_set_test: hash_set_test.o hash_set.o
	gcc -g -Wall -o hash_set_test hash_set_test.o hash_set.o

hash_set_test.o: hash_set_test.c
	gcc -g -Wall -c hash_set_test.c

hash_set.o: ../src/hash_set.c ../src/hash_set.h
	gcc -g -Wall -c ../src/hash_set.c

clean:
	rm hash_set_test *.o
