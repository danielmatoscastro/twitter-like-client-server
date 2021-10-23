client:	Connection.o Packet.o main.o
	g++ -o client Connection.o Packet.o main.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

Connection.o: ../commons/Connection.cpp ../commons/Connection.h
	g++ -c ../commons/Connection.cpp -std=c++17 -O2 -lm

Packet.o: ../commons/Packet.cpp ../commons/Packet.h
	g++ -c ../commons/Packet.cpp -std=c++17 -O2 -lm

clean:
	rm -rf client *.o
