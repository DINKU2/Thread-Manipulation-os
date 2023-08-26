#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



struct arg{
  std::string s;
  int id;
  std::string rleS;
  std::vector<int> freq;
};


//This is my RLE function container 
struct ans{
  std::string a;
  std::vector<int> b;
};

//main function
ans RLE(const std::string str) {
  int n = str.size();
  std::string answer;
  std::vector<int> freq;
  
  // basically if the next character is the same as the current we increase the frequency 
  //however this will not work for the last element as it will point to nothing
  for (int i = 0; i < n-1; i++) {
    int count = 1;
    while (str[i] == str[i + 1]) {
        count++;
        i++;
    }
    // this is how i am appending based on the freq if it needs to be two elements or one
    if(count>1){
    freq.push_back(count);
    answer.push_back(str[i]);
    answer.push_back(str[i]);
    } 
    else{
      answer.push_back(str[i]);
    }
  }
  
  //this will check the last element in case it is different
  if(str[n-1]!=str[n-2]){
    answer.push_back(str[n-1]);
  }

  //returning my answer in the container i created
  ans Z;
  Z.a = answer;
  Z.b = freq;
  return Z;
}


//this creates a package for my created RLE function to be used in pthreat create
void* procRLE(void* x_void_ptr){
  struct arg *x_ptr = (struct arg *)x_void_ptr;
  x_ptr-> rleS = RLE(x_ptr->s).a;
  x_ptr-> freq = RLE(x_ptr->s).b;
	return NULL;
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    //signal(SIGCHLD, fireman); 
    if (argc < 2)
    {
        std::cerr << "ERROR, no port provided\n";
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "ERROR opening socket";
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
    {
        std::cerr << "ERROR on binding";
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (true)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (fork() == 0)
        {
            //read the size of the msg
            int msgsize;
            n = read(newsockfd, &msgsize, sizeof(int));
            if (n < 0){
            std::cerr << "Error reading the msgsize from socket" << std::endl;
            exit(0);
            }

            char *themessage = new char[msgsize];
            bzero(themessage, msgsize);

            //read the actual msg 
            n = read(newsockfd, themessage, msgsize);
            if (n < 0){
            std::cerr << "Error reading msg from socket" << std::endl;
            exit(0);
            }
            
            ans answer = RLE(std::string(themessage));

            std::string x=answer.a;
            std::vector<int> y=answer.b;

            //rle string
            //write intial string size
            //i need to copy the message to a string
            std::string str = themessage;

            //possible error here
            int string_size = str.length();
            n = write(newsockfd, &string_size, sizeof(int));
            if (n < 0)
            {
            std::cerr << "Error writing string sizze to socket" << std::endl;
            exit(0);
            }

            n = write(newsockfd, str.c_str(), string_size);
            if (n < 0)
            {
            std::cerr << "Error writing stringto socket" << std::endl;
            exit(0);
            }

            //send rle string size
            std::string newstring = x;
            int rle_size = newstring.length() + 1;
            n = write(newsockfd, &rle_size, sizeof(int));
            if (n < 0)
            {
            std::cerr << "Error writing rle size to socket" << std::endl;
            exit(0);
            }
            //send rle string
            n = write(newsockfd, newstring.c_str(), rle_size);
            if (n < 0)
            {
            std::cerr << "Error writing rle to socket" << std::endl;
            exit(0);
            }


            //send the vector
            int freq_size = y.size();
            n = write(newsockfd, &freq_size, sizeof(int));
            if (n < 0) {
                std::cerr << "Error writing freq size to socket" << std::endl;
                exit(0);
            }

            // Send the elements of the vector
            n = write(newsockfd, y.data(), freq_size * sizeof(int));
            if (n < 0) {
                std::cerr << "Error writing freq to socket" << std::endl;
                exit(0);
            }



            //close the socket
            close(newsockfd);
            _exit(0);
        }
    }
    close(sockfd);
    return 0;
}

//reference rincon from teams
