#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <unistd.h>
#include <queue>

using namespace std;

class Waitable{
public:
    sem_t* sem;
    bool isReader;
    Waitable(sem_t* psem, bool pisReader) : sem{psem}, isReader{pisReader} {};
};

vector<int> sharedResource{};
vector<pthread_t> readerThreads{};
vector<pthread_t> writerThreads{};

pthread_mutex_t readerLock = PTHREAD_MUTEX_INITIALIZER;

queue<Waitable*> waitQueue{};

int currentlyReading{0};
double delay{0};


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

    sem_t sem{};
    sem_init(&sem, 0, 0);
    Waitable waitable{&sem, false};

    while(1){

        waitQueue.push(&waitable); //Queue Lock
        cout << "Writer(" << writerNumber << ") wartet" << endl;
        sem_wait(&sem);

        write(writerNumber);


        //cout << "Befreie Reader..." << endl;
        try{
            Waitable* first = waitQueue.front();
            while(first != nullptr && first->isReader){
                sem_post(first->sem);
                waitQueue.pop();
                first = waitQueue.front();
            }
        }
        catch(exception e){
            cout << "Fehler beim befreien: " << e.what() << endl;
        }
    }
    // pthread_exit(NULL);
}


void *readerMethod(void* tid){
    long readerNumber = (long)tid;

    sem_t sem{};
    sem_init(&sem, 0, 1);
    Waitable waitable{&sem, true};

    while(1){

        waitQueue.push(&waitable);
        cout << "Reader(" << readerNumber << ") wartet" << endl;
        sem_wait(&sem);

        //Kritischer Abschnitt, aktuell lesende inkrementieren
        pthread_mutex_lock(&readerLock);
        currentlyReading++;
        pthread_mutex_unlock(&readerLock);

        read(readerNumber);

        //Kritischer Abschnitt, aktuell lesende dekrementieren
        //Writer unlocken
        pthread_mutex_lock(&readerLock);
        currentlyReading--;
        if(currentlyReading == 0){
            try{
                Waitable* first = waitQueue.front();
                while(first != nullptr && !first->isReader){
                    sem_post(first->sem);
                    waitQueue.pop();
                    first = waitQueue.front();
                }
            }
            catch(exception e){
                cout << "Fehler beim befreien: " << e.what() << endl;
            }
        }
        pthread_mutex_unlock(&readerLock);
    }
    // pthread_exit(NULL);
}

int main() {

    int vectorSize{};
    unsigned int readerCount{};
    unsigned int writerCount{};

    cout << "Resourceplatz Anzahl_Reader Anzahl_Writer Delay" << endl;
    cin >> vectorSize >> readerCount >> writerCount >> delay;

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
