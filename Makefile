CXX=g++
CXXOPTIMIZE= -O0
BOOST=-lboost_regex
CXXFLAGS= -g $(CXXOPTIMIZE) -pthread -Wall -std=c++11 
LIBRARIES=$(BOOST)
USERID=lab2_joanne_richard
UTIL_CLASSES=utils/IPResolver.o utils/string_lib.o

.PHONY: all
all: client server

utils/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

server: $(UTIL_CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

client: $(UTIL_CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)


.PHONY: clean
clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server client targets/* *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp Makefile README.txt Vagrantfile
