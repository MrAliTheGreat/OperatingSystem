#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#include <dirent.h>
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <bits/stdc++.h>
#include <fcntl.h> 
#include <sys/stat.h>

#define BUF_SIZE 100
#define RES_BUF 50
#define NUM_ROWS_HEADER 1

using namespace std;

void print_final_results(vector<string> results){
    for(int i = 0 ; i < results.size() ; i++){
        cout << results[i] << "\n";
    }
}

vector<string> get_commands(char* fileName){
    string line;
    vector<string> commands;
    ifstream commandsFile(fileName);
    if(commandsFile.is_open()){
        while (getline (commandsFile,line)){
            commands.push_back(line);
        }
        commandsFile.close();
    }else{
        cout << "Unable to open commands file\n";
    }
    return commands;
}

vector<string> get_directory_folders(string directory){
    DIR *dir;
    struct dirent *ent;
    vector<string> folderNames;

    if ((dir = opendir (directory.c_str())) != NULL){
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type == DT_DIR){ // Folder Type
                if(strcmp(ent->d_name , ".") && strcmp(ent->d_name , "..")){
                    folderNames.push_back(ent->d_name);
                }  
            }
        }
        closedir(dir);
    }else{
    // could not open directory
        perror ("");
        folderNames.push_back("");
    }

    return folderNames;
}

vector<string> get_directory_csv(string directory){
    DIR *dir;
    struct dirent *ent;
    vector<string> csvNames;

    if ((dir = opendir (directory.c_str())) != NULL){
        while ((ent = readdir (dir)) != NULL) {
            if (ent->d_type == DT_REG){ // File Type
                csvNames.push_back(ent->d_name);
            }
        }
        closedir(dir);
    }else{
    // could not open directory
        perror ("");
        csvNames.push_back("");
    }

    return csvNames;
}

vector<string> getCommandInfo(string singleCommand){
    vector<string> info;
    istringstream ss(singleCommand);
    string word;

    while (ss >> word){
        info.push_back(word);
    }
    return info;
}

vector<string> getCSVInfo(string csvFileAddress){
    fstream csvFile(csvFileAddress);
    vector<string> extracted; 
    stringstream ss_line;
    string line , info;
    int counter_line = 1;

    while(getline(csvFile , line)){
        ss_line.str(line);
        ss_line.clear();
        if(counter_line > NUM_ROWS_HEADER){
            while(getline(ss_line , info , ',')){
                extracted.push_back(info);
            }
        }
        counter_line++;
    }
    csvFile.close();
    return extracted; 
}

bool comapre_dates(string startDate , string FinalDate , string productDate){
    stringstream start(startDate); vector<int> vec_start;
    stringstream end(FinalDate); vector<int> vec_end;
    stringstream product(productDate); vector<int> vec_product;
    string temp;
    while(getline(start , temp , '/')){ // year month day
        vec_start.push_back(stoi(temp));
    }
    while(getline(end , temp , '/')){
        vec_end.push_back(stoi(temp));
    }
    while(getline(product , temp , '/')){
        vec_product.push_back(stoi(temp));
    }

    int productDateNum = (vec_product[0] * 10000) + (vec_product[1] * 100) + vec_product[2];
    int startDateNum = (vec_start[0] * 10000) + (vec_start[1] * 100) + vec_start[2];
    int endDateNum = (vec_end[0] * 10000) + (vec_end[1] * 100) + vec_end[2];

    if(productDateNum >= startDateNum && productDateNum <= endDateNum){
        return true;
    }
    return false;
}

