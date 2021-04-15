#include "client.h" 

time_t startTime;
char* public_pipe;
static int global_id = 0;
pthread_mutex_t access_public_pipe = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_id = PTHREAD_MUTEX_INITIALIZER;


/**
 * Make a request to Servidor
 * Probably need to pass the struct request throw argument and store it in public_pipe (putting it in public_pipe is making a request)
 */ 
void make_request(Message msg) {
    pthread_mutex_lock(&access_public_pipe);        
    
    // Writing the task Client want Servidor to perform
    int fd = open(public_pipe, O_WRONLY);  
    if (fd < 0) {
        // SERVIDOR FECHOU A FIFO, GUARDAR INFORMACAO
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        register_op(msg, IWANT);
        write(fd, &msg, sizeof(msg)); // waits...
    }

    close(fd);

    /* PROCESS MESSAGE, WRITE THAT SENT A REQUEST, ETC */
    pthread_mutex_unlock(&access_public_pipe);
}

/**
 * Get response from a request. 
 * Probably need to return a struct request, read from the private pipe (idk how to read it from pipe, how does the output of the request come?)
 */ 
Message get_response() {
    char private_pipe[50];
    Message response;

    snprintf(private_pipe, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());

    // Writing the task Client want Servidor to perform
    int fd = open(private_pipe, O_RDONLY);
    if (fd < 0) {
        fprintf(stdout, "Could not open public_pipe\n");
    } else {
        read(fd, &response, sizeof(response));
        register_op(response, GOTRS);
    }

    close(fd);
    
    /* PROCESS Message, WRITE THAT RECEIVED THE RETURN OF THE REQUEST, ETC */
    return response;
}

/**
 * Func to create client threads. It creates a private fifo to communicate with Servidor
 */
void *client_thread_func(void * argument) {
    
    // generate number between 1 and 9
    int upper = 9, lower = 1;
    srand((unsigned) pthread_self());
    int num = (rand() % (upper - lower + 1)) + lower;

    Message order;
    order.tskload = num;
    order.pid = getpid();
    order.tid = pthread_self();
    order.tskres = DEFAULT_CLIENT_RESULT;

    pthread_mutex_lock(&control_id);
    order.rid = global_id;
    global_id++;
    pthread_mutex_unlock(&control_id);

    char private_fifo[50];
    snprintf(private_fifo, 50, "/tmp/%d.%lu", getpid(), (unsigned long) pthread_self());
    if (mkfifo(private_fifo, 0666) < 0){
        fprintf(stderr, "mkfifo()");
    }     // private channel

    fprintf(stdout, "%d %d %d %lu %d\n", order.rid, order.tskload, order.pid, order.tid, order.tskres);

    make_request(order);

    Message response = get_response();   
    
    if(response.tskres == -1) {
        register_op(response, CLOSD);
        // terminate the program
    }
    // DEBUG
    //fprintf(stdout, "Message:%d %d %d %lu %d\n", response.rid, response.tskload, response.pid, response.tid, response.tskres);
    
    if (remove(private_fifo) != 0){
        fprintf(stderr, "remove(private_fifo)\n");
    } 
    
    return(NULL);
}

int parse_args(int argc, char* argv[], int *inputTime) {
    if (argc != 4) return 1;
    if (strncmp(argv[1], "-t", 2) != 0) return 1;

    if (argv[2] <= 0) {
        return 1;
    } else {
        *inputTime = atoi(argv[2]);
    }

    if (mkfifo(argv[3], 0666) < 0) return 1;     // public channel
    return 0;
}

int main(int argc, char* argv[]) {
    int inputTime;
    parse_args(argc, argv, &inputTime);
    /*if (parse_args(argc, argv) != 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return 1;
    }*/

    time(&startTime);
    
    srand((unsigned) startTime);
    int upper = 9, lower = 1;
    int creationSleep = (rand() % (upper - lower + 1)) + lower;

    // time = 5, num = 1, max 50 threads (gave 10 + 1 to make sure it doesnt pass it)
    int maxNThreads = (inputTime / creationSleep) * (10 + 1);      
	
    pthread_t *ptid;
    ptid = (pthread_t *) malloc(maxNThreads * sizeof(pthread_t));
    public_pipe = argv[3];
    
    fprintf(stdout, "NUM: %d\n", creationSleep);
    // generate number between 1 and 9

    int numThreads = 0;
    for (; ; numThreads++){
        time_t currTime;
        time(&currTime);
        usleep(creationSleep*pow(10, 5));

        if ((unsigned) (currTime - startTime) >= inputTime) 
            break;
        
        pthread_create(&ptid[numThreads], NULL, client_thread_func, NULL);
    }

    for (int i = 0; i < numThreads; i++){
        pthread_join(ptid[i],NULL);
    }

    return 0;
}