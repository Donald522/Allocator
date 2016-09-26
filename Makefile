TEST_FILES = thirdparty/gtest/gtest-all.cc thirdparty/gtest/gtest_main.cc
SRC = src/allocator.cpp tests/allocator_test.cpp
HDR = include/allocator.h


all: tests.done

allocator_test: $(SRC) $(HDR)
	g++ -O1 -g -std=c++11 -o tests/allocator_test $(SRC) -Ithirdparty $(TEST_FILES) -lpthread

tests.done: allocator_test
	tests/allocator_test
	touch tests.done
