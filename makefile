all:client tracker

client: client.o
	g++ client.o utils.o ConnectToTracker.o -o client -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

tracker: tracker.o utils.o
	g++ tracker.o utils.o ConnectToTracker.o -o tracker -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

client.o: client.cpp ConnectToTracker.cpp utils.cpp
	g++ -c client.cpp
	g++ -c ConnectToTracker.cpp
	g++ -c utils.cpp
tracker.o: tracker.cpp
	g++ -c tracker.cpp
	g++ -c ConnectToTracker.cpp
	g++ -c utils.cpp

clean:
	rm *.o