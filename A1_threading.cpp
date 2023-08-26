#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
using namespace std;

//this struct is for the packageing of the RLE function to store the values when called for
struct arg{
  string s;
  int id;
  string rleS;
  vector<int> freq;
};


//This is my RLE function container 
struct ans{
  string a;
  vector<int> b;
};

//main function
ans RLE(const string str) {
  int n = str.size();
  string answer;
  vector<int> freq;
  
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



int main() {
  string input;
  vector<string> inputs;

  //this grabs all the inputs so we can create the correct number of threads
  while(cin>>input){
    inputs.push_back(input);
  }
  
  int nthreads = inputs.size();
  pthread_t tid[nthreads];
  //creating all the the containers needed for each thread
  static arg *x = new arg[nthreads];

  
  for(int i=0;i<nthreads;i++){
    //making the container each string to pass into the function
    x[i].s = inputs[i];
    //calling the function for each thread
    if(pthread_create(&tid[i], NULL, procRLE, &x[i])) ///*void?
  		{
  			fprintf(stderr, "Error creating thread\n");
  			return 1;
  		}	
  }  
  
  for (int i = 0; i < nthreads; i++){
    //holds all threads so that we can print the threads in the correct order each time
    pthread_join(tid[i], NULL);
  }
  for (int i = 0; i < nthreads; i++){
    //printing everything from each thread
    cout<<"Input string: "<<inputs[i]<<endl;
    cout<<"RLE String: "<<x[i].rleS<<endl;
    cout <<"RLE Frequencies:";
    for (int j = 0; j < x[i].freq.size(); j++) {
      cout <<" "<< x[i].freq[j];
    }
    cout<<endl;
  }

  return 0;
}