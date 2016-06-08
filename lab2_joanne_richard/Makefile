CXX=g++-5
CXXOPTIMIZE= -O0
BOOST=
CXXFLAGS= -g $(CXXOPTIMIZE) -pthread -Wall -std=c++14
LIBRARIES=$(BOOST)
USERID=lab2_joanne_richard
UTIL_CLASSES=utils/IPResolver.o utils/string_lib.o utils/TCPManager.o

.PHONY: all
all: client server client_nobuffer

utils/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

server: $(UTIL_CLASSES)
	mkdir -p targets 
	$(CXX) -o targets/$@ $^ $(CXXFLAGS) src/$@.cpp $(LIBRARIES)

client: $(UTIL_CLASSES)
	mkdir -p targets
	$(CXX) -o targets/$@ $^ $(CXXFLAGS) src/$@.cpp $(LIBRARIES)

client_nobuffer: $(UTIL_CLASSES)
	mkdir -p targets
	$(CXX) -o targets/$@ $^ $(CXXFLAGS) src/$@.cpp $(LIBRARIES)

.PHONY: clean
clean:
	rm -rf utils/*.o targets/* *.tar.gz in.data

tarball: clean
	tar -cvf $(USERID).tar.gz utils/* src/* Makefile README.md Vagrantfile
