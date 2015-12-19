bidding_system: Priority/bidding_system.o error.o
	gcc -Wall Priority/bidding_system.o error.o -o bidding_system

Priority/bidding_system.o: Priority/bidding_system.c
	cd Priority && \
	gcc -Wall -c bidding_system.c

error.o: error.c error.h
	gcc -Wall -c error.c
