CXX=g++
CXXOPTIMIZE= -O0
BOOST=
CXXFLAGS= -g $(CXXOPTIMIZE) -pthread -Wall -std=c++11 
LIBRARIES=$(BOOST)
USERID=lab2_joanne_richard
UTIL_CLASSES=utils/IPResolver.o utils/string_lib.o utils/TCPManager.o

.PHONY: all
all: client server

utils/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

server: $(UTIL_CLASSES)
	mkdir -p targets 
	$(CXX) -o targets/$@ $^ $(CXXFLAGS) src/$@.cpp $(LIBRARIES)

client: $(UTIL_CLASSES)
	mkdir -p targets
	$(CXX) -o targets/$@ $^ $(CXXFLAGS) src/$@.cpp $(LIBRARIES)


.PHONY: clean
clean:
	rm -rf utils/*.o targets/* *.tar.gz in.data

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp Makefile README.txt Vagrantfile
