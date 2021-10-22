router: Packet.o ClientConnection.o Server.o main.o
	g++ -o router main.o Server.o ClientConnection.o Packet.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

Server.o: ../commons/Server.cpp ../commons/Server.h
	g++ -c ../commons/Server.cpp -std=c++17 -O2 -lm

ClientConnection.o: ../commons/ClientConnection.cpp ../commons/ClientConnection.h
	g++ -c ../commons/ClientConnection.cpp -std=c++17 -O2 -lm

Packet.o: ../commons/Packet.cpp ../commons/Packet.h
	g++ -c ../commons/Packet.cpp -std=c++17 -O2 -lm

clean:
	rm -rf router *.o
