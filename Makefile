twitter: client server
	make clean --directory ./client
	make clean --directory ./server
	make clean --directory ./router
	make --directory ./client
	make --directory ./server
	make --directory ./router

clean:
	make clean --directory ./client
	make clean --directory ./server
	make clean --directory ./router
