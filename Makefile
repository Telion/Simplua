CXX = g++
FLAGS = -std=c++11 -O2
CFLAGS = -c -Wall -Wextra
LFLAGS = -o Simplua.exe -L./ -llua52
SRCS = Simplua.cpp main.cpp
OBJS = Simplua.o main.o
#CHECK = cppcheck -q --enable=style,performance,portability,information --error-exitcode=1
ECHO = echo

all: Simplua.exe

Simplua.exe: $(OBJS)
	$(ECHO) Linking Simplua.exe...
	$(CXX) $(OBJS) $(FLAGS) $(LFLAGS)

Simplua.h: Makefile
	#$(CHECK) Simplua.h

Simplua.o: Simplua.h Simplua.cpp Makefile
	$(ECHO) Compiling Simplua.cpp...
	#$(CHECK) Simplua.cpp
	$(CXX) Simplua.cpp -o Simplua.o $(FLAGS) $(CFLAGS)

main.o: Simplua.h main.cpp Makefile
	$(ECHO) Compiling main.cpp...
	#$(CHECK) main.cpp
	$(CXX) main.cpp -o main.o $(FLAGS) $(CFLAGS)

clean:
	rm -f $(OBJS) Simplua.exe
