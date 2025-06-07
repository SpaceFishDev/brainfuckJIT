src = $(wildcard src/*.c)
out = bfjit

all: build run

build:
	gcc $(src) -o $(out)

run:
	./$(out)