gcc clk.c -o clk.o 
gcc scheduler.c -o scheduler.o
gcc process_generator.c -o process_generator.o 

./clk.o
./process_generator.o processes.txt -sch 1


