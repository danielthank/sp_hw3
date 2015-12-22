all: bidding_system customer bidding_system_EDF

bidding_system: Priority/bidding_system.o error.o
	gcc -Wall Priority/bidding_system.o error.o -o bidding_system

customer: Priority/customer.o error.o
	gcc -Wall Priority/customer.o error.o -o customer

bidding_system_EDF: EDF/bidding_system_EDF.o error.o
	gcc -Wall EDF/bidding_system_EDF.o error.o -o bidding_system_EDF

Priority/bidding_system.o: Priority/bidding_system.c
	cd Priority && \
	gcc -Wall -c bidding_system.c

Priority/customer.o: Priority/customer.c
	cd Priority && \
	gcc -Wall -c customer.c

EDF/bidding_system_EDF.o: EDF/bidding_system_EDF.c
	cd EDF && \
	gcc -Wall -c bidding_system_EDF.c

error.o: error.c error.h
	gcc -Wall -c error.c
