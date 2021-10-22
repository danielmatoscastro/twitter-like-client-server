server: Packet.o Inbox.o Profile.o ProfileAccessController.o ProfilesManager.o ClientConnection.o Server.o main.o
	g++ -o server ClientConnection.o Server.o ProfilesManager.o ProfileAccessController.o Profile.o Inbox.o Packet.o main.o -lpthread 

main.o: main.cpp
	g++ -c main.cpp -std=c++17 -O2 -lm -lpthread

Server.o: ../commons/Server.cpp ../commons/Server.h
	g++ -c ../commons/Server.cpp -std=c++17 -O2 -lm -lpthread

ClientConnection.o: ../commons/ClientConnection.cpp ../commons/ClientConnection.h
	g++ -c ../commons/ClientConnection.cpp -std=c++17 -O2 -lm -lpthread

ProfilesManager.o: ProfilesManager.cpp ProfilesManager.h
	g++ -c ProfilesManager.cpp -std=c++17 -O2 -lm -lpthread

ProfileAccessController.o: ProfileAccessController.cpp ProfileAccessController.h
	g++ -c ProfileAccessController.cpp -std=c++17 -O2 -lm -lpthread

Profile.o: Profile.cpp Profile.h
	g++ -c Profile.cpp -std=c++17 -O2 -lm -lpthread

Inbox.o: Inbox.cpp Inbox.h
	g++ -c Inbox.cpp -std=c++17 -O2 -lm -lpthread

Packet.o: ../commons/Packet.cpp ../commons/Packet.h
	g++ -c ../commons/Packet.cpp -std=c++17 -O2 -lm

clean:
	rm -rf server *.o
