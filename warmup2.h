#ifndef _WARMUP2_H_
#define _WARMUP2_H_

typedef struct Packet {
    int index;
    double inter_arrival_time; //usec
    int token_requirement;
    double service_time; //usec

    double packet_arrival_time;
    double packet_Q1_in_time;
    double packet_Q1_out_time;
    double packet_Q2_in_time;
    double packet_Q2_out_time;
    double packet_S_in_time;
    double packet_S_out_time;
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

#endif /* _WARMUP2_H_ */
