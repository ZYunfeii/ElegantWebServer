CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = redistest
OBJS = ../*.cpp ../../log/*.cpp ../../buffer/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../$(TARGET)  -pthread -lhiredis

clean:
	rm -rf ../$(OBJS) $(TARGET)




