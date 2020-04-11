#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <cctype>
#include <dirent.h>
#include <algorithm>
#include <cstddef>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <list>
#include <sys/wait.h>
#include <limits.h>

using namespace std;
#define PROMPT_SIZE 16
#define HIST_SIZE 10
int stillAlive = 1;
// this vector is what will be used to store my history of commands
vector<string> pastCommands;
int whereInHistory = 0;

// this list will hold all the possible "tokens" in the input line. ie: |<>
list<char> tokens;

// function gets the current directory
string getCurDir(){
    //https://www.ibm.com/support/knowledgecenter/SSB27U_6.4.0/com.ibm.zvm.v640.edclv/rtgtc.htm
    char curPath[1024];
    string CWD;
    if (getcwd(curPath, sizeof(curPath))) {
        CWD = curPath;
    } else {
        CWD = "";
    }
    return CWD;
}

// this Class is an object for each command that is inputted and will split between whether it is a command or an argument
// for the case when there are <>, the cmd could also be a file name
class commandOrFile {
    public:
        string cmd;
        vector<string> arg;
        string output;
        
        commandOrFile() {
          cmd = "";
          arg.clear();
          output = ""; // save command output
        }

        void prn() {
            write(STDOUT_FILENO, "cmd: ", 5);
            write(STDOUT_FILENO, cmd.c_str(), cmd.size());
            write(STDOUT_FILENO, "\n", 1);
            write(STDOUT_FILENO, "arg: ", 5);

            for (unsigned i=0; i<arg.size(); i++) {
                write(STDOUT_FILENO, arg[i].c_str(), arg[i].size());
                write(STDOUT_FILENO, " ", 1);
            }
            write(STDOUT_FILENO, "\n", 1);
        }

        bool isForkCommand () {
            bool isFork = true;
            if (cmd == "cd" || cmd == "exit") {
                isFork = false;
            }
            return isFork;
        }

        void run() {
            if (cmd == "ls") {
                ls();
            } else if (cmd == "pwd") {
                pwd();
            } else if (cmd == "cd") {
                cd();
            } else if (cmd == "ff") {
                ff();
            } else if (cmd == "exit") {
                stillAlive = 0;
            } else {
                myexec();
            }
        }

    private:
        // convert vector string to char * argv_list[]
        const vector<char *> convertArg() {
            vector<string> cmdArg;
            cmdArg.push_back(cmd);

            for (unsigned i=0; i<arg.size(); i++) {
                cmdArg.push_back(arg[i]);
            }

            vector<char *> argv(cmdArg.size() + 1);    // one extra for the null
            for (std::size_t i = 0; i != cmdArg.size(); ++i) {
                argv[i] = &arg[i][0];
            }

            return(argv);
        }

        void PrintPermissions(string dir_name, struct dirent *fileIn) {
            //https://pubs.opengroup.org/onlinepubs/009695399/functions/stat.html
            struct stat sb;
            string fileName = "";
            fileName = fileIn->d_name;
            string fullName = dir_name + fileName;
        
            stat (fileName.c_str(), &sb);
            
            //https://linux.die.net/man/2/stat
            switch(sb.st_mode & S_IFMT) {
                case S_IFDIR:
                    output += "d";
                    break;
                case S_IFBLK:
                    output += "b";
                    break;
                case S_IFCHR:
                    output += "c";
                    break;
                case S_IFSOCK:
                    output += "s";
                    break;    
                default:
                    output += "-";
            }

            // owner permissions
            if (sb.st_mode & S_IRUSR) {
                output += "r";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IWUSR) {
                output += "w";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IXUSR) {
                output += "x";
            } else {
                output += "-";
            }

            //group permissions
            if(sb.st_mode & S_IRGRP) {
                output += "r";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IWGRP) {
                output += "w";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IXGRP) {
                output += "x";
            } else {
                output += "-";
            }

            //other permissions
            if(sb.st_mode & S_IROTH) {
                output += "r";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IWOTH) {
                output += "w";
            } else {
                output += "-";
            }
            if(sb.st_mode & S_IXOTH) {
                output += "x ";
            } else {
                output +=  "- ";
            }

            output += fileName;
        }

