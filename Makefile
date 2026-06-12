CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall
RSA_SRCS = rsa.cpp montgomery.cpp

all: step1_test step2_test step3_test step4_test

step1_test: step1_test.cpp bignum.h
	$(CXX) $(CXXFLAGS) -o step1_test step1_test.cpp

step2_test: step2_test.cpp $(RSA_SRCS)
	$(CXX) $(CXXFLAGS) -o step2_test step2_test.cpp $(RSA_SRCS)

step3_test: step3_test.cpp $(RSA_SRCS)
	$(CXX) $(CXXFLAGS) -o step3_test step3_test.cpp $(RSA_SRCS)

step4_test: step4_test.cpp $(RSA_SRCS)
	$(CXX) $(CXXFLAGS) -o step4_test step4_test.cpp $(RSA_SRCS)

clean:
	rm -f step1_test step2_test step3_test step4_test \
	      test_results.txt verify_prime_result.txt

.PHONY: all clean
