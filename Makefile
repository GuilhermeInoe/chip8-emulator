CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -std=c++17 $(shell sdl2-config --cflags)

TARGET = chip8
OBJS = c8vm.o main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(shell sdl2-config --libs)

c8vm.o: c8vm.cpp c8vm.h defs.h
	$(CXX) $(CXXFLAGS) -c c8vm.cpp

main.o: main.cpp c8vm.h defs.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET) 1-chip8-logo.ch8