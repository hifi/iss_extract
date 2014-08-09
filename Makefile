all:
	gcc -std=c99 -Wall -o iss_extract iss_extract.c

clean:
	rm -f iss_extract
