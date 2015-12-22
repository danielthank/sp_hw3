all: bidding_system customer bidding_system_EDF customer_EDF

bidding_system: Priority/bidding_system.o utility.o
	gcc -Wall Priority/bidding_system.o utility.o -o bidding_system

customer: Priority/customer.o utility.o
	gcc -Wall Priority/customer.o utility.o -o customer

bidding_system_EDF: EDF/bidding_system_EDF.o utility.o
	gcc -Wall EDF/bidding_system_EDF.o utility.o -o bidding_system_EDF

customer_EDF: EDF/customer_EDF.o utility.o
	gcc -Wall EDF/customer_EDF.o utility.o -o customer_EDF

Priority/bidding_system.o: Priority/bidding_system.c
	cd Priority && \
	gcc -Wall -c bidding_system.c

Priority/customer.o: Priority/customer.c
	cd Priority && \
	gcc -Wall -c customer.c

EDF/bidding_system_EDF.o: EDF/bidding_system_EDF.c
	cd EDF && \
	gcc -Wall -c bidding_system_EDF.c

EDF/customer_EDF.o: EDF/customer_EDF.c
	cd EDF && \
	gcc -Wall -c customer_EDF.c

utility.o: utility.c utility.h
	gcc -Wall -c utility.c

clean:
	rm -f *.o
	cd Priority && rm -f *.o
	cd EDF && rm -f *.o
