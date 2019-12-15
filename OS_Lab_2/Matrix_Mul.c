#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define DELIM " \t\n"

static int *ptrA, *ptrB, *ptrC; /// global pointers to the three matrices

static unsigned int n,m,l; /** n: number of rows in first matrix which is also the number of rows in result Matrix
                               m: number of columns in first matrix which is also the number of rows in second Matrix
                               l: number of columns in second matrix which is also the number of columns in result Matrix
                            **/


static FILE * fStream;

static void procedure1();
static void * calculateElement(void *);

static void procedure2();
static void * calculateRow(void *);

static void initializeResultArray(); /// initializes result array with zeroes

int main(){

	char buffer[255];
	char * token;

	printf("Opening input file <MMinput.txt> .....\n\n");
	fStream = fopen("MMinput.txt","r");
	printf("..... input file opened\n\n");


	fgets(buffer,255,fStream);

	printf("parsing first matrix dimensions\n");
	token = strtok(buffer,DELIM);
	sscanf(token,"%d",&n);
	printf("n = %d\n",n);
	token = strtok(NULL,DELIM);
	sscanf(token,"%d",&m);
	printf("m = %d\n",m);

	int A[n][m]; /// declaring first Matrix

	printf("parsing first Matrix : \n");
	for (int i = 0; i < n ; ++i){
	    fgets(buffer,255,fStream);
	    token = strtok(buffer,DELIM);
	    printf("\t\t\t");
	    for(int j = 0; j < m;++j){
	        sscanf(token,"%d",&A[i][j]);
	        printf("%d\t",A[i][j]);
	        token = strtok(NULL,DELIM);
	    }
	    printf("\n\n");
	}

	printf("parsing second matrix dimensions\n");
	fgets(buffer,255,fStream);
	token = strtok(buffer,DELIM);
	int m2 ;	
	sscanf(token,"%d",&m2);
	
	if (m != m2){
		printf("\ninvalid matrices dimesions !!!\n\n");
		pthread_exit(NULL);
	}
	token = strtok(NULL,DELIM);
	sscanf(token,"%d",&l);
	printf("m = %d\n",m);
	printf("l = %d\n",l);


	int B[m][l],C[n][l]; /// declaring secand Matrix and Result Matrix

	printf("parsing second Matrix : \n");
	for(int j = 0; j < m; ++j){
	    fgets(buffer,255,fStream);
	    token = strtok(buffer,DELIM);
	    printf("\t\t\t");
	    for(int k = 0; k < l; ++k){
	        sscanf(token,"%d",&B[j][k]);
	        printf("%d\t",B[j][k]);
	        token = strtok(NULL,DELIM);
	    }
	    printf("\n\n");
	}


	fclose(fStream);

	printf("input file <MMinput.txt> closed\n\n");

	ptrA = &A[0][0];
	ptrB = &B[0][0];
	ptrC = &C[0][0];

	/**
	            global pointrs are now pointing to the three Matrices
	**/

	initializeResultArray(); /// initializing result Matrix

	procedure1(); /// applying procedure 1

	initializeResultArray(); /// initializing result Matrix

	procedure2(); /// applying procedure 2

	printf("\n\n\t\t*****program terminated successfully*****\n");

	return 0;
}


static void procedure1(){

    printf("\nMethod 1 started\n");

    struct timespec start, finish;

    printf("opening output file <MMoutput.txt> ..... \n\n");

    fStream = fopen("MMoutput.txt","w");

    printf("..... output file opened \n");

    pthread_t threads [n][l]; /// number of thread needed = number of elements in the result array

    int ik [n][l][2]; /// to send i & k for each thread

    printf("\nnow we are in threads .......... \n\n");

    clock_gettime(CLOCK_REALTIME, &start);

    for(int i = 0; i < n; ++i){
        for (int k = 0; k < l; ++k){

            ik[i][k][0] = i;
            ik[i][k][1] = k;

            pthread_create( &threads[i][k],NULL,calculateElement,&ik[i][k][0]); /// creating threads
        }
    }
    for (int i = 0; i < n; ++i){
        for (int k = 0; k < l; ++k)
            pthread_join(threads[i][k],NULL); /// joining created threads
    }

    printf("\n.......... threads ends now \n\n");

    clock_gettime(CLOCK_REALTIME, &finish);

    long seconds = finish.tv_sec - start.tv_sec;
    long ns = finish.tv_nsec - start.tv_nsec;

    if (start.tv_nsec > finish.tv_nsec) { // clock underflow
        --seconds;
        ns += 1000000000;
    }
    printf("elapsed time of procedure 1 in : \n");
    printf("\t\t- seconds without ns: %ld\n", seconds);
    printf("\t\t- nanoseconds: %ld\n", ns);
    printf("\t\t- total seconds: %e\n", (double)seconds + (double)ns/(double)1000000000);

    printf("\nHere is the Result Matrix:\n\n");
    for (int i = 0; i < n; ++i){
        printf("\t\t\t");
        for (int k = 0; k < l; ++k){
            printf("%d\t", *(ptrC + ((i*l)+k)));
            fprintf(fStream,"%d ",*(ptrC + ((i*l)+k))); /// printing result matrix to output file
        }
        printf("\n\n");
        fprintf(fStream,"\n");
    }

    fprintf(fStream,"END1\ttime elapsed in nano seconds: %ld\n",ns);

    printf("method 1 ended \n\n");
}


