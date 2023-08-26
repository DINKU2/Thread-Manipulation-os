#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
using namespace std;

// This struct is for packaging the RLE function to store the values when called for
struct arg {
  string s;
  int id;
  string rleS;
  vector<int> freq;
  pthread_mutex_t *bsem;
  pthread_cond_t *printTurn;
  int *turn;
  int threadID;
};

// This is my RLE function container
struct ans {
  string a;
  vector<int> b;
};

// RLE function
ans RLE(const string str) {
  int n = str.size();
  string answer;
  vector<int> freq;

  for (int i = 0; i < n - 1; i++) {
    int count = 1;
    while (str[i] == str[i + 1]) {
      count++;
      i++;
    }
    if (count > 1) {
      freq.push_back(count);
      answer.push_back(str[i]);
      answer.push_back(str[i]);
    } else {
      answer.push_back(str[i]);
    }
  }

  if (str[n - 1] != str[n - 2]) {
    answer.push_back(str[n - 1]);
  }

  ans Z;
  Z.a = answer;
  Z.b = freq;
  return Z;
}

// Function to be used in pthread_create
void *procRLE(void *x_void_ptr) {
  struct arg *ptr = (struct arg *)x_void_ptr;

  // Call RLE function with the local copy of the string
  ans result = RLE(ptr->s);

  pthread_mutex_lock(ptr->bsem);
  while (*(ptr->turn) != ptr->threadID)
    pthread_cond_wait(ptr->printTurn, ptr->bsem);
  pthread_mutex_unlock(ptr->bsem);

  ptr->rleS = result.a;
  ptr->freq = result.b;

  cout << "Input string: " << ptr->s << endl;
  cout << "RLE String: " << ptr->rleS << endl;
  cout << "RLE Frequencies:";
  for (int i = 0; i < ptr->freq.size(); i++) {
    cout << " " << ptr->freq[i];
  }
  cout << endl << endl;

  pthread_mutex_lock(ptr->bsem);
  (*(ptr->turn))++;
  pthread_cond_broadcast(ptr->printTurn);
  pthread_mutex_unlock(ptr->bsem);

  return NULL;
}

int main() {
  pthread_mutex_t bsem;
  pthread_cond_t printTurn = PTHREAD_COND_INITIALIZER;
  pthread_mutex_init(&bsem, NULL);
  static int turn = 0;

  string input;
  vector<string> inputs;

  while (cin >> input) {
    inputs.push_back(input);
  }

  int nthreads = inputs.size();
  pthread_t tid[nthreads];
  struct arg *x = new arg[nthreads];

  for (int i = 0; i < nthreads; i++) {
    x[i].s = inputs[i];
    x[i].id = i;  // Assign the id for each thread
    x[i].bsem = &bsem;
    x[i].printTurn = &printTurn;
    x[i].turn = &turn;
    x[i].threadID = i;
    if (pthread_create(&tid[i], NULL, procRLE, &x[i])) {
      fprintf(stderr, "Error creating thread\n");
      return 1;
    }
  }

  for (int i = 0; i < nthreads; i++) {
    pthread_join(tid[i], NULL);
  }

  return 0;
}
