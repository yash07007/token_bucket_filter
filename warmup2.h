#ifndef _WARMUP2_H_
#define _WARMUP2_H_

#include <stdio.h>
#include "my402list.h"

#ifndef BUFF_LEN
#define BUFF_LEN 1024
#endif /* ~BUFF_LEN */

#ifndef OPTIONS_LEN
#define OPTIONS_LEN 7
#endif /* ~OPTIONS_LEN */

#ifndef MAX_INT
#define MAX_INT 2147483647
#endif /* ~MAX_INT */

#ifndef LAMBDA_INITIAL_VALUE
#define LAMBDA_INITIAL_VALUE 0.5
#endif /* ~LAMBDA_INITIAL_VALUE */

#ifndef MU_INITIAL_VALUE
#define MU_INITIAL_VALUE 0.35
#endif /* ~MU_INITIAL_VALUE */

#ifndef R_INITIAL_VALUE
#define R_INITIAL_VALUE 1.5
#endif /* ~R_INITIAL_VALUE */

#ifndef B_INITIAL_VALUE
#define B_INITIAL_VALUE 10
#endif /* ~B_INITIAL_VALUE */

#ifndef P_INITIAL_VALUE
#define P_INITIAL_VALUE 3
#endif /* ~P_INITIAL_VALUE */

#ifndef N_INITIAL_VALUE
#define N_INITIAL_VALUE 20
#endif /* ~N_INITIAL_VALUE */

// all time in usec
typedef struct Packet {
    int index;
    double inter_arrival_time;
    double mesured_inter_arrival_time;
    int token_requirement;
    double service_time;
    double mesured_service_time;

    double packet_arrival_time;
    double packet_Q1_in_time;
    double packet_Q1_out_time;
    double packet_Q2_in_time;
    double packet_Q2_out_time;
    double packet_S_in_time;
    double packet_S_out_time;

    int processing_server_id;
} Packet;

enum OPTIONS {
    INDEX_OF_LAMBDA, // index of packet arrival rate
    INDEX_OF_MU,     // index of packet service time 
    INDEX_OF_R,      // index of token arrival rate
    INDEX_OF_B,      // index of token bucket size
    INDEX_OF_P,      // index of token requirement for packet
    INDEX_OF_N,      // index of number of packets
    INDEX_OF_TSFILE, // index of trace specification file
};

enum MODE {
    DETERMINISTC_MODE,
    TRACE_DRIVEN_MODE
};

enum SERVER {
    SERVER_A,
    SERVER_B
};

enum STATS {
    AVG_INTER_ARRIVAL_TIME,
    AVG_PACKET_SERVICE_TIME,
    AVG_NO_OF_PACKET_IN_Q1,
    AVG_NO_OF_PACKET_IN_Q2,
    AVG_NO_OF_PACKET_AT_S1,
    AVG_NO_OF_PACKET_AT_S2,
    AVG_TIME_IN_SYSTEM,
    AVG_TIME_IN_SYSTEM_SQUARE,
    STD_TIME_IN_SYSTEM,
    PACKET_DROP_PROBABLITY,
    TOKEN_DROP_PROBABLITY
};

/* get filename from path */
void setProgramName(char*);
/* print valid command visa */
void usage();
/* print error message and exit */
void reportError(char*);
/* read valid float */
double getValidFloat(char*);
/* read valid int */
int getValidInt(char*);
/* square double and return */
double square(double);
/* get current relative time (current time - program start time) in msec */
double getCurrentTime();
/* convert seconds to micro seconds */
double timeSecToUsec(double);
/* convert milli seconds to micro seconds */
double timeMsecToUsec(int);
/* update running average */
double updateAvg(int, double, double);
/* update running statistics */
void updateStatistics(Packet*);
/* get valid file handler */
FILE* getFileHandler(char*);
/* print list */
void printList(My402List*);
/* get index of valid parameter */
int getOptionIndex(char*);;
/* read program parameters and update global variables */
void processOptions(int, char**);
/* create event queue from global parameters */
void setupSimulation();
/* start simulation of event queue */
void startSimulation();
/* 
start
sleep for next packet to arrive
lock mutex
timestamp packet for arrival
if required tokens for packet > bucket size
    drop packet
    unlock mutex
    goto start
enqueue packet to Q1
timestamp packet for Q1 start
if Q1 has just one element and token bucket has enouch tokens for first element in Q1 
    use all tokens
    transfer packet form Q1 to Q2
    if Q2 was empty before transfer
        broadcast to cv
unlock mutex
goto start
*/
void* handlePacketArrivalThread(void*);

/* 
start
sleep for next token to arrive
lock mutex
if Q1 and eventQ both are empty 
    unlock mutex and terminate thread
if token < size of bucket
    add token
else
    drop token
if Q1 is not empty and token bucket has enouch tokens for first element in Q1 
    use all tokens
    transfer packet form Q1 to Q2
    if Q2 was empty before transfer
        broadcast to cv
unlock mutex
goto start
*/
void* handleTokenArrivalThread(void*);
/* 
start
lock mutex
if Q2, Q1 and eventQ are empty
    unlock mutex
    exit thread
while Q2 is empty
    condition wait for cv
dequeue from Q2
unlock mutex
timestamp packet for Q2 out
timestamp packet in S in
sleep for service time
timestamp packet for S out
update statistics
*/
void* handleServerThread(void*);
/* update running statistics */
void updateStatistics(Packet*);
/* display statistics */
void displayStatistics(double);

#endif /* _WARMUP2_H_ */
