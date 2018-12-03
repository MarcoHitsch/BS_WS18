#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

vector<int> sharedResource{};
vector<pthread_t> readerThreads{};
vector<pthread_t> writerThreads{};

pthread_mutex_t readerLock = PTHREAD_MUTEX_INITIALIZER;
sem_t writerLock{};
int currentlyReading{0};
double delay{0};

bool traceLog{false};

void write(long writerNumber){
    cout << "Schreibe (" << writerNumber << ")" << endl;

    for(int i{}; i < sharedResource.size(); i++)
        sharedResource[i] = (rand()%10);
    usleep(delay * 1000000);
}

void read(long readerNumber){
    int index = rand()%sharedResource.size();
    int value = sharedResource[index];

    cout << "Lese (" << readerNumber << ") [" << index << "] = " << value << endl;
    usleep(delay * 1000000);
}

void *writerMethod(void* tid){
    long writerNumber = (long)tid;

    while(1){
        if(traceLog)        
            cout << "Writer(" << writerNumber << ") wartet..."<< endl;
        sem_wait(&writerLock);
        write(writerNumber);
        sem_post(&writerLock);
	usleep(0.1*1000000);
    }
    // pthread_exit(NULL);
}


void *readerMethod(void* tid){
    long readerNumber = (long)tid;

    while(1){
        //cout << "Reader(" << readerNumber << ") wartet..." << endl;
        pthread_mutex_lock(&readerLock);
        currentlyReading++;
        if(traceLog)        
            cout << currentlyReading << " lesen" << endl;
        if(currentlyReading == 1){
            if(traceLog)        
                cout << "Reader(" << readerNumber << ") versucht WriterLock..." << endl;
            sem_wait(&writerLock);
            if(traceLog)        
                cout << "Reader(" << readerNumber << ") setzt WriterLock..." << endl;
        }

        pthread_mutex_unlock(&readerLock);

        read(readerNumber);

        //cout << "Reader(" << readerNumber << ") wartet (fertig)..." << endl;
        pthread_mutex_lock(&readerLock);
        currentlyReading--;

        if(traceLog)        
            cout << currentlyReading << " lesen" << endl;
        if(currentlyReading == 0){
            if(traceLog)        
                cout << "Reader(" << readerNumber << ") versucht befreit WriterLock..." << endl;
            sem_post(&writerLock);
            if(traceLog)        
                cout << "Reader(" << readerNumber << ") befreit WriterLock..." << endl;
        }
        pthread_mutex_unlock(&readerLock);
	usleep(0.1*1000000);
    }
    // pthread_exit(NULL);
}

int main() {

    int tracelog{0};
    int vectorSize{};
    unsigned int readerCount{};
    unsigned int writerCount{};

    sem_init(&writerLock, 0, 1);

    cout << "Resourceplatz Anzahl_Reader Anzahl_Writer Tracelog Delay" << endl;
    cin >> vectorSize >> readerCount >> writerCount >> tracelog >> delay;
    traceLog = (tracelog == 1);

    srand(time(NULL));
    int status{};
    void* param{};

    for(int i{}; i < vectorSize; i++)
        sharedResource.push_back(rand()%10);

    writerThreads = vector<pthread_t>{writerCount};
    readerThreads = vector<pthread_t>{readerCount};

    for(long i{}; i < writerCount; i++)
        pthread_create(&writerThreads[i], NULL, writerMethod, (void*)i);

    for(long i{}; i < readerCount; i++)
        pthread_create(&readerThreads[i], NULL, readerMethod, (void*)i);


    for(int i{}; i < writerCount; i++)
        pthread_join(writerThreads[i], NULL);

    for(int i{}; i < readerCount; i++)
        pthread_join(readerThreads[i], NULL);
}