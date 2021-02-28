#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

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
    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)
    std::cout << "osshell> ";
    char usr_command[100];
    std::cin.getline(usr_command, sizeof(usr_command));
    std::cout << usr_command << std::endl;
    
    //std::string example_command = "ls -lh";
    //splitString(example_command, ' ', command_list);
    splitString(usr_command, ' ', command_list);
    vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);

    /*
        cur_path   -> store env path, must add .c_str() when using cur_path as param for stat()
        buf        -> pass as &buf as second param to stat()
        found_path -> if 0, no path contains executable file, 1 vis versa
        pid        -> if pid > 0 = parent, if pid = 0 = child
    */

    std::string cur_path;
    struct stat buf;
    int found_path = 0;
    pid_t pid;
    
    //Uncomment to print cmd line
    /*
    int j = 0;
    while (command_list_exec[j] != NULL)
    {
        std::cout << command_list_exec[j] << std::endl;
        j++;
    }
    */

    /*
        Done:
        Parse through env paths, appends '/filename' to cur_path
        if stat() returns valid, set found_path to true and fork()
        if inside child process, execv to execute user process.  ---> probably need error check for bad user params

        To Do:
        - error check for bad user params on execv
        - create loop to reprompt user
        - exit -> check if command_list_exec[0] is exit, then call exit(1)
        - history -> create a history size 128 string array
        - create printHist(int start)
        - in same error check as exit, check for ./ executable files
        - readHist() writeHist() functions for saving/reading from file
        - what happens when we get new commands? probably need to keep an index of oldest/most recent for history array
        - on history clear, should initialize all to NULL?
    */
    for (int i = 0; i < os_path_list.size(); i++){
        cur_path = os_path_list[i].c_str();
        cur_path += "/";
        cur_path += command_list_exec[0];
        if (stat(cur_path.c_str(), &buf) == 0 && found_path == 0) {
            found_path = 1;
            std::cout << cur_path << std::endl;
            pid = fork();
            if (pid == 0) {
                printf("child spawned\n");
                execv(cur_path.c_str(), command_list_exec);
                exit(0);
            }
            else if (pid > 0) {
                wait(NULL);
            }
        }
    }
    freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    
    return 0;
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
