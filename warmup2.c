#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#include "cs402.h"
#include "my402list.h"
#include "warmup2.h"

/* global variables */

// program utility variables
char programName[MAXPATHLENGTH];
char errorMessage[1024];
struct timeval baseTime;
int debug = 0; 
extern int errno;

char* options[OPTIONS_LEN] = {};
int mode;
int STOP_SIGNAL = 0;

// program parameters
double LAMBDA = LAMBDA_INITIAL_VALUE;
double MU = MU_INITIAL_VALUE;
double R = R_INITIAL_VALUE; 
int B = B_INITIAL_VALUE; 
int P = P_INITIAL_VALUE; 
int N = N_INITIAL_VALUE;
char* TSFILE;

// thread variables
pthread_t packet_arrival_thread_id;
pthread_t token_arrival_thread_id;
pthread_t server_a_thread_id;
pthread_t server_b_thread_id;
pthread_t signal_handler_thread_id;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
sigset_t set;

// program data structures
My402List Q1;
My402List Q2;
My402List eventQ;
int TOKENS = 0;

// statistics state variables
double STATISTICS[11] = {0};
int STAT_TOTAL_PACKETS = 0;
int STAT_TOTAL_TOKENS = 0;
int STAT_DROPPED_TOKENS = 0;
int STAT_DROPPED_PACKETS = 0;


/* utility methods */

void setProgramName(char* path) {
    char* filename = strrchr(path, DIR_SEP);

    if (filename == NULL) {
		// direct filename given
        strcpy(programName, path);
    } else {
		// filename by skipping / prefix
        strcpy(programName, ++filename);
    }
}

void usage() {
    fprintf(stderr, "usage: %s [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] [-n num] [-t tsfile]\n", programName);
}

void reportError(char* message) {
	fprintf(stderr,"[Error]: %s\nExiting Program..\n", message);
	exit(-1);
}

double getValidFloat(char* floatStr) {

	int dotParsed = FALSE;
	for (int i = 0; i < strlen(floatStr); i++) {
		if(!isdigit(floatStr[i])) {
            if(floatStr[i] == '.' && dotParsed == TRUE) { 
				sprintf(errorMessage, "Invalid float: %s <too many decimal points> (valid: zero or one)", floatStr);
                reportError(errorMessage);
			}
			else if(floatStr[i] == '.' && dotParsed == FALSE) { 
				dotParsed = TRUE; 
			}
			else {
                sprintf(errorMessage, "Invalid float: %s <unknown character at position %d>", floatStr, i);
                reportError(errorMessage);
			}
		}
	}
	
	return atof(floatStr);
}

int getValidInt(char* intStr) {

	for (int i = 0; i < strlen(intStr); i++) {
		if(!isdigit(intStr[i])) {
            sprintf(errorMessage, "Invalid int: %s <unknown character at position %d>", intStr, i);
            reportError(errorMessage);
		}
	}
    long long tempInt = atoll(intStr);
	if(tempInt > MAX_INT) {
        sprintf(errorMessage, "Invalid int: %s <greater than max_int> (valid: <= 2147483647)", intStr);
		reportError(errorMessage);
	}
	
	return atoi(intStr);
}

double square(double x) {
    return x * x;
}

double getCurrentTime() {
    struct timeval currTime, res;
    gettimeofday(&currTime, 0);
    timersub(&currTime, &baseTime, &res);
    return res.tv_sec * 1000000 + res.tv_usec;
}

double timeSecToUsec(double secs) {
    if(secs > 10.0) secs = 10.0;
    double msec = (double)round(secs * 1000);
    double usec = msec * 1000;
    return usec;
}

double timeMsecToUsec(int msec) {
    double usec = msec * 1000;
    return usec;
}

double updateAvg(int n, double oldAvg, double point) {
    return (n * oldAvg + point) / (n + 1);
}

