all:client tracker

client: client.o
	g++ client.o -o client

tracker: tracker.o
	g++ tracker.o -o tracker

client.o: client.cpp
	g++ -c client.cpp

tracker.o: tracker.cpp
	g++ -c tracker.cpp

clean:
	rm client.o client
