#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;

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
    }
}

int main() {
    vector<string> inputVector{};
    string input{};
    int status{};
    bool waitForProcess{true};

    cout << "BS Shell" << endl;

    while(true){
        getline(cin, input);
        if(input == "")
            continue;

        inputVector = splitString(input);
        waitForProcess = inputVector[inputVector.size() - 1] != "&";

        // Logout Kriterium
        if(inputVector[0] == "logout"){
            cout << "Wirklich ausloggen (y/n)?";
            cin >> input;
            if(input == "y")
                break;
            else
                continue;
        }

        // Neuer Prozess und Unterscheidung
        pid_t pid = fork();
        if(pid == 0){               // ChildProcess
            execMyProgram(inputVector);
        }
        else{                       // ParentProcess
            if(waitForProcess)
                waitpid(pid, &status, 0);
            else
                cout << "Child process: " << pid << endl;
        }

    } // while(true)
    return 0;
}