        void ls() {

            // this means that there isn't any arguments so just typing in ls
            string dir = "";

            if (arg.size() == 0) {
                dir = "./";
            } else {
                int lastIndex = arg.size()-1;

                // sorry, no option for ls
                if ( arg[lastIndex][0] == '-' ) {
                    dir = "./";
                } else {
                    dir = arg[lastIndex].c_str();
                }
            }

            DIR *parentDir;
            //const char *directionyName = dir.c_str(); 
            //parentDir = opendir(directionyName); 
            struct dirent *current;
            if ((parentDir = opendir(dir.c_str()))) {
                while((current = readdir(parentDir)) != NULL) {
                    PrintPermissions(dir, current);
                    output += "\n";
                }
                closedir(parentDir);
            }
        }

        void cd() {
            if (arg.size() == 0){
                // means that cd has no arguments
                chdir(getenv("HOME"));
            } else {
                // cd has arg
                int rc = chdir(arg[0].c_str());
                // print different error message 
            }
        }

        void ff() {
            string filename = "";
            string directory = "";
            
            if (arg.empty()) {
                output += "ff command requires a filename!";
            } else if (arg.size() == 1) {
                filename = arg[0];
                directory = "./";
            } else if (arg.size() == 2) {
                filename = arg[0];
                directory = arg[1];
            }

            //DIR *directory;
        }

        void pwd() {
            string cwd = getCurDir();
            output += cwd.c_str();
            output += "\n";
        }

        void myexec() {
            int fd, size;
            vector<char *> argv = convertArg();
            static char tmp[] = "/tmp/myfileXXXXXX";
	        char fname[PATH_MAX];
            strcpy(fname, tmp);		/* Copy template */
	        fd = mkstemp(fname);			/* Create and open temp file */

            //string tmpfile = tmpnam(nullptr);
            //fd = open(tmpfile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXO | S_IRWXU | S_IRWXG);
            dup2(fd, STDOUT_FILENO);
            execvp(argv[0], argv.data());

            //fd = open(tmpfile, O_RDONLY);
            lseek(fd, 0L, SEEK_SET);
            //char *buffer = (char *) calloc(100, sizeof(char));
            char buffer[100];
            while ((size = read(fd, buffer, 100)) > 0) {
                output += buffer;
            }
            close(fd);
            unlink(fname);
        }
};

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    char *name;
    
    // Make sure stdin is a terminal. 
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }
    
    // Save the terminal attributes so we can restore them later. 
    tcgetattr(fd, savedattributes);
    
    // Set the funny terminal modes. 
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO. 
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

 vector<commandOrFile> parser(string inputLine) {
    string command = "";
    string argument = "";
    vector<string> arguments;
    bool commandDone = false;
    bool argumentDone = false;
    vector<commandOrFile> InstanceCommands;

    for (int i = 0; i < inputLine.size(); i++) {
        if (isalpha(inputLine[i]) || inputLine[i] == '/' || inputLine[i] == '-' || inputLine[i] == '_' || inputLine[i] == '.') {
            if (commandDone) {
                argument += inputLine[i];
                //cout << "argument: " << argument << endl;
            } else {   
                command += inputLine[i];
            }
        } else if (isspace(inputLine[i])) {
            if (command.size() == 0) { // skip if it's starting space
                continue;
            } else if (inputLine[i-1] == ' ') { // skip if it's trailing space
                continue;
            } else if (commandDone == false && command.size() != 0) {
                commandDone = true;
            } else if (argument.size() != 0 ) {
                arguments.push_back(argument);
                argument.clear();
            }
        } else if (inputLine[i] == '|' || inputLine[i] == '>' || inputLine[i] == '<') {
            tokens.push_back(inputLine[i]);
            //cout << "token: " << inputLine[i] << endl;
            commandOrFile newCmd;
            newCmd.cmd = command;
            newCmd.arg = arguments;
            InstanceCommands.push_back(newCmd);

            command.clear();
            argument.clear();
            arguments.clear();
            commandDone = false;
        } else {
            continue;
        }
    }

    if (argument.size() != 0 ) {
        arguments.push_back(argument);
    }

    commandOrFile newCmd;
    newCmd.cmd = command;
    newCmd.arg = arguments;
    InstanceCommands.push_back(newCmd);

    write(STDOUT_FILENO, "Command array: ", 15);
    write(STDOUT_FILENO, "\n", 1);
    for (unsigned i=0; i<InstanceCommands.size(); i++) {
        InstanceCommands[i].prn();
    }

    return(InstanceCommands);

}

