#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

using std::string;
using std::cin;
using std::cout;
using std::endl;
using std::vector;

vector<string> splitString(const string& input)
{
    std::istringstream ss{input};
    using StrIt = std::istream_iterator<std::string>;
    std::vector<std::string> container{StrIt{ss}, StrIt{}};
    return container;
}

void execMyProgram(std::vector<string> &inputVector) {
    char* argv[inputVector.size() + 1];

    for(int i = 0; i < inputVector.size(); i++)
        argv[i] = &inputVector[i][0u];
    argv[inputVector.size()] = NULL;

    int err = execvp(argv[0], argv);
    if(err != 0){
        cout << "Fehler beim ausführen!" << endl;
        exit(-1);
    }
}


int main() {
    vector<string> inputVector{};
    string input{};
    int status{};
    time_t t;

    cout << "BS Shell" << endl;

    while(true){
        getline(cin, input);
        inputVector = splitString(input);

        // Logout criteria
        if(inputVector[0] == "logout"){
            cout << "Wirklich ausloggen (Y/N)?";
            cin >> input;
            if(input == "Y")
                break;
            else
                continue;
        }

        //execMyProgram(inputVector);

        pid_t pid = fork();
        if(pid == 0){ //    ChildProcess
            execMyProgram(inputVector);
        }
        else{
            //wait(0);
            cout << "Child done" << endl;
        }
        /*
        else do {
            if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
                perror("wait() error");
            else if (pid == 0) {
                time(&t);
                printf("child is still running at %s", ctime(&t));
                sleep(1);
            }
            else {
                if (WIFEXITED(status))
                    printf("child exited with status of %d\n", WEXITSTATUS(status));
                else puts("child did not exit successfully");
            }
        } while (pid == 0);
         */

    }
    cout << "Done";


    return 0;
}
