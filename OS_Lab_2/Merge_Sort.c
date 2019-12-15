#include <stdio.h>
#include <pthread.h>
#include <string.h>

#define PARAM 2
#define DELIM " /t/n"

static unsigned int n; /// size of the array
static int * ptrA; /// glopal pointer to the array to be sorted

void merge (int first, int mid, int last);

void * mergeSort(void * args);


int main(){

    FILE * fStream;
    char buffer[255];
    char * token;

    printf("Opening input file <MSinput.txt> .....\n\n");
    fStream = fopen("MSinput.txt","r");
    printf("..... input file opened\n\n");

    fgets(buffer,255,fStream); /// reading first line from input File

    printf("parsing array size ......\n");
    token = strtok(buffer,DELIM);
    sscanf(token,"%d",&n);
    printf("n = %d\n",n);

    if (n > 0){

        int A[n];

        /// parsing array elements
        fgets(buffer,255,fStream);
        token = strtok(buffer,DELIM);

        printf("Parsing array elements ...... \n\nArray before sorting : ");
        for (int i = 0; i < n; ++i){
            sscanf(token,"%d",&A[i]);
            printf("%d ",A[i]);
            token = strtok(NULL,DELIM);
        }

        printf("\n");

        fclose(fStream);
        printf("\ninput file <MSinput.txt> closed\n\n");

        ptrA = &A[0];

        struct timespec start, finish; /// for calculating elapsed time

        pthread_t t; /// major thread

        int args[PARAM]; /// major args to be passed to major thread

        args[0] = 0;
        args[1] = n - 1; /// thats mean that we want to sort the whole array

        printf("major thread started...... \n");

        clock_gettime(CLOCK_REALTIME, &start);

        pthread_create(&t,NULL,mergeSort,&args[0]);
        pthread_join(t,NULL);

        printf("\n...... major thread ended \n");
        printf ("\n\t\t\t ***** sorting done successfully *****\n\n");

        clock_gettime(CLOCK_REALTIME, &finish);

        long seconds = finish.tv_sec - start.tv_sec;
        long ns = finish.tv_nsec - start.tv_nsec;

        if (start.tv_nsec > finish.tv_nsec) { // clock underflow
            --seconds;
            ns += 1000000000;
        }
        printf("elapsed time in: \n");
        printf("\t\t - seconds without ns: %ld\n", seconds);
        printf("\t\t - nanoseconds: %ld\n", ns);
        printf("\t\t - total seconds: %e\n", (double)seconds + (double)ns/(double)1000000000);

        printf("opening output file <MSoutput.txt> ..... \n\n");

        fStream = fopen("MSoutput.txt","w");

        printf("..... output file opened \n");

        printf("\nArray after sorting : ");
        for(int i = 0; i < n; ++i){
            printf("%d ",A[i]);
            fprintf(fStream,"%d ",A[i]); /// printing sorted array to output file
        }
        printf("\n");

        fclose(fStream);
        printf("\noutput file <MSoutput> closed \n");

    }
    return 0;
}

void * mergeSort(void * args){

    int *lPtr, *rPtr;

    lPtr = args;
    rPtr = args + sizeof(int);

    int l = *lPtr, r = *rPtr;

    if( l < r ){

        pthread_t threads[PARAM]; /// two threads needed : one for left sub-problem and onother for right sub_problem

        int q = (l+r)/2;

        int lArgs[PARAM]; /// arguments to be passed to the left sub-problem
        lArgs[0] = l;
        lArgs[1] = q;

        /// creating thread for left sub-problem
        pthread_create(&threads[0],NULL,mergeSort,&lArgs[0]);

        int rArgs[PARAM]; /// arguments to be passed to the right sub-problem
        rArgs[0] = q+1;
        rArgs[1] = r;

        /// creating thread for right sub-problem
        pthread_create(&threads[1],NULL,mergeSort,&rArgs[0]);


        /// joining created threads
        pthread_join(threads[0],NULL);
        pthread_join(threads[1],NULL);


        /// merging left and right sub-problems
        merge(l,q,r);
    }



}
void merge(int first, int mid, int last){

    int n1 = mid - first + 1;
    int n2 = last - mid;

    /// declaring two temp arrays
    int L[n1],R[n2];

    /// copying array elements into temp arrays
    for (int i = 0; i < n1; ++i)
        L[i] = *(ptrA + first + i);
    for (int j = 0; j < n2; ++j)
        R[j] = *(ptrA + mid + 1 + j);


    int i = 0, j = 0, k = first;

    /// merging the two temp array back into original array
    while (i < n1  && j < n2){

        if (L[i] <= R[j]){
            *(ptrA + k) = L[i]; /// A[k] = L[i]
            ++i;
        } else {
            *(ptrA + k) = R[j]; /// A[k] = R[j]
            ++j;
        }

        ++k;
    }

    /// in case if we have remaining elements in left temp array
    while ( i < n1 ){
        *(ptrA + k) = L[i]; /// A[k] = L[i]
        ++i;
        ++k;
    }

    /// in case if we have remaining elements in right temp array
    while ( j < n2 ){
        *(ptrA + k) = R[j]; /// A[k] = R[j]
        ++j;
        ++k;
    }
}
