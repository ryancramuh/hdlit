CC = g++
CFLAGS = -Wall -Wextra -Werror -std=c++20 -g

.PHONY: hdlit
hdlit: hdlit.o
	$(CC) $(CFLAGS) -o bin/hdlit obj/hdlit.o
hdlit.o: 
	$(CC) $(CFLAGS) -c -o obj/hdlit.o src/hdlit.cpp
clean:
	rm -f obj/hdlit.o
