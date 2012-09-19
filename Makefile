test.out:main.o FileCacheImpl.o
	g++ -o test.out main.o FileCacheImpl.o -lpthread

main.o: main.cpp
	g++ -c main.cpp

FileCacheImpl.o:FileCacheImpl.cpp
	g++ -c FileCacheImpl.cpp


clean:
	rm *.o
	rm *.out
