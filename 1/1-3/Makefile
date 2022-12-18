all: main


main: main.cpp libfunctions.a
	g++ $< -L. -lfunctions -o $@

libfunctions.a: myls.o
	ar r $@ $^


myls.o: myls.cpp
	g++ -c $< -o $@

clean:
	rm -rf *.o *.a main  