string getCommandResult(string singleCommand , string csvFileAddress){
    vector<string> commandInfo = getCommandInfo(singleCommand);
    vector<string> csvInfo = getCSVInfo(csvFileAddress);
    int max = numeric_limits<int>::min() , min = numeric_limits<int>::max();

    if(!strcmp(commandInfo[0].c_str() , "MAX")){
        for(int i = 0 ; i < csvInfo.size() ; i++){
            if(i % 3 == 1){
                if(!strcmp(commandInfo[1].c_str() , csvInfo[i].c_str()) && comapre_dates(commandInfo[2] , commandInfo[3] , csvInfo[i - 1])){
                    if(stoi(csvInfo[i + 1]) > max){
                        max = stoi(csvInfo[i + 1]);
                    }
                }
            }
        }
        if(max == numeric_limits<int>::min()){
            return to_string(-1);
        }
        return to_string(max);
    }
    else if(!strcmp(commandInfo[0].c_str() , "MIN")){
        for(int i = 0 ; i < csvInfo.size() ; i++){
            if(i % 3 == 1){
                if(!strcmp(commandInfo[1].c_str() , csvInfo[i].c_str()) && comapre_dates(commandInfo[2] , commandInfo[3] , csvInfo[i - 1])){
                    if(stoi(csvInfo[i + 1]) < min){
                        min = stoi(csvInfo[i + 1]);
                    }
                }
            }
        }
        if(min == numeric_limits<int>::max()){
            return to_string(-1);
        }
        return to_string(min);
    }

    return to_string(-2); // Invalid command occured
}

string createShopProcesses(string singleCommand , string city_directory , string operation){
    vector<string> csvFiles = get_directory_csv(city_directory);
    int fd_child , fd_parent;
    string result;

    string myfifo = "/tmp/myfifo" + to_string(getpid());
    mkfifo(myfifo.c_str(), 0666);

    int max_parent = numeric_limits<int>::min() , min_parent = numeric_limits<int>::max(); 
    
    if(csvFiles.size() > 0){
        int fd[2 * csvFiles.size()];
        for (int i = 0; i < csvFiles.size(); i++) {
            if (pipe(&fd[2 * i]) == -1) { 
                fprintf(stderr, "Pipe Failed" ); 
                return to_string(-2); 
            }
        }
        for(int i = 0 ; i < csvFiles.size() ; i++){
            pid_t pid = fork();
            // Child
            if(pid == 0){
                char singleCommand[BUF_SIZE];
                close(fd[2 * i + 1]); // Close write for child i
                read(fd[2 * i] , singleCommand , BUF_SIZE);
                result = getCommandResult(singleCommand , city_directory + "/" + csvFiles[i]);

                // Named Pipe
                char sendingRes[RES_BUF];
                sprintf(sendingRes , "%s" , result.c_str());
                fd_child = open(myfifo.c_str(), O_WRONLY);
                // cout << "write: >" << sendingRes << " -- " << singleCommand << " ++ " << city_directory + "/" + csvFiles[i] << "\n";
                write(fd_child, sendingRes, RES_BUF);
                close(fd_child);
                exit(0);
            }
            // Parent
            else if(pid > 0){
                close(fd[2 * i]); // close read for parent
                write(fd[2 * i + 1] , singleCommand.c_str() , strlen(singleCommand.c_str()) + 1 );
                close(fd[2 * i + 1]);
            }
        }

        fd_parent = open(myfifo.c_str(),O_RDWR);

        for(int i = 0 ; i < csvFiles.size() ; i++){
            // Named Pipe Read Parent
            char received_res[RES_BUF];
            read(fd_parent, received_res, RES_BUF);
    
            // cout << "read: >" << received_res << " -- " << singleCommand << " ++ " << city_directory + "/" + csvFiles[i] << "\n";
            if(!strcmp(operation.c_str() , "MAX")){
                if(stoi(received_res) > max_parent && stoi(received_res) != -1){
                    max_parent = stoi(received_res);
                }
            }else if(!strcmp(operation.c_str() , "MIN")){
                if(stoi(received_res) < min_parent && stoi(received_res) != -1){
                    min_parent = stoi(received_res);
                }                    
            } 
        }

        close(fd_parent);


        for(int i = 0; i < csvFiles.size(); i++){
            wait(NULL);
        }

        if(!strcmp(operation.c_str() , "MAX")){
            if(max_parent == numeric_limits<int>::min()){
                return to_string(-1);
            }
            return to_string(max_parent);
        }
        if(min_parent == numeric_limits<int>::max()){
            return to_string(-1);
        }
        return to_string(min_parent);
    }
    return to_string(-1);    
}

