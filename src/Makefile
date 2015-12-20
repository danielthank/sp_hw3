all: bidding_system customer

bidding_system: Priority/bidding_system.o error.o
	gcc -Wall Priority/bidding_system.o error.o -o bidding_system

customer: Priority/customer.o error.o
	gcc -Wall Priority/customer.o error.o -o customer

Priority/bidding_system.o: Priority/bidding_system.c
	cd Priority && \
	gcc -Wall -c bidding_system.c

Priority/customer.o: Priority/customer.c
	cd Priority && \
	gcc -Wall -c customer.c

error.o: error.c error.h
	gcc -Wall -c error.c
