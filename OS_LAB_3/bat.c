#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>


static pthread_mutex_t lock;
static pthread_cond_t eastQueue,northQueue,westQueue,southQueue;
static int eastCounter,northCounter,westCounter,southCounter;
static pthread_cond_t eastFirst,northFirst,westFirst,southFirst;
static int batsCounter;
static pthread_cond_t deadlockFinished;

typedef struct {
    int num;
    char* dir;
}BAT;

void arrive(BAT b);

void cross(BAT b);

void leave(BAT b);

void check();

void * batRoutine(void *arg);

void init_vars();

void delete_vars();

char *validateInput(char *string);

void * batsManager (void * args){


    while (batsCounter > 0){
        //printf("currently we have %d BATs\n",batsCounter);
        check();
    }
}

void check() {

    pthread_mutex_lock(&lock);

    if (eastCounter > 0 && northCounter > 0 && westCounter > 0 && southCounter > 0){

        printf("DEADLOCK: BAT jam detected, signalling North to go\n");

        pthread_cond_signal(&westFirst);
        pthread_cond_wait(&deadlockFinished,&lock);

    }

    pthread_mutex_unlock(&lock);

}

void batsProgram (char * input){

    init_vars();


    pthread_t man;
    pthread_create(&man,NULL,batsManager,NULL); //creating manager thread that checks for deadlocks

    int numOfBats = (int) strlen(input);
    printf("number of BATs : %d\n",numOfBats);

    batsCounter = numOfBats;

    BAT bats[numOfBats];
    pthread_t bats_threads[numOfBats];

    for (int i = 0; i < numOfBats; ++i) {

        bats[i].num = i + 1;

        if(input[i] == 'e'){
            bats[i].dir = "EAST";
         } else if(input[i] == 'n') {
            bats[i].dir = "NORTH";
        }  else if(input[i] == 'w'){
            bats[i].dir = "WEST";
        } else { // input[i] == 's')
            bats[i].dir = "SOUTH";
        }

        pthread_create(&bats_threads[i], NULL, batRoutine, &bats[i]); //creating thread for each BAT


    }

    for (int j = 0; j < numOfBats; ++j) {

        pthread_join(bats_threads[j],NULL); // joining created threads

    }

    pthread_join(man,NULL);
    delete_vars();
}

void delete_vars() {

    pthread_mutex_destroy(&lock);

    pthread_cond_destroy(&eastQueue);
    pthread_cond_destroy(&northQueue);
    pthread_cond_destroy(&westQueue);
    pthread_cond_destroy(&southQueue);

    pthread_cond_destroy(&eastFirst);
    pthread_cond_destroy(&northFirst);
    pthread_cond_destroy(&westFirst);
    pthread_cond_destroy(&southFirst);


    pthread_cond_destroy(&deadlockFinished);

}

void init_vars() {

    eastCounter = northCounter = westCounter = southCounter = 0;

    pthread_mutex_init(&lock,NULL);

    pthread_cond_init(&eastQueue,NULL);
    pthread_cond_init(&northQueue,NULL);
    pthread_cond_init(&westQueue,NULL);
    pthread_cond_init(&southQueue,NULL);

    pthread_cond_init(&eastFirst,NULL);
    pthread_cond_init(&northFirst,NULL);
    pthread_cond_init(&westFirst,NULL);
    pthread_cond_init(&southFirst,NULL);

    pthread_cond_init(&deadlockFinished,NULL);

}

void * batRoutine(void *arg){

    BAT b;
    b = * (BAT *)arg;

    arrive(b);
    cross(b);
    leave(b);

}