void updateStatistics(Packet* packet) {

    double time_in_Q1 = packet->packet_Q1_out_time - packet->packet_Q1_in_time;
    double time_in_Q2 = packet->packet_Q2_out_time - packet->packet_Q2_in_time;
    double time_in_S = packet->packet_S_out_time - packet->packet_S_in_time;
    double time_spent_in_system = packet->packet_S_out_time - packet->packet_arrival_time;

    STATISTICS[AVG_PACKET_SERVICE_TIME] = updateAvg(STAT_TOTAL_PACKETS, STATISTICS[AVG_PACKET_SERVICE_TIME], packet->mesured_service_time);
    STATISTICS[AVG_NO_OF_PACKET_IN_Q1] = STATISTICS[AVG_NO_OF_PACKET_IN_Q1] + time_in_Q1; // Normalization while printing
    STATISTICS[AVG_NO_OF_PACKET_IN_Q2] = STATISTICS[AVG_NO_OF_PACKET_IN_Q2] + time_in_Q2; // Normalization while printing

    if(packet->processing_server_id == 1) {
        STATISTICS[AVG_NO_OF_PACKET_AT_S1] = STATISTICS[AVG_NO_OF_PACKET_AT_S1] + time_in_S; // Normalization while printing
    }
    else if(packet->processing_server_id == 2) {
        STATISTICS[AVG_NO_OF_PACKET_AT_S2] = STATISTICS[AVG_NO_OF_PACKET_AT_S2] + time_in_S; // Normalization while printing
    }

    STATISTICS[AVG_TIME_IN_SYSTEM] = updateAvg(STAT_TOTAL_PACKETS, STATISTICS[AVG_TIME_IN_SYSTEM], time_spent_in_system);
    STATISTICS[AVG_TIME_IN_SYSTEM_SQUARE] = updateAvg(STAT_TOTAL_PACKETS, STATISTICS[AVG_TIME_IN_SYSTEM_SQUARE], square(time_spent_in_system));

    STAT_TOTAL_PACKETS += 1;

    free(packet);

    return;
}

FILE* getFileHandler(char* filepath) {
    FILE* F = fopen(filepath, "r");
    // check if file is a directory
    DIR* D = opendir(filepath);
    if(D != NULL){
        closedir(D);
        reportError("Error opening file: Given file path is a directory");
    }
    // check if file doesnt exists
    if(F == NULL) {
        if(errno == 20) {
            sprintf(errorMessage, "Error opening file: Permission Denied\n");
        }
        else {
            sprintf(errorMessage, "Error opening file: %s\n", strerror(errno));
        }
        reportError(errorMessage);
    }
    return F;
}

void printList(My402List* list) {

    for (My402ListElem* i = My402ListFirst(list); i != NULL; i = My402ListNext(list, i)) {
        Packet* packet = (Packet*) i->obj;

        printf("------\n");
        printf("index %d\n", packet->index);
        printf("inter_arrival_time %lf\n", packet->inter_arrival_time);
        printf("token_requirement %d\n", packet->token_requirement);
        printf("service_time %lf\n", packet->service_time);
    }
    
}

int getOptionIndex(char* option) {
    if(!strcmp(option, "-lambda")) return INDEX_OF_LAMBDA;
    if(!strcmp(option, "-mu")) return INDEX_OF_MU;
    if(!strcmp(option, "-r")) return INDEX_OF_R;
    if(!strcmp(option, "-B")) return INDEX_OF_B;
    if(!strcmp(option, "-P")) return INDEX_OF_P;
    if(!strcmp(option, "-n")) return INDEX_OF_N;
    if(!strcmp(option, "-t")) return INDEX_OF_TSFILE;
    return -1;
}