string createCityProcesses(string singleCommand , string province_directory , string operation){
    vector<string> folders = get_directory_folders(province_directory);
    string result;
    int fd_child , fd_parent;

    string myfifo = "/tmp/myfifo" + to_string(getpid());
    mkfifo(myfifo.c_str(), 0666);

    int max_parent = numeric_limits<int>::min() , min_parent = numeric_limits<int>::max(); 
    
    if(folders.size() > 0){
        int fd[2 * folders.size()];
        for (int i = 0; i < folders.size(); i++) {
            if (pipe(&fd[2 * i]) == -1) { 
                fprintf(stderr, "Pipe Failed" ); 
                return to_string(-2); 
            }
        }

        for(int i = 0 ; i < folders.size() ; i++){
            pid_t pid = fork();
            // Child
            if(pid == 0){
                char singleCommand[BUF_SIZE];
                close(fd[2 * i + 1]); // Close write for child i
                read(fd[2 * i] , singleCommand , BUF_SIZE);
                result = createShopProcesses(singleCommand , province_directory + "/" + folders[i] , operation);

                // Named Pipe
                char sendingRes[RES_BUF];
                sprintf(sendingRes , "%s" , result.c_str());
                fd_child = open(myfifo.c_str(), O_WRONLY);
                write(fd_child, sendingRes, RES_BUF);
                close(fd_child);
                exit(0);
            }
            // Parent
            else if(pid > 0){
                close(fd[2 * i]); // close read for parent
                write(fd[2 * i + 1] , singleCommand.c_str() , strlen(singleCommand.c_str()) + 1 );
                close(fd[2 * i + 1]);              
            }
        }

        fd_parent = open(myfifo.c_str(),O_RDWR);

        for(int i = 0 ; i < folders.size() ; i++){
            // Named Pipe Read Parent
            char received_res[RES_BUF];
            read(fd_parent, received_res, RES_BUF);
            
            if(!strcmp(operation.c_str() , "MAX")){
                if(stoi(received_res) > max_parent && stoi(received_res) != -1){
                    max_parent = stoi(received_res);
                }
            }else if(!strcmp(operation.c_str() , "MIN")){
                if(stoi(received_res) < min_parent && stoi(received_res) != -1){
                    min_parent = stoi(received_res);
                }                    
            }         
        }

        close(fd_parent);

        for(int i = 0; i < folders.size(); i++){
            wait(NULL);
        }

        if(!strcmp(operation.c_str() , "MAX")){
            if(max_parent == numeric_limits<int>::min()){
                return to_string(-1);
            }
            return to_string(max_parent);
        }
        if(min_parent == numeric_limits<int>::max()){
            return to_string(-1);
        }
        return to_string(min_parent);          
    }
    return to_string(-1);
}

