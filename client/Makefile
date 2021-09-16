client:	Connection.o main.o
	g++ -o client Connection.o main.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

Connection.o: Connection.cpp Connection.h
	g++ -c Connection.cpp -std=c++17 -O2 -lm

clean:
	rm -rf client *.o
