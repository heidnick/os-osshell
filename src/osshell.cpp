#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fstream>
#include <iterator>

#define MAXSTR 128
std::vector<std::string> history;

void readHist(int *hist_idx);
void printHist(int start_idx);
void clearHist();
void writeHist();
void addHist(char usr_command[10000]);
void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);
 
    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; // to store command user types in, split into its variour parameters
    char **command_list_exec; // command_list converted to an array of character arrays

    //Initializing history array
    int cur_hist_idx;
    readHist(&cur_hist_idx);
    //printHist(0);
    
    //std::cout << "cur_hist_idx = " << cur_hist_idx << std::endl;
    
    
    int i;
    //begin argument search
    while(true){
        int has_arg = 0;
        char usr_command[10000];
        while(has_arg == 0) {
            std::cout << "osshell> ";
            

            std::cin.getline(usr_command, sizeof(usr_command));
           
            splitString(usr_command, ' ', command_list);
            vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
            if (!command_list.empty()){
                has_arg = 1;
            }    
        }
        
        std::string first_command = command_list_exec[0];
        


        if (first_command == "exit"){
            std::cout << "exit runs" << std::endl;
            addHist(usr_command);
            writeHist();
            exit(1);
        }
        else if (first_command.at(0) == '.'){
            std::cout << "execuatable program" << std::endl;

            /*pid_t pid = fork();
            if (pid == 0) {
                int 
            }*/

        }else if(first_command.at(0) == '/'){
            struct stat buf;
            if (stat(first_command.c_str(), &buf) == 0){
                pid_t pid = fork();
                if (pid == 0) {
                    printf("child spawned\n");
                    execv(first_command.c_str(), command_list_exec);
                }
                else {
                    int status;
                    waitpid(pid, &status, 0);
                }
                addHist(usr_command);

            }
        }else if(first_command == "history"){
            if (command_list.size() == 2){
                std::string arg = command_list_exec[1];
                if (arg == "clear") {
                    clearHist();
                }else if (atoi(command_list_exec[1]) >= 0){
                    printHist(atoi(command_list_exec[1]));
                    addHist(usr_command);
                }else {
                    std::cout << "Error: history expects an integer > 0 (or 'clear')" << std::endl;
                    addHist(usr_command);
                }
            }else if (command_list.size() == 1){
                printHist(0);
                addHist(usr_command);
            }else {
                std::cout << "Error: history expects an integer > 0 (or 'clear')" << std::endl;
                addHist(usr_command);
            }
        }
        else {
            std::string cur_path;
            struct stat buf;
            int found_path = 0;
            pid_t pid;
            addHist(usr_command);
            
            for (i = 0; i < os_path_list.size(); i++){
                cur_path = os_path_list[i].c_str();
                cur_path += "/";
                cur_path += command_list_exec[0];
                if (stat(cur_path.c_str(), &buf) == 0 && found_path == 0) {
                    found_path = 1;
                    pid = fork();
                    if (pid == 0) {
                        execv(cur_path.c_str(), command_list_exec);
                    }
                    else {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                }
            }
            if (found_path == 0) {
                std::cout << "Error command not found" << std::endl;
            }
            freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
        }    
    }
    return 0;
}

void readHist(int *hist_idx){
    std::vector<std::string> file_history;
    std::ifstream in("../tests/input/history.txt");
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string test = buffer.str();
    splitString(test, '\n', file_history);
    int start_hist_idx = 0;
    *hist_idx = file_history.size();
    
    int i = 0;
    for (int k=start_hist_idx; k<file_history.size(); k++){
        history.push_back(file_history[k]);
        i++;
    }

    
}

void printHist(int start_idx){
    int i;
    for(i=start_idx; i<history.size(); i++){
        std::cout << "  " << i+1 << ": " << history[i] << std::endl;
    }
}

void clearHist(){
    while (history.size() > 0){
        history.erase(history.begin());
    }
}

void writeHist() {
    std::ofstream output_file("../tests/input/history.txt");
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(history.begin(), history.end(), output_iterator);
}

void addHist(char usr_command[10000]) {
    if (history.size() == 128){
        history.erase(history.begin());
    }
    history.push_back(usr_command);
}


/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
   list: vector of strings to convert to an array of character arrays
   result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings (array of character arrays) to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}
