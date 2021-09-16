server: main.o
	g++ -o server main.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

clean:
	rm -rf server *.o
