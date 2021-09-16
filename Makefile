twitter: client server
	make clean --directory ./client
	make clean --directory ./server
	make --directory ./client
	make --directory ./server

clean:
	make clean --directory ./client
	make clean --directory ./server
