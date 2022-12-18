all: main

#Final linking
main: main.cpp libfunctions.a
	g++ $< -L. -lfunctions -o $@

#Library creation
libfunctions.a: soucet.o soucin.o podil.o
	ar r $@ $^

#Creating object files of library
soucet.o: add.cpp
	g++ -c $< -o $@

soucin.o: mul.cpp
	g++ -c $< -o $@

podil.o: div.cpp
	g++ -c $< -o $@

clean:
	rm -rf *.o *.a main  