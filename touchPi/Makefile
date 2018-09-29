all: wmClient testClient

wmClient: wmClient.c
	gcc -Wall wmClient.c -lbluetooth -lcwiid -o bin/wmclient

testClient: testClient.c
	gcc -Wall testClient.c -o bin/testclient

clean:
	rm -f *.out
	rm -f bin/*
