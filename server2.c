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
#include <ifaddrs.h>

#define PORTNUM "49999"    /* Port number for server */
#define BACKLOG 10
#define BUFSIZE 5


/**
 *
 * @param n :the size of the resulting vector
 * @param vector :the slice of matrix A from client2.c
 * @param matrix :matrix B from client2.c
 * @param cfd :the socket we use
 */
void multiply(long int n, const float vector[], float matrix[][n], int cfd){
    float result[n];
    for(int i = 0; i < n;i++){
        result[i] = 0;
    }
    for(int i = 0 ; i < n; i++){
        for(int j = 0 ; j < n;j++){
            result[i] += matrix[j][i] * vector[j];
        }
    }

    printf("\n");
    for(int i = 0 ; i < n;i++){
        printf("%.2f ,",result[i]);
    }

    send(cfd,result,sizeof(result),0);

}


int main(int argc, char *argv[])
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, PORTNUM, &hints, &result) != 0)
        exit(-1);

    int lfd, optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);

        if (lfd == -1)
            continue;   /* On error, try next address */

        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
                       &optval, sizeof(optval)) == -1)
            exit(-1);

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break; /* Success */

        /* bind() failed, close this socket, try next address */
        close(lfd);
    }

    if (rp == NULL)
        exit(-1);

    {
        char host[NI_MAXHOST];
        char service[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *)rp->ai_addr, rp->ai_addrlen,
                        host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
            fprintf(stdout, "Listening on (%s, %s)\n", host, service);
        else
            exit(-1);
    }

    freeaddrinfo(result);

    if (listen(lfd, BACKLOG) == -1)
        exit(-1);

    for (;;) /* Handle clients iteratively */
    {
        struct sockaddr_storage claddr;
        socklen_t addrlen = sizeof(struct sockaddr_storage);
        int cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
        if (cfd == -1) {
            continue;   /* Print an error message */
        }

        {
            char host[NI_MAXHOST];
            char service[NI_MAXSERV];
            if (getnameinfo((struct sockaddr *) &claddr, addrlen,
                            host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
                fprintf(stdout, "Connection from (%s, %s)\n", host, service);
            else
                fprintf(stderr, "Connection from (?UNKNOWN?)");
        }


        int n = 0;
        recv(cfd, &n, sizeof(n), 0);   //receiving the size of the result vector from master

        float matrixB[n][n];

        recv(cfd, matrixB, sizeof(matrixB), 0); //receiving matrixB

        printf("\n*RESULT VECTOR*\n");
        int counter = 0;
        while(counter != n ){
            float vector[n];            //Used to store matrixA slices as they are received
            recv(cfd, vector, sizeof(vector), 0);
            multiply(n,vector,matrixB,cfd);
            counter++;
        }
        printf("\n");
        if (close(cfd) == -1) /* Close connection */
        {
            fprintf(stderr, "close error.\n");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

}


