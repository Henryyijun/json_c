def:
	gcc -Wall -g -fprofile-arcs -ftest-coverage -c -o json.o json.c
	gcc -Wall -g -fprofile-arcs -ftest-coverage -c -o demo.o demo.c
	gcc -Wall -g -fprofile-arcs -ftest-coverage -c -o test_main.o test_main.c
	gcc -Wall -g -fprofile-arcs -ftest-coverage -c -o xtest.o xtest.c
	gcc -Wall -o demo demo.o json.o -lgcov
	gcc -Wall -o test xtest.o test_main.o json.o -lgcov

clean: 
	rm -f *.o *.gcda *.gcno *.gcov demo.info
	rm -f *.yml
	rm -rf demo_web
	rm -f demo
	rm -f test

test: def
	./test --fork

check:
	valgrind --leak-check=full -v ./demo

lcov:
	lcov -d ./ -t 'demo' -o 'demo.info' -b . -c
	genhtml -o demo_web demo.info

.PHONY: def clean ut test
