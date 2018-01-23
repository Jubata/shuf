.POSIX:
CXX     = clang++-6.0
#CXX     = g++
CXXFLAGS = -g -O3 -std=c++17 -pedantic -Wall -Wextra

BIN=shuffle_ssd

SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)

all: $(OBJ)
	$(CXX) -o $(BIN) $^

%.o: %.c
	$(CXX) $@ -c $<

clean:
	rm -f *.o
	rm $(BIN)

depend: .depend

.depend: $(SRC)
	rm -f ./.depend
	$(CXX) $(CXXFLAGS) -MM $^ -MF  ./.depend;

include .depend

tests:
	cd test; $(MAKE) $(MFLAGS)

clean-test:
	cd test; $(MAKE) $(MFLAGS) clean
