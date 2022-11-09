CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = threadpooltest
OBJS = ../threadpoolV2_test.cc

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../$(TARGET)  -pthread

clean:
	rm -rf ../$(OBJS) $(TARGET)