/*void pipping(vector<commandOrFile>InstanceCommands) {
    int numPipes = InstanceCommands.size() - 1;
    vector<int> pipes;
    for (int i = 0; i < InstanceCommands.size() - 1; i++) {
        
    }
    pid_t pipe = fork();
}*/

void shellPrompt() {
    string shellPrompt = getCurDir();
    if (shellPrompt.size() > PROMPT_SIZE) {
        int lastDash =  shellPrompt.rfind('/');
        shellPrompt = "/..." + shellPrompt.substr(lastDash, shellPrompt.size());
    }
    shellPrompt += "% ";
    write(STDOUT_FILENO, shellPrompt.c_str(), shellPrompt.length());
}

bool mgrPipe(bool inUse) {
    if ( inUse ) {
        dup2(STDOUT_FILENO, STDOUT_FILENO);
    }

    return(false);
}

int main(int argc, char **argv){
    struct termios SavedTermAttributes;
    string line = "";
    char RXChar;
    int newPrompt = 1;
    vector<string> commands;
    vector<commandOrFile>InstanceCommands;
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);

    while(stillAlive == 1){
        if(0x04 == RXChar){ // C-d
            write(STDOUT_FILENO, "\n", 1);
            break;
            stillAlive = 0;
        }
        if(newPrompt == 1)
        {
            shellPrompt();
            newPrompt = 0;
        }
        read(STDIN_FILENO, &RXChar, 1);

        // enter key is pressed
        // add previous command into passCommands
        // clear line
        if(RXChar == 0x0A) {
            write(STDOUT_FILENO, "\n", 1);
            pastCommands.push_back(line);
            
            write(STDOUT_FILENO, "Command: ", 9);
            for (int i = 0; i < line.size(); i++) {
                write(STDOUT_FILENO, &line[i], 1);
            }
            write(STDOUT_FILENO, "\n", 1);
            InstanceCommands = parser(line);
            
            if (InstanceCommands[0].isForkCommand()) {
                //https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
                pid_t childPid; 
                childPid = fork();

                if (childPid == 0) { //fork success
                    int i = 0;
                    char token;
                    int fd;
                    int pipefd[2];
                    pipe(pipefd);
                    bool pipeUsed = false;

                    while ( i < InstanceCommands.size() ) {
                        if ( tokens.size() == 0 ) {
                            InstanceCommands[i].run();
                            write(STDOUT_FILENO, InstanceCommands[i].output.c_str(), InstanceCommands[i].output.size());
                            break; // no token means only one command to run
                        } else {
                            token = tokens.front();
                            tokens.pop_front();

                            if ( token == '>' ) {
                                InstanceCommands[i].run();
                                pipeUsed = mgrPipe(pipeUsed);
                                i++;
                                fd = open(InstanceCommands[i].cmd.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXO | S_IRWXU | S_IRWXG);
                                dup2(fd, STDOUT_FILENO);
                                write(STDOUT_FILENO, InstanceCommands[i-1].output.c_str(), InstanceCommands[i-1].output.size());
                                close(fd);
                            } else if ( token == '<' ) {
                                i++;
                                fd = open(InstanceCommands[i].cmd.c_str(), O_RDONLY);
                                dup2(fd, STDIN_FILENO);
                                InstanceCommands[i-1].run();
                                close(fd);
                                write(STDOUT_FILENO, InstanceCommands[i-1].output.c_str(), InstanceCommands[i-1].output.size());
                                pipeUsed = mgrPipe(pipeUsed);
                            } else if ( token == '|' ) {
                                 // redirect stdout to pipe write end, all output will go to pipe pipefd[1]
                                dup2(pipefd[1], STDOUT_FILENO);
                                InstanceCommands[i].run();
                                write(STDOUT_FILENO, InstanceCommands[i].output.c_str(), InstanceCommands[i].output.size());
                                // redirect stdout back to stdout/console
                                dup2(STDOUT_FILENO, STDOUT_FILENO);

                                // redirect stdin to pipe
                                if ( ! pipeUsed ) {
                                    dup2(pipefd[0], STDIN_FILENO);
                                    pipeUsed = true;
                                }
                            }
                        }
                    }

                    // close pipe
                    close(pipefd[0]);
                    close(pipefd[1]);

                    exit(0); // exits the child process

                }  else { // parent
                    int returnStatus;    
                    waitpid(childPid, &returnStatus, 0);
                }
            } else {
                InstanceCommands[0].run();
            }

            line = "";
            // reset isNewPrompt to yes after user presses enter
            newPrompt = 1;
            //stillAlive = 0;

        //backspace
        } else if (RXChar == 0x7F) {
            // if its the first time typing and the line of inserts is currently empty, then output the bell
            if (line.empty()) {
                write(STDOUT_FILENO, "\a", 1); // \a is the output bell
            } else {
                //You’re popping off the last char of the string but when you print it out
                //write, you don’t want to just do a back space, you want to do \b \b (backspace space backspace)
                write(STDOUT_FILENO, "\b \b", 3);
                line.erase(line.size() - 1);
            }
        // up key pressed
        } else if (RXChar == 0x1B) {
            read(STDIN_FILENO, &RXChar, 1);
            read(STDIN_FILENO, &RXChar, 1);
            if(RXChar == 0x41) {
                string substitute = "";
                // nothing in the history queue
                if (pastCommands.size() == 0) {
                    write(STDOUT_FILENO, "\a", 1);
                    continue;
                }
                if (whereInHistory + 1 > pastCommands.size()) {
                    write(STDOUT_FILENO, "\a", 1);
                    continue;
                }
                //move one space in history
                whereInHistory++;
                // the value wanted to return and go in reverse order
                substitute = pastCommands[pastCommands.size() - whereInHistory];
                // this for loop clears up all command line so you when type up, you get a the previous command instead of all the commands concatenated with one another
                int i = 0;
                while (i < line.size()) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    i++;
                }
                // write out the new "history" command
                write(STDOUT_FILENO, substitute.c_str(), substitute.length());
                line = substitute;

            //down key pressed
            } else if (RXChar == 0x42) {
                string substitute = "";
                // nothing in the history queue
                if (pastCommands.size() == 0 || whereInHistory == 0) {
                    write(STDOUT_FILENO, "\a", 1);
                    continue;
                }
                //move one space down the history
                whereInHistory--;
                // this is the case where you are already at the very end of your "history of commands"
                // want to have the line be clear so that the user can input something new
                if (whereInHistory == 0) {
                    substitute = "";
                } else {
                    // the value wanted to return and go in reverse order
                    substitute = pastCommands[pastCommands.size() - whereInHistory];
                }
                
                int i = 0;
                while (i < line.size()) {
                    write(STDOUT_FILENO, "\b \b", 3);
                    i++;
                }
                // write out the new "history" command
                write(STDOUT_FILENO, substitute.c_str(),substitute.length());
                //rewrite over your line with the correct value in the history vectory
                line = substitute;
            }
        // if a key is just alphanumeric, then add it to the current line of commands
        } else {
            write(STDOUT_FILENO, &RXChar, 1);
            line += RXChar;
        }
    } 
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    return 0;
}