void processOptions(int argc, char** argv) {

    /* get options */

    int optionIndex;
    char* option;
    for (int i = 1; i < argc; i+=2)
    {
        option = argv[i];
        optionIndex = getOptionIndex(option);

        if(optionIndex != -1) {
            if(i+1 >= argc || argv[i+1][0] == '-') {
                usage();
                sprintf(errorMessage, "Option %s has a missing value", argv[i]);
                reportError(errorMessage);
            }
            options[optionIndex] = argv[i+1];
        } else {
            usage();
            sprintf(errorMessage, "Invalid option: <given: %s > (valid: [-lambda|-mu|-r|-B|-P|-n|-t])", option);
            reportError(errorMessage);
        }
    }

    /* update global parameters */

    if(options[INDEX_OF_TSFILE]) {
        mode = TRACE_DRIVEN_MODE;
        TSFILE = options[INDEX_OF_TSFILE];
    }
    else {
        mode = DETERMINISTC_MODE;

        if(options[INDEX_OF_LAMBDA]) LAMBDA = getValidFloat(options[INDEX_OF_LAMBDA]);
        if(options[INDEX_OF_MU]) MU = getValidFloat(options[INDEX_OF_MU]);
        if(options[INDEX_OF_P]) P = getValidInt(options[INDEX_OF_P]);
        if(options[INDEX_OF_N]) N = getValidInt(options[INDEX_OF_N]);
    }

    if(options[INDEX_OF_R]) R = getValidFloat(options[INDEX_OF_R]);
    if(options[INDEX_OF_B]) B = getValidInt(options[INDEX_OF_B]);

    /* print emulation parameters */

    printf("\n");
    printf("Emulation Parameters:\n");
    if(mode == DETERMINISTC_MODE) printf("\tlambda = %g\n", LAMBDA);
    if(mode == DETERMINISTC_MODE) printf("\tmu = %g\n", MU);
    printf("\tr = %g\n", R);
    printf("\tB = %d\n", B);
    if(mode == DETERMINISTC_MODE) printf("\tP = %d\n", P);
    if(mode == DETERMINISTC_MODE) printf("\tN = %d\n", N);
    if(mode == TRACE_DRIVEN_MODE) printf("\ttsfile = %s\n", TSFILE);
    printf("\n");

    // Dirty Fix
    if(N > 200000) N = 200000;
}

void* handlePacketArrivalThread(void* arg) {

    if(debug) printf("Packet Arrival Thread Start\n");

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);

    double prev_packet_start_time = 0;
    double prev_packet_end_time = 0;

    double curr_packet_start_time;
    double curr_packet_end_time;

    while(TRUE) {
        
        My402ListElem* elem = My402ListFirst(&eventQ);
        if(!elem) {
            if(debug) printf("Packet Arrival Thread Exit\n");    
            break;
        }
        Packet* packet = (Packet*) elem->obj;

        double sleepTime = (prev_packet_start_time + packet->inter_arrival_time) - prev_packet_end_time;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
        if(sleepTime > 0) usleep(sleepTime);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);

        pthread_mutex_lock(&mutex);

        if(STOP_SIGNAL) {
            pthread_mutex_unlock(&mutex);
            if(debug) printf("Packet Arrival Thread Exit (STOP SIGNAL)\n");    
            break;
        }

        curr_packet_start_time = getCurrentTime();
        packet->packet_arrival_time = curr_packet_start_time;

        double mesured_inter_arrival_time = curr_packet_start_time - prev_packet_start_time;
        STATISTICS[AVG_INTER_ARRIVAL_TIME] += mesured_inter_arrival_time;

        if(packet->token_requirement > B) {

			My402ListUnlink(&eventQ, elem);

            STAT_DROPPED_PACKETS += 1;

			printf(
                "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms, dropped\n", 
                packet->packet_arrival_time/1000, packet->index, packet->token_requirement, mesured_inter_arrival_time/1000
            );

            curr_packet_end_time = getCurrentTime();
            prev_packet_start_time = curr_packet_start_time;
            prev_packet_end_time = curr_packet_end_time;

			pthread_mutex_unlock(&mutex);
			free(packet);
			continue;
		}

        printf(
            "%012.3fms: p%d arrives, needs %d tokens, inter-arrival time = %.3fms\n",
			packet->packet_arrival_time/1000, packet->index, packet->token_requirement, mesured_inter_arrival_time/1000
        );

        My402ListAppend(&Q1, (void*) packet);
        packet->packet_Q1_in_time = getCurrentTime();

        printf(
            "%012.3fms: p%d enters Q1\n", 
            packet->packet_Q1_in_time/1000, packet->index
        );

        if(My402ListLength(&Q1) == 1) {
            
            My402ListElem* qHead = My402ListFirst(&Q1);
            Packet* packet = (Packet*)(qHead->obj);

    		if(packet->token_requirement <= TOKENS) {
    			
                TOKENS -= packet->token_requirement;                
    			My402ListUnlink(&Q1, qHead);
    			packet->packet_Q1_out_time = getCurrentTime();

                double token_in_Q1_time = packet->packet_Q1_out_time - packet->packet_Q1_in_time;
    			printf(
                    "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n",
    				packet->packet_Q1_out_time/1000, packet->index, token_in_Q1_time/1000, TOKENS
                );

    			int Q2_empty_before_append = My402ListEmpty(&Q2);
    			My402ListAppend(&Q2, (void*) packet);

    			packet->packet_Q2_in_time = getCurrentTime();

    			printf(
                    "%012.3fms: p%d enters Q2\n", 
                    packet->packet_Q2_in_time/1000, packet->index
                );

    			if(Q2_empty_before_append) {
    				pthread_cond_broadcast(&cv);                    
                }

    		}
        }

        curr_packet_end_time = getCurrentTime();
        prev_packet_start_time = curr_packet_start_time;
        prev_packet_end_time = curr_packet_end_time;

        My402ListUnlink(&eventQ, elem);
        
        pthread_mutex_unlock(&mutex);
    }

    return(0);
}