static void * calculateElement(void * ik){

    int *iptr,*kptr;
    iptr =  ik;
    kptr =  (ik+sizeof(int));

    int i = *iptr,k = *kptr;

    iptr = NULL,kptr = NULL;
    for (int j = 0; j < m; ++j){
        *(ptrC + ((i*l)+k)) += (*(ptrA +((i*m)+j))) *  (*(ptrB + ((j*l)+k)));/// C[i][k] += A[i][j] * b[j][k]
    }

    printf("Thread #%d : C[%d][%d] = %d \n",(i*l)+k,i,k,*(ptrC + ((i*l)+k))); /// for debugging

}

static void procedure2(){

    struct timespec start, finish;

    printf("\nMethod 2 started\n");

    pthread_t threads[n]; /// we need threads equal to number of rows of the result matrix

    int arg[n]; /// indeces i to be sent to threads

    printf("\nnow we are in threads ..........\n\n");

    clock_gettime(CLOCK_REALTIME, &start);

    for(int i = 0; i < n; ++i){
        arg[i] = i;
        pthread_create(&threads[i],NULL,calculateRow,&arg[i]); /// creating threads
    }

    for (int i = 0; i < n; ++i){
        pthread_join(threads[i],NULL); /// joining created threads
    }

    printf("\n.......... threads ends now \n\n");

    clock_gettime(CLOCK_REALTIME, &finish);

    long seconds = finish.tv_sec - start.tv_sec;
    long ns = finish.tv_nsec - start.tv_nsec;

    if (start.tv_nsec > finish.tv_nsec) { // clock underflow
        --seconds;
        ns += 1000000000;
    }
    printf("elapsed time of procedure 2 in : \n\n");
    printf("\t\t- seconds without ns: %ld\n", seconds);
    printf("\t\t- nanoseconds: %ld\n", ns);
    printf("\t\t- total seconds: %e\n", (double)seconds + (double)ns/(double)1000000000);

    printf("Here is the Result Matrix: \n\n");
    for (int i = 0; i < n; ++i){
        printf("\t\t\t");
        for (int k = 0; k < l; ++k){
            printf("%d\t", *(ptrC + ((i*l)+k)));
            fprintf(fStream,"%d ",*(ptrC + ((i*l)+k)));
        }
        printf("\n\n");
        fprintf(fStream, "\n");
    }

    fprintf(fStream,"END2\ttime elapsed in nano seconds: %ld\n",ns);

    printf("method 2 ended \n\n");

    printf("closing output file ..... \n");

    fclose(fStream);

    printf("output file <MMoutput> closed \n");

}

static void * calculateRow(void * arg){

    int * ptr;
    ptr = arg;
    int i  = *ptr;

    printf("Thread #%d : working at row #%d \n",i,i); /// for debugging

    ptr = NULL;

    for (int k = 0; k < l; ++k){
        for (int j = 0; j < m; ++j){
            *(ptrC + ((i*l)+k)) += (*(ptrA +((i*m)+j))) *  (*(ptrB + ((j*l)+k)));/// C[i][k] += A[i][j] * b[j][k]
        }
        printf("\tColumn #%d : C[%d][%d] = %d \n",k,i,k,*(ptrC + ((i*l)+k))); /// for debugging
    }
}


static void initializeResultArray(){

    printf("initializing result Matrix with zeroes : \n");
    for (int i = 0; i < n; ++i){
        printf("\t\t\t");
        for (int k = 0; k < l; ++k){
            *(ptrC + ((i*l)+k)) = 0; /// C[i][k] = 0
            printf("%d\t",*(ptrC + ((i*l)+k)));
        }
    printf("\n\n");
    }
}
