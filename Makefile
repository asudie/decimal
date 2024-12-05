CC=gcc
FLAGS= -Wall -Werror -Wextra
LIBS_LINUX= -lcheck -lm -lpthread -lrt -lsubunit -D_GNU_SOURCE
LIBS_MACOS= -L /usr/local/lib -lcheck

OS = $(shell uname)
ifeq ($(OS), Darwin)
OPEN=open gcov_report/index.html
LIBS=$(LIBS_MACOS)
# LEAK_CHECK = CK_FORK=no leaks -atExit -- ./s21_test
endif
 
ifeq ($(OS), Linux)
OPEN=lynx
LIBS=$(LIBS_LINUX)
# LEAK_CHECK = valgrind --leak-check=full --show-leak-kinds=all -s ./test
endif




all: test gcov_report

s21_decimal.o_with_gcov: s21_decimal.c s21_decimal_utils.c 
	$(CC) -c s21_decimal.c s21_decimal_utils.c --coverage

s21_decimal.a_with_gcov: s21_decimal.o_with_gcov 	
	ar -rcs s21_decimal.a s21_decimal.o s21_decimal_utils.o


test: test.check s21_decimal.a_with_gcov
	checkmk clean_mode=1 test.check > test.c
	$(CC) $(FLAGS) test.c s21_decimal.a -o executable $(LIBS) --coverage
	./executable

s21_decimal.a:
	$(CC) -c s21_decimal.c s21_decimal_utils.c
	ar -rcs s21_decimal.a s21_decimal.o s21_decimal_utils.o

gcov_report:
	gcovr --html --html-details -o report.html
	open report.html

cpp:
# cppcheck --enable=all --suppress=missingIncludeSystem *.h *.c
	clang-format -style=google -i *.c
	clang-format -style=google -i *.h
	clang-format -style=google -n *.c
	clang-format -style=google -n *.h


clean:
	rm -f *.out *.gcda *.gcno *.o *.a report.* s21_test test_with_gcov* *.gcov test.c