void* handleTokenArrivalThread(void* arg) {

    if(debug) printf("Token Arrival Thread Start\n");

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);

    double inter_token_arrival_time = timeSecToUsec(1.0/R); // usec (rounded by msec)

    double prev_token_start_time = 0;
    double prev_token_end_time = 0;

    double curr_token_start_time;
    double curr_token_end_time;

    int index = 1;

	while(TRUE) {

        double token_arrival_time;

        double sleepTime = (prev_token_start_time + inter_token_arrival_time) - prev_token_end_time;

        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
        if(sleepTime > 0) usleep(sleepTime);
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, 0);

        pthread_mutex_lock(&mutex);

        if(STOP_SIGNAL) {
            pthread_mutex_unlock(&mutex);
            if(debug) printf("Token Arrival Thread Exit (STOP SIGNAL)\n");
            break;
        }

        if(My402ListEmpty(&eventQ) && My402ListEmpty(&Q1)) {
            if(My402ListEmpty(&Q2)) pthread_cond_broadcast(&cv);
            pthread_mutex_unlock(&mutex);
            if(debug) printf("Token Arrival Thread Exit\n");
            break;
        }

        curr_token_start_time = getCurrentTime();
        token_arrival_time = curr_token_start_time;

        if(TOKENS < B) {
            TOKENS++;
            printf(
                "%012.3fms: token t%d arrives, token bucket now has %d token\n", 
                token_arrival_time/1000, index, TOKENS
            );
        }
        else {
            STAT_DROPPED_TOKENS++;
            printf(
                "%012.3fms: token t%d arrives, dropped\n", 
                token_arrival_time/1000, index
            );
        }

        if(!My402ListEmpty(&Q1)) {
            
            My402ListElem* elem = My402ListFirst(&Q1);
            Packet* packet = (Packet*)(elem->obj);

    		if(packet->token_requirement <= TOKENS) {
    			
                TOKENS -= packet->token_requirement;
    			My402ListUnlink(&Q1, elem);
    			packet->packet_Q1_out_time = getCurrentTime();

                double token_in_Q1_time = packet->packet_Q1_out_time - packet->packet_Q1_in_time;
    			printf(
                    "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token bucket now has %d token\n",
    				packet->packet_Q1_out_time/1000, packet->index, token_in_Q1_time/1000, TOKENS
                );


    			int Q2_empty_before_append = My402ListEmpty(&Q2);
    			My402ListAppend(&Q2, (void*) packet);

    			packet->packet_Q2_in_time = getCurrentTime();

    			printf(
                    "%012.3fms: p%d enters Q2\n", 
                    packet->packet_Q2_in_time/1000, packet->index
                );

    			if(Q2_empty_before_append) {
    				pthread_cond_broadcast(&cv);                    
                }

    		}
        }

        curr_token_end_time = getCurrentTime();
        prev_token_start_time = curr_token_start_time;
        prev_token_end_time = curr_token_end_time;

        pthread_mutex_unlock(&mutex);

        index++;
        STAT_TOTAL_TOKENS++;
	}

    return(0);
}

