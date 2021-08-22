#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h>

#define PORT 8000
#define SERVER_ADDRESS "127.0.0.1"

void get_input_msg(char* inputStr){
    int numBytes;
    do{
        numBytes = read(0, inputStr, 128);
        inputStr[numBytes - 1] = '\0';
    }while(numBytes < 2);
    
}

int main(int argc, char const *argv[]){ 
	int sock = 0;  
	char* message; 
	char buffer[1024] = {0}; 
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
		write(1 , "\n Socket creation error \n" , 26); 
		return -1; 
	}

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	if(inet_pton(AF_INET, SERVER_ADDRESS , &serv_addr.sin_addr) <= 0){ 
		write(1 , "\nInvalid address \n" , 19); 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 

	int valread;

	memset(buffer, 0, 1024);
	valread = read(sock , buffer, 1024); 
	write(1 , buffer , 1024);
	
	while(1){
		memset(buffer, 0, 1024);
		write(1 , "YOU: " , 6);
		get_input_msg(message);
		send(sock , message , strlen(message) , 0); 
	} 
	return 0; 
}
