#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/time.h>

#include "cs402.h"
#include "my402list.h"
#include "warmup2.h"

/* global variables */

// program utility variables
char programName[MAXPATHLENGTH];
char errorMessage[1024];
struct timeval timer;

char* options[OPTIONS_LEN] = {};
int mode;

// program parameters
double LAMBDA = LAMBDA_INITIAL_VALUE;
double MU = MU_INITIAL_VALUE;
double R = R_INITIAL_VALUE; 
int B = B_INITIAL_VALUE; 
int P = P_INITIAL_VALUE; 
int N = N_INITIAL_VALUE;
char* TSFILE;

// thread identifiers
pthread_t packet_arrival_thread_id;
pthread_t token_arrival_thread_id;
pthread_t server_a_thread_id;
pthread_t server_b_thread_id;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// program data structures
My402List Q1;
My402List Q2;
My402List eventQ;
int tokens;

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

double getValidFloat(char* floatStr) {

	int dotParsed = FALSE;
	for (int i = 0; i < strlen(floatStr); i++) {
		if(!isdigit(floatStr[i])) {
            if(floatStr[i] == '.' && dotParsed == TRUE) { 
				sprintf(errorMessage, "Invalid float: %s <too many decimal points> (valid: exactly one '.')", floatStr);
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
	if(dotParsed == 0) {
        sprintf(errorMessage, "Invalid float: %s <no decimal point> (valid: exactly one '.')", floatStr);
		reportError(errorMessage);
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

void processOptions(int argc, char** argv) {

    /* get options */

    int optionIndex;
    char* option;
    for (int i = 1; i < argc; i+=2)
    {
        if(i+1 >= argc || argv[i+1][0] == '-') {
            usage();
            sprintf(errorMessage, "Option %s has a missing value", argv[i]);
            reportError(errorMessage);
        }
        option = argv[i];
        optionIndex = getOptionIndex(option);

        if(optionIndex != -1) {
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

    printf("Emulation Parameters:\n");
    if(mode == DETERMINISTC_MODE) printf("\tlambda = %g\n", LAMBDA);
    if(mode == DETERMINISTC_MODE) printf("\tmu = %g\n", MU);
    printf("\tr = %g\n", R);
    printf("\tB = %d\n", B);
    if(mode == DETERMINISTC_MODE) printf("\tP = %d\n", P);
    if(mode == TRACE_DRIVEN_MODE) printf("\ttsfile = %s\n", TSFILE);

}

void* handlePacketArrivalThread(void* arg) {

    printf("In Packet Arrival Thread\n");
    int packet_arrival_time = gettimeofday(&timer, NULL);

    // for (My402ListElem* elem = My402ListFirst(eventQ); elem != NULL; elem = My402ListNext(eventQ, elem)) {
        
    //     Packet* packet = (Packet*) elem->obj;

    //     packet->packet_arrival_time = packet_arrival_time;

    //     int time_to_sleep = packet->packet_arrival_time + packet->inter_arrival_time;

    //     usleep(time_to_sleep);

    //     pthread_mutex_lock(&mutex);

    //     My402ListAppend(&Q1, (void*) packet);

    //     pthread_cond_broadcast(&cv);

    //     pthread_mutex_unlock(&mutex);

    // }


    return(0);
}

void* handleTokenArrivalThread(void* arg) {

    printf("In Token Arrival Thread\n");

    return(0);
}

void* handleServerThread(void* arg) {

    int server = (int) arg;

    if(server == SERVER_A) {
        printf("In Server A Thread\n");
    }
    else if(server == SERVER_B) {
        printf("In Server B Thread\n");
    }

    return(0);
}

FILE* getFileHandler(char* filepath) {
    FILE* F = fopen(filepath, "r");
    // check if file is a directory
    DIR* D = opendir(filepath);
    if(D != NULL){
        closedir(D);
        reportError("Given file path is a directory");
    }
    // check if file doesnt exists
    if(F == NULL) {
        reportError("Unable to open file");
    }
    return F;
}

int timeSecToUsec(double secs) {
    if(secs > 10.0) secs = 10.0;
    int msec = (int) (secs * 1000);
    int usec = msec * 1000;
    return usec;
}

int timeMsecToUsec(int msec) {
    if(msec > 10000) msec = 10000;
    int usec = msec * 1000;
    return usec;
}

void printList(My402List* list) {

    for (My402ListElem* i = My402ListFirst(list); i != NULL; i = My402ListNext(list, i)) {
        Packet* packet = (Packet*) i->obj;

        printf("------\n");
        printf("index %d\n", packet->index);
        printf("inter_arrival_time %d\n", packet->inter_arrival_time);
        printf("token_requirement %d\n", packet->token_requirement);
        printf("service_time %d\n", packet->service_time);
    }
    
}

void setupSimulation() {

    memset(&Q1, 0, sizeof(My402List));
	My402ListInit(&Q1);
	
    memset(&Q2, 0, sizeof(My402List));
	My402ListInit(&Q2);

    memset(&eventQ, 0, sizeof(My402List));
	My402ListInit(&eventQ);

    Packet* packet;
    int inter_arrival_time = 0;
    int service_time = 0;
    int token_requirement = 0;


    if(mode == DETERMINISTC_MODE) {
        for (int i = 0; i < N; i++) {

            packet = (Packet*) malloc(sizeof(Packet));
            
            // sec -> usec
            inter_arrival_time = timeSecToUsec(1.0/LAMBDA); // usec (rounded by msec)
            service_time = timeSecToUsec(1.0/MU); // usec (rounded by msec)
            token_requirement = P;
            
            // create packet
            packet->index = i;
            packet->inter_arrival_time = inter_arrival_time;
            packet->service_time = service_time;
            packet->token_requirement = token_requirement;

            My402ListAppend(&eventQ, (void*)packet);
        }    
        // printList(&eventQ);
    }
    else if(mode == TRACE_DRIVEN_MODE) {

        FILE* F = getFileHandler(TSFILE);
        char line[1024];
        int N;

        if(fgets(line, sizeof(line), F) == NULL) {
            sprintf(errorMessage, "File %s is empty!", TSFILE);
            reportError(errorMessage);
        }
        line[strlen(line) - 1] = '\0';
        N = getValidInt(line);

        for (int i = 0; i < N; i++) {

            if(fgets(line, sizeof(line), F) != NULL) {

                if (sscanf(line, "%d %d %d", &inter_arrival_time, &token_requirement, &service_time) == 3) {
                    
                    packet = (Packet*) malloc(sizeof(Packet));            

                    // msec -> usec
                    inter_arrival_time = timeMsecToUsec(inter_arrival_time);
                    service_time = timeMsecToUsec(service_time);

                    // create packet
                    packet->index = i;
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

void startSimulation() {

    /* creating worker threads */

    pthread_create(&packet_arrival_thread_id, 0, handlePacketArrivalThread, 0);
    pthread_create(&token_arrival_thread_id, 0, handleTokenArrivalThread, 0);
    pthread_create(&server_a_thread_id, 0, handleServerThread, (void*)SERVER_A);
    pthread_create(&server_b_thread_id, 0, handleServerThread, (void*)SERVER_B);

    /* joining all threads */

    pthread_join(packet_arrival_thread_id, 0);
    pthread_join(token_arrival_thread_id, 0);
    pthread_join(server_a_thread_id, 0);
    pthread_join(server_b_thread_id, 0);

}

int main(int argc, char** argv) {

    if(argc) setProgramName(argv[0]);
    processOptions(argc, argv);
    setupSimulation();
    // startSimulation();

    return 0;
}