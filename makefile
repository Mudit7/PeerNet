all:client tracker

client: client.o
	g++ client.o utils.o ConnectToTracker.o -o client -Wall -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

tracker: tracker.o utils.o
	g++ tracker.o utils.o ConnectToTracker.o -o tracker -Wall -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto -lpthread

client.o: client.cpp ConnectToTracker.o utils.o
	g++ -c client.cpp
	
tracker.o: tracker.cpp ConnectToTracker.o utils.o
	g++ -c tracker.cpp
	
ConnectToTracker.o: ConnectToTracker.cpp
	g++ -c ConnectToTracker.cpp
utils.o:utils.cpp
	g++ -c utils.cpp


clean:
	rm *.o