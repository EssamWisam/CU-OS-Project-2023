build:
	gcc process_generator.c -o process_generator.o
	gcc clk.c -o clk.o
	gcc scheduler.c -o scheduler.o
	gcc process.c -o process.o
	gcc test_generator.c -o test_generator.o
clean:
	rm -f *.o  processes.txt

all: clean build

run:
	./process_generator.o
