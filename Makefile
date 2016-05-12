CXX=g++
CXXOPTIMIZE= -O0
BOOST=
CXXFLAGS= -g $(CXXOPTIMIZE) -pthread -Wall -std=c++11 
LIBRARIES=$(BOOST)
USERID=lab2_joanne_richard
CLASSES=

.PHONY: all
all: web-server web-client web-client-timeout web-server-timeout web-server-async web-server-1_1 web-client-1_1

%.o: %.h %.c
	$(CXX) $(CXXFLAGS) -c %.cpp 

web-server: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-client: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-server-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-client-timeout: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-server-async: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-server-1_1: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)

web-client-1_1: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp $(LIBRARIES)



%: $(CLASSES) 
	$(CXX)

	
.PHONY: clean
clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server-async web-server web-client a.out web-server-timeout web-client-timeout web-server-1_1 web-client-1_1 *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.cpp Makefile README.txt Vagrantfile