void* handleServerThread(void* server_id) {

    if(debug) printf("Server %d Thread Start\n", (int)server_id + 1);

	while(TRUE) { 

	    pthread_mutex_lock(&mutex);

		while(My402ListEmpty(&Q2)) {
            if(My402ListEmpty(&eventQ) && My402ListEmpty(&Q1)) {
                if(debug) printf("Server %d Thread Exit\n", (int)server_id + 1);
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&cv, &mutex);
            if(STOP_SIGNAL) {
                break;
            }
        }

        if(STOP_SIGNAL) {
            while (!My402ListEmpty(&Q1)) {
                My402ListElem* elem = My402ListFirst(&Q1);
                Packet* packet = (Packet*)(elem->obj);
                printf(
                    "%012.3fms: p%d removed from Q1\n", 
                    getCurrentTime()/1000, packet->index
                );
                My402ListUnlink(&Q1, elem);
                free(packet);
            }
            while (!My402ListEmpty(&Q2)) {
                My402ListElem* elem = My402ListFirst(&Q2);
                Packet* packet = (Packet*)(elem->obj);
                printf(
                    "%012.3fms: p%d removed from Q2\n", 
                    getCurrentTime()/1000, packet->index
                );
                My402ListUnlink(&Q2, elem);
                free(packet);
            }
            while (!My402ListEmpty(&eventQ)) {
                My402ListElem* elem = My402ListFirst(&eventQ);
                Packet* packet = (Packet*)(elem->obj);
                My402ListUnlink(&eventQ, elem);
                free(packet);
            }
            if(debug) printf("Server %d Thread Exit (STOP_SIGNAL)\n", (int)server_id + 1);
            pthread_mutex_unlock(&mutex);
            pthread_exit(0);
        }
        
        My402ListElem* elem = My402ListFirst(&Q2);
        Packet* packet = (Packet*)(elem->obj);

        packet->processing_server_id = (int) server_id + 1;

        My402ListUnlink(&Q2, elem);

        pthread_mutex_unlock(&mutex);

        double currentTime = getCurrentTime();

        packet->packet_Q2_out_time = currentTime;
        double token_in_Q2_time = packet->packet_Q2_out_time - packet->packet_Q2_in_time;

        printf(
            "%012.3fms: p%d leaves Q2, time in Q2 = %.3fms\n",
            packet->packet_Q2_out_time/1000, packet->index, token_in_Q2_time/1000
        );

        packet->packet_S_in_time = currentTime;

        printf(
            "%012.3fms: p%d begins service at S%d, requesting %dms of service\n",
            packet->packet_S_in_time/1000, packet->index, packet->processing_server_id, (int)packet->service_time/1000
        );

        usleep(packet->service_time);

        packet->packet_S_out_time = getCurrentTime();

        packet->mesured_service_time = packet->packet_S_out_time - packet->packet_S_in_time;
        double time_in_system = packet->packet_S_out_time - packet->packet_arrival_time;

        printf(
            "%012.3fms: p%d departs from S%d, service time = %.3fms, time in system = %.3fms\n",
            packet->packet_S_out_time/1000, packet->index, packet->processing_server_id, packet->mesured_service_time/1000, time_in_system/1000
        );

        updateStatistics(packet);
    }

    return(0);
}

