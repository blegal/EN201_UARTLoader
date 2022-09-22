CC=g++
CFLAGS=-O3 -Wall -std=c++11 -I./include/
LDFLAGS=

EXEC=main

SRC=./src/main.cpp
	
OBJ= $(SRC:.cpp=.o)

all: $(EXEC)

main: $(OBJ)
	$(CC) $(CFLAGS) -o ./bin/$@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	find . -name "*.o" -delete
	find ./bin/$(EXEC) -delete
