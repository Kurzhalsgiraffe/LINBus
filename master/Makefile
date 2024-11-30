
lin.o: lin.cpp
	g++ -c lin.cpp -o lin.o

lin: lin.o
	g++ -Wall -pthread -o lin lin.cpp -lpigpio -lrt

ser.o: ser.cpp
	g++ -c ser.cpp -o ser.o

ser: ser.o
	g++ -Wall -pthread -o ser ser.cpp -lpigpio -lrt

clean:
	rm lin
	rm lin.o
