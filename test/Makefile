# Make unit tests
CXXFLAGS = -g -std=c++17 -pedantic -Wall -Wextra

CATCHURL = https://raw.githubusercontent.com/CatchOrg/Catch2/master/single_include/catch.hpp

all : test

test : catch.hpp unittests
	valgrind --leak-check=full ./unittests

catch.hpp :
	wget $(CATCHURL)

unittests :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) line_size_test.cpp lines_index_test.cpp -o unittests

clean-tests :
	rm -f unittests

clean-catch :
	rm -f catch.hpp
	
clean : clean-tests