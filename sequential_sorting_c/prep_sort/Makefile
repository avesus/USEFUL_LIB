CC=gcc
CFLAGS=-O3 -g -D_GNU_SOURCE
LDFLAGS=-lpthread -lm

H_OBJS=helpers/locks.o helpers/util.o helpers/arg.o helpers/bits.o helpers/timing.o prepsort.o helpers/opt.o helpers/temp.o sorts/timsort.o

all: driver


locks.o: helpers/locks.c helpers/locks.h
	$(CC) $(CFLAGS) -c helpers/locks.c -o helpers/locks.o $(LDFLAGS)

temp.o: helpers/temp.c helpers/temp.h
	$(CC) $(CFLAGS) -c helpers/temp.c -o helpers/temp.o $(LDFLAGS)

util.o: helpers/util.c helpers/util.h
	$(CC) $(CFLAGS) -c helpers/util.c -o helpers/util.o $(LDFLAGS)

opt.o: helpers/opt.c helpers/opt.h
	$(CC) $(CFLAGS) -c helpers/opt.c -o helpers/opt.o $(LDFLAGS)

arg.o: helpers/arg.c helpers/arg.h
	$(CC) $(CFLAGS) -c helpers/arg.c -o helpers/arg.o $(LDFLAGS)

bits.o: helpers/bits.c helpers/bits.h
	$(CC) $(CFLAGS) -c helpers/bits.c -o helpers/bits.o $(LDFLAGS)

timing.o: helpers/timing.c helpers/timing.h
	$(CC) $(CFLAGS) -c helpers/timing.c -o helpers/timing.o $(LDFLAGS)

prepsort.o: prepsort.c prepsort.h
	$(CC) $(CFLAGS) -c prepsort.c -o prepsort.o $(LDFLAGS)

timsort.o: sorts/timsort.c sorts/timsort.h
	$(CC) $(CFLAGS) -c sorts/timsort.c -o timsort.o $(LDFLAGS)

driver.o: driver.c driver.h
	$(CC) $(CFLAGS) -c driver.c -o driver.o $(LDFLAGS)

driver: driver.o $(H_OBJS)
	$(CC) $(CFLAGS) driver.o $(H_OBJS) -o driver $(LDFLAGS)

clean:
	rm -f driver *~ *.o *#* helpers/*~ helpers/*#* + helpers/*.o sorts/*~ sorts/*#* + sorts/*.o

