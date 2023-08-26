#include <unistd.h>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

struct ThreadArgs {
    int argc;
    char** argv;
    std::string s;
    int id;
    std::string rleS;
    std::vector<int> freq;
};

void* threadSock(void* arg_ptr) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg_ptr);
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = atoi(args->argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR opening socket";
        exit(1);
    }
    server = gethostbyname(args->argv[1]);
    if (server == NULL) {
        std::cerr << "ERROR, no such host\n";
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "ERROR connecting";
        exit(1);
    }

    // TODO: Send data to the server and receive the result

    std::string message=args->s;
    int msgsize = message.length()+1;
    char *msg = new char[msgsize];
    strcpy(msg, message.c_str());


    //writing each msgsize
    n = write(sockfd,&msgsize,sizeof(int));
    if (n < 0) 
    {
        std::cerr << "ERROR writing msgsize socket" << std::endl;
        exit(0);
    }

    //writing actual msg
    n = write(sockfd, msg, msgsize);
    if (n < 0) 
    {
        std::cerr << "ERROR writing msg socket" << std::endl;
        exit(0);
    }

    //read initial string size
    int initial_string_size;
    n = read(sockfd, &initial_string_size, sizeof(int));
    if (n < 0){
        std::cerr << "Error reading the initial string's size from socket" << std::endl;
        exit(0);
    }

    char *initial_string = new char[initial_string_size];
    bzero(initial_string, initial_string_size);

    //read the actual initial string 
    n = read(sockfd, initial_string, initial_string_size);
    if (n < 0)
    {
        std::cerr << "Error reading the inital string from socket" << std::endl;
        exit(0);
    }
    
    //args->s=std::string(initial_string) ;

    //read the rlssting size
    int rle_string_size;
    n = read(sockfd, &rle_string_size, sizeof(int));
    if (n < 0){
        std::cerr << "Error reading rle string size from socket" << std::endl;
        exit(0);
    }

    char* rlestring = new char[rle_string_size];
    bzero(rlestring, rle_string_size);

    //read the actual rlestring
    n=read(sockfd, rlestring, rle_string_size);
    if (n < 0){
        std::cerr << "Error reading rle string from socket" << std::endl;
        exit(0);
    }

    args->rleS=std::string(rlestring);

    //read the freq size
    // read the freq size
    int freq_size;
    n = read(sockfd, &freq_size, sizeof(int));
    if (n < 0) {
        std::cerr << "Error reading freq size from socket" << std::endl;
        exit(0);
    }

    int* freq = new int[freq_size];

    // read the freq
    n = read(sockfd, freq, freq_size * sizeof(int));
    if (n < 0) {
        std::cerr << "Error reading freq from socket" << std::endl;
        exit(0);
    }

    // Store freq in a vector for easier access
    std::vector<int> freqVector(freq, freq + freq_size);
    args->freq=freqVector;




    close(sockfd);

    return NULL;
}

int main(int argc, char *argv[]) {
    std::string input;
    std::vector<std::string> inputs;

    // This grabs all the inputs so we can create the correct number of threads
    while (std::cin >> input) {
        inputs.push_back(input);
    }

    int nthreads = inputs.size();
    pthread_t tid[nthreads];
    static ThreadArgs* args = new ThreadArgs[nthreads];
    //thread them make sure they have the same host and port
    for (int i = 0; i < nthreads; i++) {
        args[i].argc = argc;
        args[i].argv = argv;
        args[i].s = inputs[i];
        if (pthread_create(&tid[i], NULL, threadSock, &args[i]) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }	
    }  


    for (int i = 0; i < nthreads; i++) {
        pthread_join(tid[i], NULL);
    }

    //printing
    for (int i = 0; i < nthreads; i++) {
    std::cout << "Input string: " << inputs[i] << std::endl;
    std::cout << "RLE String: " << args[i].rleS << std::endl;
    std::cout << "RLE Frequencies:";
    int freqSize = args[i].freq.size();
    for (int j = 0; j < freqSize; j++) {
        std::cout << " " << args[i].freq[j];
    }
    std::cout << std::endl;
}


    delete[] args;

    return 0;
}

//reference rincon from teams
