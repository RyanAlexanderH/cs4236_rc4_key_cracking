#include <algorithm>
#include <cstdio>
#include <vector>
using namespace std;

typedef vector<int> vi;

const int N = 160;    // must be <= 256   for higher value, use version 2 rc4v2.c
                      // WEP uses 256.  In this project, we set it to 160.
const int MAX = 90;   // The maximum key value where the key is [0,89]

int  L;     // Size of array K. must be <= N,   WEP use 8. In this project, it can be
            // larger than 8.
int  S[N];  // Permutation table
int  K[N];  // Keys (size of key is L but we allocated a larger memory for programming convenience).
int tuples; // Number of data tuples
int j;

vector<vi> candidates;
vector<vi> data;

//-------------------------
// Key Scheduling Algorithm
//-------------------------
void ksa(int lim) {
  int i;
  
  for (i = 0; i < N; i++)
    S[i]=i;

  j=0;

  for (i = 0; i < lim; i++) {
    int temp;
    j = ( j+  S[i] + K [ i % L ] ) % N;
    temp = S[j]; S[j] = S[i]; S[i] = temp;
  }
}

//-------------------------
// Key Checking Algorithm
//-------------------------
bool confirmKey() {
  for (int i = 0; i < tuples; i++) {
    for (int d = 0; d < 3; d++) {
      K[d] = data[i][d];
    }
    ksa(N);
    // Check whether the first output byte of PGRA is still correct
    // after a full round of KSA (using all the keys)
    if (S[ ( S[1] + S[S[1]] ) % N ] != data[i][3]) {
      return false;
    }
  }

  printf("Key Found: ");
  for (int i = 3; i < L; i++) {
    printf("%d ", K[i]);
  }
  printf("\n");
  return true;
}

//-------------------------
// Utility Modulo Function
//-------------------------
int mod(int a, int b) {
  int r = a % b;
  return r >= 0 ? r : r + b;
}

//-------------------------
// Key Finder Algorithm
//-------------------------
bool findKey(int B) {
  int p = B+3;

  if (p >= L-3) { // Use bruteforce for the last 3 bytes
    for (int a = 0; a < MAX; a++) {
      K[p] = a;
      for (int b = 0; b < MAX ; b++) {
        K[p+1] = b;
        for (int c = 0; c < MAX; c++) {
          K[p+2] = c;
          if (confirmKey()) return true;
        }
      }
    }
    // Reaching here means no key is found
    printf("Key not found...\n");
    return false;
  }

  for (int c = 0; c < tuples; c++) {
    for (int d = 0; d < 3; d++) {
      K[d] = data[c][d];
    }
    int x = data[c][3];

    ksa(p);

    int cand, Si0, SiX;
    for (int i = 0; i < N; i++) {
      if (S[i] == 0) {
        Si0 = i;
      } else if (S[i] == x) {
        SiX = i;
      }
    }
    // Korek 1: FMS
    if (S[1] < p && mod(S[1] + S[S[1]], N) == p && SiX != 1 && SiX != 4 && SiX != S[S[1]]) {
      cand = mod(SiX - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 2
    if (S[1] == p && x == p) {
      cand = mod(Si0 - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 3
    if (S[1] == p && x == mod(1-p, N)) {
      cand = mod(SiX - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 4
    if (S[1] == p && x != mod((1-p), N) && x != p && SiX < p) {
      int loc;
      for (int i = 0; i < N; i++) {
        if (S[i] == mod(SiX - p, N)) {
          loc = i; break;
        }
      }
      if (loc != 1) {
        cand = mod(loc - S[p] - j, N);
        candidates[p][cand]++;
      }
    }
    // Korek 5
    if (SiX == 2 && S[p] == 1) {
      cand = mod(1 - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 6
    if (S[p] == p && S[1] == 0 && x == p) {
      cand = mod(1 - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 7
    if (S[p] == p && x == S[1] && S[1] == mod(1-p, N)) {
      cand = mod(1 - S[p] - j, N);
      candidates[p][cand]++;
    }
    // Korek 8
    if (S[p] == p && S[1] >= (N - p) % N && S[1] == mod(SiX - p, N) && SiX != 1) {
      cand = mod(1 - S[p] - j, N);
      candidates[p][cand]++;
    }
  }
  
  K[p] = distance(candidates[p].begin(), max_element(candidates[p].begin(), candidates[p].end()));
  return findKey(B+1);
}

int main(int argc, char *argv[]) {
  FILE *dataFile;
  unsigned char buffer[4];

  dataFile = fopen (argv[1] , "rb");
  if (dataFile==NULL) {
    fputs ("File error", stderr);
    exit (1);
  }

  //Read L
  fread(buffer, 4, 1, dataFile);
  int *keySize = (int*)buffer;
  L = *keySize;
  printf("L (size of K) = %d\n", L);

  //Read # of tuples
  fread(buffer, 4, 1, dataFile);
  tuples = *(int*)buffer;
  printf("Number of Tuples = %d\n", tuples);

  data.assign(tuples, vi(L+1,0));

  int counter = 0;
  while (fread(buffer, 4, 1, dataFile) > 0) {
    data[counter][0] = buffer[0];
    data[counter][1] = buffer[1];
    data[counter][2] = buffer[2];
    data[counter][3] = buffer[3];
    counter++;
  }

  fclose (dataFile);

  candidates.assign(L, vi(N, 0));
  findKey(0);

  return 0;
}