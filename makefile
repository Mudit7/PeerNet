all:client tracker

client: client.o
	g++ client.o GetFileHash.o ConnectToTracker.o -o client -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

tracker: tracker.o
	g++ tracker.o -o tracker -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

client.o: client.cpp GetFileHash.cpp ConnectToTracker.cpp
	g++ -c client.cpp
	g++ -c GetFileHash.cpp
	g++ -c ConnectToTracker.cpp

tracker.o: tracker.cpp
	g++ -c tracker.cpp

clean:
	rm *.o