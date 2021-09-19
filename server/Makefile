server: Packet.o Profile.o ClientConnection.o Server.o main.o
	g++ -o server ClientConnection.o Server.o Profile.o Packet.o main.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

Server.o: Server.cpp Server.h
	g++ -c Server.cpp -std=c++17 -O2 -lm -lpthread

ClientConnection.o: ClientConnection.cpp ClientConnection.h
	g++ -c ClientConnection.cpp -std=c++17 -O2 -lm -lpthread

Profile.o: Profile.cpp Profile.h
	g++ -c Profile.cpp -std=c++17 -O2 -lm -lpthread

Packet.o: ../commons/Packet.cpp ../commons/Packet.h
	g++ -c ../commons/Packet.cpp -std=c++17 -O2 -lm

clean:
	rm -rf server *.o
