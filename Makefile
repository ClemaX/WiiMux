all: wmClient testClient

wmClient: wmClient.c
	mkdir -p bin
	gcc -Wall wmClient.c -lbluetooth -lcwiid -o bin/wmclient

testClient: testClient.c
	mkdir -p bin
	gcc -Wall testClient.c -o bin/testclient

clean:
	rm -f *.out
	rm -f bin/*
