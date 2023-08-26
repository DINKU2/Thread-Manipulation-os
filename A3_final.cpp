#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
using namespace std;


// This struct is for packaging the RLE function to store the values when called for
struct arg
{
    string s;
    int id;
    string rleS;
    vector<int> freq;
    pthread_mutex_t *bsem;
    pthread_mutex_t *sem2;
    pthread_cond_t *printTurn;
    int *turn;
    int threadID;
};

// This is my RLE function container
struct ans
{
    string a;
    vector<int> b;
};

// RLE function
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

// Function to be used in pthread_create
void* procRLE(void* x_void_ptr)
{

    //unlocking arg so it can change for other threads
    struct arg ptr = *((struct arg *)x_void_ptr);
    pthread_mutex_unlock(ptr.sem2);


    // Call RLE function with the local copy of the string
    ans result = RLE(ptr.s);

    // Critical section (RLE result computation)
    ptr.rleS = result.a;
    ptr.freq = result.b;


    pthread_mutex_lock(ptr.bsem);
    // Synchronize and print results
    while (*(ptr.turn) != ptr.threadID)
        pthread_cond_wait(ptr.printTurn, ptr.bsem);
    pthread_mutex_unlock(ptr.bsem);

    //only the correct thread will print based on the right turn
    cout << "Input string: " << ptr.s << endl;
    cout << "RLE String: " << ptr.rleS << endl;
    cout << "RLE Frequencies:";
    for (int i = 0; i < ptr.freq.size(); i++)
    {
        cout << " " << ptr.freq[i];
    }
    cout << endl << endl;

    //incrementing turn so the next thread can proceed in the correct order
    pthread_mutex_lock(ptr.bsem);
    (*(ptr.turn))++;
    pthread_cond_broadcast(ptr.printTurn);
    pthread_mutex_unlock(ptr.bsem);

    return NULL;
}


int main()
{
    //initialize mutex semaphore and condition variable
    pthread_mutex_t bsem;
    pthread_mutex_init(&bsem, NULL);
    pthread_mutex_t sem2;
    pthread_mutex_init(&sem2, NULL);
    pthread_cond_t printTurn = PTHREAD_COND_INITIALIZER;
    static int turn = 0;

    //saving the input
    string input;
    vector<string> inputs;
    while (cin >> input)
    {
        inputs.push_back(input);
    }


    //structuring the arg
    int nthreads = inputs.size();
    pthread_t tid[nthreads];
    struct arg x;
    x.bsem = &bsem;
    x.sem2 = &sem2;
    x.printTurn = &printTurn;
    x.turn = &turn;

    //creating thread but using a lock to copy arg
    for (int i = 0; i < nthreads; i++)
    {
        pthread_mutex_lock(x.sem2);
        x.s = inputs[i];
        x.threadID = i;
        if (pthread_create(&tid[i], NULL, procRLE, &x))
        {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }

    }

    //Wait for all threads
    for (int i = 0; i < nthreads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    return 0;
}
