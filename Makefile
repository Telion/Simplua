CXX = /usr/csshare/pkgs/gcc-4.7.2/bin/g++
FLAGS = -std=c++0x -O2 -static
CFLAGS = -c -Wall -Wextra -I./include
LFLAGS = -o Simplua.exe -L./ -L./lib -llua -ldl
SRCS = Simplua.cpp main.cpp
OBJS = Simplua.o main.o
ECHO = echo

all: Simplua.exe

Simplua.exe: $(OBJS)
	$(ECHO) Linking Simplua.exe...
	$(CXX) $(OBJS) $(FLAGS) $(LFLAGS)

Simplua.h: Makefile

Simplua.o: Simplua.h Simplua.cpp Makefile
	$(ECHO) Compiling Simplua.cpp...
	$(CXX) Simplua.cpp -o Simplua.o $(FLAGS) $(CFLAGS)

main.o: Simplua.h main.cpp Makefile
	$(ECHO) Compiling main.cpp...
	$(CXX) main.cpp -o main.o $(FLAGS) $(CFLAGS)

clean:
	rm -f $(OBJS) Simplua.exe