string createProvinceProcesses(string singleCommand , string assets_directory , string operation){
    string currentDir = "./" + assets_directory;
    vector<string> folders = get_directory_folders(currentDir);

    string result;
    int fd_child , fd_parent;

    string myfifo = "/tmp/myfifo" + to_string(getpid());
    mkfifo(myfifo.c_str(), 0666);

    int max_parent = numeric_limits<int>::min() , min_parent = numeric_limits<int>::max(); 
    
    if(folders.size() > 0){
        int fd[2 * folders.size()];
        for (int i = 0; i < folders.size(); i++) {
            if (pipe(&fd[2 * i]) == -1) { 
                fprintf(stderr, "Pipe Failed" ); 
                return to_string(-2); 
            }
        }

        for(int i = 0 ; i < folders.size() ; i++){
            pid_t pid = fork();
            // Child
            if(pid == 0){
                char singleCommand[BUF_SIZE];
                close(fd[2 * i + 1]); // Close write for child i
                read(fd[2 * i] , singleCommand , BUF_SIZE);
                result = createCityProcesses(singleCommand , currentDir + "/" + folders[i] , operation);
                
                // Named Pipe
                char sendingRes[RES_BUF];
                sprintf(sendingRes , "%s" , result.c_str());
                fd_child = open(myfifo.c_str(), O_WRONLY);
                write(fd_child, sendingRes, RES_BUF);
                close(fd_child);
                exit(0);
            }
            // Parent
            else if(pid > 0){
                close(fd[2 * i]); // close read for parent
                write(fd[2 * i + 1] , singleCommand.c_str() , strlen(singleCommand.c_str()) + 1 );
                close(fd[2 * i + 1]);               
            }
        }

        fd_parent = open(myfifo.c_str(),O_RDWR);

        for(int i = 0 ; i < folders.size() ; i++){
            // Named Pipe Read Parent
            char received_res[RES_BUF];
            read(fd_parent, received_res, RES_BUF);

            if(!strcmp(operation.c_str() , "MAX")){
                if(stoi(received_res) > max_parent && stoi(received_res) != -1){
                    max_parent = stoi(received_res);
                }
            }else if(!strcmp(operation.c_str() , "MIN")){
                if(stoi(received_res) < min_parent && stoi(received_res) != -1){
                    min_parent = stoi(received_res);
                }                    
            }
        }

        close(fd_parent);

        for(int i = 0; i < folders.size(); i++){
            wait(NULL);
        }

        if(!strcmp(operation.c_str() , "MAX")){
            if(max_parent == numeric_limits<int>::min()){
                return to_string(-1);
            }
            return to_string(max_parent);
        }
        if(min_parent == numeric_limits<int>::max()){
            return to_string(-1);
        }
        return to_string(min_parent);         
    }
    return to_string(-1);
}

void createCommandProcesses(vector<string> commands , string assets_directory){
    string result;
    int fd_child , fd_parent;

    string myfifo = "/tmp/myfifo" + to_string(getpid());
    mkfifo(myfifo.c_str(), 0666);

    int fd[2 * commands.size()];

    // Initializing the pipes
    for (int i = 0; i < commands.size(); i++) {
        if (pipe(&fd[2 * i]) == -1) { 
            fprintf(stderr, "Pipe Failed" ); 
            return; 
        }
    }

    for(int i = 0 ; i < commands.size() ; i++){
        pid_t pid = fork();
        // Child
        if(pid == 0){
            char singleCommand[BUF_SIZE];
            close(fd[2 * i + 1]); // Close write for child i
            read(fd[2 * i] , singleCommand , BUF_SIZE);
            result = createProvinceProcesses(singleCommand , assets_directory , getCommandInfo(singleCommand)[0]);
            result = result + "$" + to_string(i);

            // Named Pipe
            char sendingRes[RES_BUF];
            sprintf(sendingRes , "%s" , result.c_str());
            fd_child = open(myfifo.c_str(), O_WRONLY);
            write(fd_child, sendingRes, RES_BUF);
            close(fd_child);
            exit(0);
        }
        // Parent
        else if(pid > 0){
            close(fd[2 * i]); // close read for parent
            write(fd[2 * i + 1] , commands[i].c_str() , strlen(commands[i].c_str()) + 1 );
            close(fd[2 * i + 1]);
        }
    }

    vector<string> extracted;
    stringstream ss_line;
    string info;

    fd_parent = open(myfifo.c_str(),O_RDWR);

    for(int i = 0 ; i < commands.size() ; i++){
        // Named Pipe Read Parent
        char received_res[RES_BUF];
        read(fd_parent, received_res, RES_BUF);

        ss_line.str(received_res);
        ss_line.clear();
        while(getline(ss_line , info , '$')){
            extracted.push_back(info);
        }
        commands[stoi(extracted[1])] = extracted[0];
        extracted.clear();
    }

    close(fd_parent);    

    for(int i = 0; i < commands.size(); i++){
        wait(NULL);
    }

    print_final_results(commands);
}

int main(int argc, char *argv[]){
    if(argc == 3){
        createCommandProcesses(get_commands(argv[1]) , argv[2]);
    }else{
        cout << "Input Invalid!\n";
    }
}