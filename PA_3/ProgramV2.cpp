#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <signal.h>

using namespace std;

int status{};
pid_t currentForegroundPid{};
int currentProcessStatus{};
vector<pid_t> backgroundProcesses{};

vector<string> splitString(const string& input)
{
    istringstream ss{input};
    using StrIt = istream_iterator<string>;
    vector<string> container{StrIt{ss}, StrIt{}};
    return container;
}

void execMyProgram(std::vector<string> &inputVector) {
    char* argv[inputVector.size() + 1];

    for(int i = 0; i < inputVector.size(); i++)
        argv[i] = &inputVector[i][0u];
    argv[inputVector.size()] = NULL; // Null terminiertes Argumente Array

    int err = execvp(argv[0], argv);
    if(err != 0){
        cout << "Fehler beim ausführen!" << endl;
        exit(-1);
    }
}

void checkBackgroundProcesses(){
    for(int i{0}; i < backgroundProcesses.size(); i++){
        waitpid(backgroundProcesses[i], &status, WNOHANG);
        if(WIFEXITED(status)){
            cout << backgroundProcesses[i] << " beendet" << endl;
            backgroundProcesses.erase(backgroundProcesses.begin() + i);
        }
    }
}

bool logout() {
    string input{};

    if (backgroundProcesses.size() > 0) {
        cout << "Es laufen noch " << backgroundProcesses.size() << " Hintergrundprozesse" << endl;
        return false;
    }

    cout << "Wollen sie die Shell wirklich beenden? (y|n)" << endl;
    cin >> input;
    return input == "y";
}

void signalHandler(int signum){

    if(currentForegroundPid == 0 && signum == SIGINT){
        if(logout())
            exit(0);
        else
            return;
    }

    if(currentForegroundPid == 0 && signum != SIGCHLD)
        return;

    switch(signum){
        case SIGINT:
            cout << " caught SIGINT" << endl;
            kill(currentForegroundPid, SIGINT);
            currentForegroundPid = 0;
            break;
        case SIGTSTP:
            cout << " caught SIGTSTP" << endl;
            kill(currentForegroundPid, SIGTSTP);
            break;
        case SIGCHLD:
            cout << "caught SIGCHLD" << endl;
            checkBackgroundProcesses();
            // waitpid(-1, &status, WNOHANG);
            break;
    }
}

void unlockProcess(bool fg){
    kill(currentForegroundPid, SIGCONT);
    if(fg){
        cout << "Aktiviere Prozess " << currentForegroundPid << " im FG" << endl;
        // pause();
        waitpid(currentForegroundPid, &currentProcessStatus, 0);
    }
    else{
        cout << "Aktiviere Prozess " << currentForegroundPid << " im BG" << endl;
        backgroundProcesses.push_back(currentForegroundPid);
        currentForegroundPid = 0;
    }
}



int main() {
    vector<string> inputVector{};
    string input{};
    bool waitForProcess{true};

    cout << "BS Shell" << endl;

    signal(SIGINT, signalHandler);
    signal(SIGTSTP, signalHandler);
    signal(SIGCHLD, signalHandler);

    while(true){
        getline(cin, input);
        if(input == "")
            continue;

        inputVector = splitString(input);
        waitForProcess = inputVector[inputVector.size() - 1] != "&";

        // Logout Kriterium
        if(inputVector[0] == "logout"){
            if(logout())
                break;
            else
                continue;
        }

        // fg
        if(inputVector[0] == "fg"){
            unlockProcess(true);
            continue;
        }

        // bg
        if(inputVector[0] == "bg"){
            unlockProcess(false);
            continue;
        }

        // Neuer Prozess und Unterscheidung
        pid_t pid = fork();
        setpgid(pid, pid);
        if(pid == 0){               // ChildProcess
            execMyProgram(inputVector);
        }
        else{                       // ParentProcess
            if(waitForProcess){
                currentForegroundPid = pid;
                pause();
                // waitpid(pid, &currentProcessStatus, 0);
            }
            else{
                cout << "Child process: " << pid << endl;
                backgroundProcesses.push_back(pid);
            }
        }

    } // while(true)
    return 0;
}