void* handleSignalThread(void* arg) {
    int sig;
    sigwait(&set, &sig);
    if(sig == SIGINT) {
        printf("SIGINT caught, no new packets or tokens will be allowed\n");
    }
    pthread_mutex_lock(&mutex);
    STOP_SIGNAL = 1;
    pthread_cancel(packet_arrival_thread_id);
    pthread_cancel(token_arrival_thread_id);
    pthread_cond_broadcast(&cv);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void setupSimulation() {

    memset(&Q1, 0, sizeof(My402List));
	My402ListInit(&Q1);
	
    memset(&Q2, 0, sizeof(My402List));
	My402ListInit(&Q2);

    memset(&eventQ, 0, sizeof(My402List));
	My402ListInit(&eventQ);

    Packet* packet;
    double inter_arrival_time = 0;
    double service_time = 0;
    int token_requirement = 0;


    if(mode == DETERMINISTC_MODE) {
        for (int i = 0; i < N; i++) {

            packet = (Packet*) malloc(sizeof(Packet));
            
            // sec -> usec
            inter_arrival_time = timeSecToUsec(1.0/LAMBDA); // usec (rounded by msec)
            service_time = timeSecToUsec(1.0/MU); // usec (rounded by msec)
            token_requirement = P;
            
            // create packet
            packet->index = i + 1;
            packet->inter_arrival_time = inter_arrival_time;
            packet->service_time = service_time;
            packet->token_requirement = token_requirement;
            
            packet->packet_Q1_in_time = 0.0;
            packet->packet_Q1_out_time = 0.0;
            packet->packet_Q2_in_time = 0.0;
            packet->packet_Q2_out_time = 0.0;
            packet->packet_S_in_time = 0.0;
            packet->packet_S_out_time = 0.0;

            My402ListAppend(&eventQ, (void*)packet);
        }    
        // printList(&eventQ);
    }
    else if(mode == TRACE_DRIVEN_MODE) {

        FILE* F = getFileHandler(TSFILE);
        char line[1024];

        if(fgets(line, sizeof(line), F) == NULL) {
            sprintf(errorMessage, "File %s is empty!", TSFILE);
            reportError(errorMessage);
        }
        line[strlen(line) - 1] = '\0';
        N = getValidInt(line);

        for (int i = 0; i < N; i++) {

            if(fgets(line, sizeof(line), F) != NULL) {

                if (sscanf(line, "%lf %d %lf", &inter_arrival_time, &token_requirement, &service_time) == 3) {
                    
                    packet = (Packet*) malloc(sizeof(Packet));            

                    // msec -> usec
                    inter_arrival_time = timeMsecToUsec(inter_arrival_time);
                    service_time = timeMsecToUsec(service_time);

                    // create packet
                    packet->index = i + 1;
                    packet->inter_arrival_time = inter_arrival_time;
                    packet->service_time = service_time;
                    packet->token_requirement = token_requirement;

                    My402ListAppend(&eventQ, (void*)packet);
                }
                else {
                    sprintf(errorMessage, "Cannot scan 3 integers as expected at line index %d", i);
                    reportError(errorMessage);
                }
            }
            else {
                sprintf(errorMessage, "File %s has a line missing <have: %d>(expected: %d lines)", TSFILE, i+1, N);
                reportError(errorMessage);
            }
        }
        
        fclose(F);
        // printList(&eventQ);
    }

}

void displayStatistics(double total_emulation_time) {

    STATISTICS[AVG_INTER_ARRIVAL_TIME] = STATISTICS[AVG_INTER_ARRIVAL_TIME] / (STAT_DROPPED_PACKETS + STAT_TOTAL_PACKETS);
    STATISTICS[TOKEN_DROP_PROBABLITY] = (double) STAT_DROPPED_TOKENS / (double) STAT_TOTAL_TOKENS;
    STATISTICS[PACKET_DROP_PROBABLITY] = (double) STAT_DROPPED_PACKETS / (double) N;
    STATISTICS[AVG_NO_OF_PACKET_IN_Q1] = STATISTICS[AVG_NO_OF_PACKET_IN_Q1] / total_emulation_time; 
    STATISTICS[AVG_NO_OF_PACKET_IN_Q2] = STATISTICS[AVG_NO_OF_PACKET_IN_Q2] / total_emulation_time; 
    STATISTICS[AVG_NO_OF_PACKET_AT_S1] = STATISTICS[AVG_NO_OF_PACKET_AT_S1] / total_emulation_time; 
    STATISTICS[AVG_NO_OF_PACKET_AT_S2] = STATISTICS[AVG_NO_OF_PACKET_AT_S2] / total_emulation_time; 
    STATISTICS[STD_TIME_IN_SYSTEM] = sqrt( STATISTICS[AVG_TIME_IN_SYSTEM_SQUARE] - square(STATISTICS[AVG_TIME_IN_SYSTEM]));

    printf("\nStatistics:\n");
	printf("\n\taverage packet inter-arrival time = %.7gs\n", STATISTICS[AVG_INTER_ARRIVAL_TIME] / 1000000);
	printf("\taverage packet service time = %.7gs\n", STATISTICS[AVG_PACKET_SERVICE_TIME] / 1000000);

	printf("\n\taverage number of packets in Q1 = %.7g\n", STATISTICS[AVG_NO_OF_PACKET_IN_Q1]);
	printf("\taverage number of packets in Q2 = %.7g\n", STATISTICS[AVG_NO_OF_PACKET_IN_Q2]);
	printf("\taverage number of packets in S1 = %.7g\n", STATISTICS[AVG_NO_OF_PACKET_AT_S1]);
	printf("\taverage number of packets in S2 = %.7g\n", STATISTICS[AVG_NO_OF_PACKET_AT_S2]);

	printf("\n\taverage time a packet spent in system = %.7gs\n", STATISTICS[AVG_TIME_IN_SYSTEM] / 1000000);
	printf("\tstandard deviation for time spent in system = %.7gs\n", STATISTICS[STD_TIME_IN_SYSTEM] / 1000000);

	printf("\n\ttoken drop probability = %.7g\n", STATISTICS[TOKEN_DROP_PROBABLITY]);
	printf("\tpacket drop probability = %.7g\n", STATISTICS[PACKET_DROP_PROBABLITY]);
    printf("\n");
    return;
}

void startSimulation() {

    gettimeofday(&baseTime, 0);

	printf("%012.3fms: emulation begins\n", (double)0);

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, 0);

    /* creating worker threads */

    pthread_create(&packet_arrival_thread_id, 0, handlePacketArrivalThread, 0);
    pthread_create(&token_arrival_thread_id, 0, handleTokenArrivalThread, 0);
    pthread_create(&server_a_thread_id, 0, handleServerThread, (void*)SERVER_A);
    pthread_create(&server_b_thread_id, 0, handleServerThread, (void*)SERVER_B);
    pthread_create(&signal_handler_thread_id, 0, handleSignalThread, 0);

    /* joining all threads */

    pthread_join(packet_arrival_thread_id, 0);
    pthread_join(token_arrival_thread_id, 0);
    pthread_join(server_a_thread_id, 0);
    pthread_join(server_b_thread_id, 0);

    double emulation_ends = getCurrentTime();

    printf("%012.3fms: emulation ends\n", emulation_ends / 1000);

    displayStatistics(emulation_ends);
}

int main(int argc, char** argv) {

    if(argc) setProgramName(argv[0]);
    processOptions(argc, argv);
    setupSimulation();
    startSimulation();

    return 0;
}