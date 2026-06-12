CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall

all: step1_test step2_test step3_test step4_test

step1_test: step1_test.cpp bignum.h
	$(CXX) $(CXXFLAGS) -o step1_test step1_test.cpp

step2_test: step2_test.cpp rsa.cpp montgomery.cpp
	$(CXX) $(CXXFLAGS) -o step2_test step2_test.cpp rsa.cpp montgomery.cpp

step3_test: step3_test.cpp rsa.cpp montgomery.cpp
	$(CXX) $(CXXFLAGS) -o step3_test step3_test.cpp rsa.cpp montgomery.cpp

step4_test: step4_test.cpp rsa.cpp montgomery.cpp
	$(CXX) $(CXXFLAGS) -o step4_test step4_test.cpp rsa.cpp montgomery.cpp

clean:
	rm -f step1_test step2_test step3_test step4_test test_results.txt verify_prime_result.txt

.PHONY: all clean
