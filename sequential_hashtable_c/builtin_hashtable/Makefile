CXX=g++
CXXFLAGS=-O3 -std=c++11 -g -D_GNU_SOURCE
LDFLAGS=-lpthread -lm

H_OBJS=helpers/locks.o helpers/temp.o helpers/opt.o helpers/util.o helpers/arg.o helpers/bits.o helpers/timing.o

all: driver

locks.o: helpers/locks.cpp helpers/locks.h
	$(CXX) $(CXXFLAGS) -c helpers/locks.cpp -o helpers/locks.o $(LDFLAGS)

temp.o: helpers/temp.cpp helpers/temp.h
	$(CXX) $(CXXFLAGS) -c helpers/temp.cpp -o helpers/temp.o $(LDFLAGS)

opt.o: helpers/opt.cpp helpers/opt.h
	$(CXX) $(CXXFLAGS) -c helpers/opt.cpp -o helpers/opt.o $(LDFLAGS)

util.o: helpers/util.cpp helpers/util.h
	$(CXX) $(CXXFLAGS) -c helpers/util.cpp -o helpers/util.o $(LDFLAGS)

arg.o: helpers/arg.cpp helpers/arg.h
	$(CXX) $(CXXFLAGS) -c helpers/arg.cpp -o helpers/arg.o $(LDFLAGS)

bits.o: helpers/bits.cpp helpers/bits.h
	$(CXX) $(CXXFLAGS) -c helpers/bits.cpp -o helpers/bits.o $(LDFLAGS)

timing.o: helpers/timing.cpp helpers/timing.h
	$(CXX) $(CXXFLAGS) -c helpers/timing.cpp -o helpers/timing.o $(LDFLAGS)

driver.o: driver.cpp driver.h
	$(CXX) $(CXXFLAGS) -c driver.cpp -o driver.o $(LDFLAGS)

hashtable_alt.o: hashtable_alt.cpp hashtable_alt.h 
	$(CXX) $(CXXFLAGS) -c hashtable_alt.cpp -o hashtable_alt.o $(LDFLAGS)

driver: driver.o $(H_OBJS)
	$(CXX) $(CXXFLAGS) driver.o $(H_OBJS) -o driver $(LDFLAGS)

clean:
	rm -f driver *~ *.o *#* helpers/*~ helpers/*#* + helpers/*.o