void arrive(BAT b) {

    pthread_mutex_lock(&lock);

    printf("BAT %d from %s arrives at crossing\n", b.num, b.dir);

    // this function typically increments dir counter and checks
    // if there BATs came before me in my dir
    // if so then waits on my dir queue

    if (!strcmp(b.dir,"EAST")){
        ++eastCounter;

        if (eastCounter > 1)
            pthread_cond_wait(&eastQueue,&lock);

    } else if (!strcmp(b.dir,"NORTH")){
        ++northCounter;

        if(northCounter > 1)
            pthread_cond_wait(&northQueue,&lock);

    } else if (!strcmp(b.dir,"WEST")){
        ++westCounter;

        if(westCounter > 1)
            pthread_cond_wait(&westQueue,&lock);

    } else if (!strcmp(b.dir,"SOUTH")){
        ++southCounter;

        if(southCounter > 1)
            pthread_cond_wait(&southQueue,&lock);
    }

    pthread_mutex_unlock(&lock);
}

void cross(BAT b) {

    pthread_mutex_lock(&lock);

    // this function checks if there any BAT on my right
    // if so then waits on it
    // second i cross and decrement my dir counter
    // finally signaling BATs waiting in my dir queue

    if (!strcmp(b.dir,"EAST")){

        // checking if there bats on my right
        if (northCounter > 0)
            pthread_cond_wait(&northFirst,&lock);

        // crossing
        printf("BAT %d from %s crossing\n", b.num, b.dir);
        sleep(1); // it takes one second for a BAT to cross


    } else if (!strcmp(b.dir,"NORTH")){

        // checking if there bats on my right
        if(westCounter > 0)
            pthread_cond_wait(&westFirst,&lock);

        // crossing
        printf("BAT %d from %s crossing\n", b.num, b.dir);
        sleep(1); // it takes one second for a BAT to cross



    } else if (!strcmp(b.dir,"WEST")){

        // checking if there bats on my right
        if(southCounter > 0)
            pthread_cond_wait(&southFirst,&lock);

        // crossing
        printf("BAT %d from %s crossing\n", b.num, b.dir);
        sleep(1); // it takes one second for a BAT to cross



    } else if (!strcmp(b.dir,"SOUTH")){

        // checking if there bats on my right
        if(eastCounter > 0)
            pthread_cond_wait(&eastFirst,&lock);


        // crossing
        printf("BAT %d from %s crossing\n", b.num, b.dir);
        sleep(1); // it takes one second for a BAT to cross

    }

    pthread_mutex_unlock(&lock);
}

void leave(BAT b) {

    pthread_mutex_lock(&lock);

    printf("BAT %d from %s leaving crossing\n", b.num, b.dir);

    // this function signals awaiting BATS on my left

    if (!strcmp(b.dir,"EAST")){

        pthread_cond_signal(&eastFirst);


        // signaling east queue
        --eastCounter;
        pthread_cond_signal(&eastQueue);


    } else if (!strcmp(b.dir,"NORTH")){

        pthread_cond_signal(&northFirst);

        //signaling north queue
        --northCounter;
        pthread_cond_signal(&northQueue);


    } else if (!strcmp(b.dir,"WEST")){

        pthread_cond_signal(&westFirst);

        //signaling west queue
        --westCounter;
        pthread_cond_signal(&westQueue);

    } else if (!strcmp(b.dir,"SOUTH")){

        pthread_cond_signal(&southFirst);

        //signaling south queue
        --southCounter;
        pthread_cond_signal(&southQueue);

    }

    --batsCounter;
    pthread_cond_signal(&deadlockFinished);

    pthread_mutex_unlock(&lock);
}


int main(int argc, char * argv[]) {

    if (argc < 2){
        printf("no input found !!!\n");
        exit(0);
    }

    char * input = validateInput(argv[1]);

    printf("input after validation : %s\n",input);

    batsProgram(input);

    return 0;
}

char *validateInput(char *string) {

    int n = (int) strlen(string);

    int m = 0;

    int j = 0;

    for (int i = 0; i < n; ++i) {

        if (string[i] == 'e' || string[i] == 'n' || string[i] == 'w' || string[i] == 's')
            ++m;
    }

    char * realInput = malloc(sizeof(char) * m);


    for (int i = 0; i < n; ++i) {

        if (string[i] == 'e' || string[i] == 'n' || string[i] == 'w' || string[i] == 's') {
            realInput[j] = string[i];
            ++j;

        }
    }

    return realInput;
}
