#define _DEFAULT_SOURCE /* For NI_MAXHOST and NI_MAXSERV */
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#define PORTNUM "49999"    /* Port number for server */

/**
 * used to assign dimensions of matrix
 */
int init_matix(){
    printf("Please input the dimensions you want in the matrix \nWhere n creates an n x n matrix: ");
    int n;
    scanf("%d",&n);
    return n;
}

/**
 * used to assign number of processes
 */
int innit_number_of_process(int matrix_size){
    int control = 0;
    int n = 0;
    while(control == 0) {
        printf("Please input the number of processes: ");
        scanf("%d", &n);

        if(matrix_size % n != 0){
            printf("Invalid number of processes, the processes need to be a multiple of %d\n",n);
            continue;
        }
        control++;
    }
    return n;
}

/**
 *  this is used for when we are sending and receiving the result vector.
 * @param tracker               the index of matrix A that shoul be sent over
 * @param number_of_processes   number of steps allowed for each run of the function
 * @param matrix_size           size of matrixA
 * @param matrixA
 * @param cfd                   socket we use to communicate with worker
 */
void handleProcess(int tracker,int number_of_processes,int matrix_size,float matrixA[][matrix_size],int cfd){


    for (int i = tracker; i < number_of_processes; i++) {
        send(cfd, matrixA[i], sizeof(matrixA[i]), 0);
        float vector1[matrix_size];

        for (int k = 0; k < matrix_size; k++) {
            vector1[k] = 0;
        }
        recv(cfd, &vector1, sizeof(vector1), 0);

        for (int p = 0; p < matrix_size; p++) {
            printf("%.2f ", vector1[p]);
        }
        printf("\n");
    }
}

int main(int argc, char *argv[]) {

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    if (getaddrinfo(argv[1], PORTNUM, &hints, &result) != 0)
        exit(-1);

    int cfd;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        cfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);

        if (cfd == -1)
            continue;   /* On error, try next address */

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        /* close() failed, close this socket, try next address */
        close(cfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "No socket to bind...\n");
        exit(-1);
    }

    freeaddrinfo(result);

    {
        /***
         * initialising the two matrices with random numbers
         */
        int matrix_size = init_matix();
        int number_of_processes = innit_number_of_process(matrix_size);
        printf("%d \n", number_of_processes);

        send(cfd, &matrix_size, sizeof(matrix_size), 0);
        printf("MATRIX A\n");
        float matrixA[matrix_size][matrix_size];

        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                matrixA[i][j] = rand() % 20 ;
            }
        }

        float matrixB[matrix_size][matrix_size];
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                matrixB[i][j] = rand() % 20;
            }
        }

        //printing matrixA
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                printf("%.2f ,", matrixA[i][j]);
            }
            printf("\n");
        }

        printf("\nMATRIX B\n");
        //printing matrixB
        for (int i = 0; i < matrix_size; i++) {
            for (int j = 0; j < matrix_size; j++) {
                printf("%.2f , ", matrixB[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        send(cfd, matrixB, sizeof(matrixB), 0);

        printf("MATRIX C \n");

        int i = 0; //for iteratring up to number of processes
        int tracker = 0; //index of where the handleProcess function should start
        int jumps = matrix_size/number_of_processes; //how many steps a function is allowed to take before iteration of while loop
        while (i < number_of_processes ) {
            int pid = fork();
            if (pid != 0) {    //child processes only
                if(number_of_processes == matrix_size){
                    handleProcess(i,i + 1,matrix_size,matrixA,cfd);
                    exit(1);
                }

                if(number_of_processes == 1){
                    handleProcess(0,matrix_size,matrix_size,matrixA,cfd);
                    exit(100);
                }else{
                    handleProcess(tracker,jumps,matrix_size,matrixA,cfd);
                    exit(100);
                }
            }
            tracker += matrix_size/number_of_processes;
            jumps+=matrix_size/number_of_processes;
            i++;
        }
    }
    exit(EXIT_SUCCESS);
}

