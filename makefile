all:client tracker

client: client.o
	g++ client.o -o client -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto

tracker: tracker.o
	g++ tracker.o -o tracker -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lssl -lcrypto

client.o: client.cpp
	g++ -c client.cpp

tracker.o: tracker.cpp
	g++ -c tracker.cpp

clean:
	rm *.o