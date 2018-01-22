.POSIX:
CXX     = clang++-6.0
#CXX     = g++
CXXFLAGS = -O3 -std=c++17 -pedantic -Wall -Wextra

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
