// #include <unistd.h> 
// #include <stdio.h> 
// #include <sys/socket.h> 
// #include <stdlib.h> 
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <string.h>

// #define SERVER_PORT 8000 
// #define SERVER_ADDRESS "127.0.0.1"

// void get_input_msg(char* inputStr){
//     int numBytes;
//     do 
//     {
//         numBytes = read(0, inputStr, 128);
//         inputStr[numBytes - 1] = '\0';
//     }while(numBytes < 2);
    
// }

// int main(int argc, char const *argv[]){ 
//     int server_fd; 
//     char buffer[1024] = {0}; 
//     char *server_msg; 
       
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
//         write(1 , "socket failed" , 14); 
//         exit(EXIT_FAILURE); 
//     } 
       
//     int opt = 1;    
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
//         write(1 , "setsockopt" , 12); 
//         exit(EXIT_FAILURE); 
//     } 

//     struct sockaddr_in address;
//     int addrlen = sizeof(address);
//     address.sin_family = AF_INET; 
//     address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS); 
//     address.sin_port = htons(SERVER_PORT); 
       
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) { 
//         write(1 , "bind failed" , 12); 
//         exit(EXIT_FAILURE); 
//     }

//     int new_socket , valread;

//     if (listen(server_fd, 3) < 0){ 
//         write(1 , "listen" , 7); 
//         exit(EXIT_FAILURE); 
//     }

//     if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){ 
//         write(1 , "accept" , 7); 
//         exit(EXIT_FAILURE); 
//     }

//     while(1){
//         write(1 , "Client: " , 9);
//         valread = read(new_socket , buffer, 1024); 
//         write(1 , buffer , strlen(buffer));
//         write(1 , "\nYOU: " , 7);
//         get_input_msg(server_msg);
//         send(new_socket , server_msg , strlen(server_msg) , 0);
//     }
//     return 0; 
// }

 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
    
#define SERVER_ADDRESS "127.0.0.1" 
#define PORT 8000
#define MAX_CLIENTS 10 


static char *itoa_recursive(char *dest, int i) {
  if (i <= -10){
    dest = itoa_recursive(dest, i/10);
  }
  *dest++ = '0' - i%10;
  return dest;
}

char *itoa_main(char *dest, int i) {
  char *s = dest;
  if (i < 0) {
    *s++ = '-';
  } else {
    i = -i;
  }
  *itoa_recursive(s, i) = '\0';
  return dest;
}
    
int main(int argc , char *argv[]) 
{ 
    int opt = 1; 
    int main_socket , new_socket;
        
    char buffer[1025]; 
    
    int client_socket[MAX_CLIENTS]; 
    for (int i = 0; i < MAX_CLIENTS; i++){ 
        client_socket[i] = 0; 
    }
         
    if((main_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){ 
        write(1 , "Socket failed!" , 15); 
        exit(EXIT_FAILURE); 
    } 
    
    if(setsockopt(main_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){ 
        write(1 , "setsockopt" , 12); 
        exit(EXIT_FAILURE); 
    } 
    
    struct sockaddr_in address;
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = inet_addr(SERVER_ADDRESS); 
    address.sin_port = htons( PORT ); 
         
    if (bind(main_socket, (struct sockaddr *)&address, sizeof(address)) < 0){ 
        write(1 , "bind failed" , 12); 
        exit(EXIT_FAILURE); 
    }

    char port_s[256]; itoa_main(port_s , PORT);
    write(1 , "Listener on port ", 18);
    write(1 , port_s , strlen(port_s));
    write(1 , "\n" , 2);
    
    //Maximum of 3 pending connections 
    if (listen(main_socket, 3) < 0){ 
        write(1 , "listen" , 7); 
        exit(EXIT_FAILURE); 
    } 
        
    int addrlen = sizeof(address); 
    write(1 , "Waiting for connections ..." , 28);
    write(1 , "\n" , 2); 
    
    int max_sd , sd , valread;
    fd_set readfds;

    char new_socket_s[256], addr_port_s[256], i_s[256];

    while(1) 
    { 
        FD_ZERO(&readfds); 
    
        FD_SET(main_socket, &readfds); 
        max_sd = main_socket; 
            
        for (int i = 0 ; i < MAX_CLIENTS ; i++){  
            sd = client_socket[i]; 
                 
            if(sd > 0){
                FD_SET(sd , &readfds); 
            }
                 
            if(sd > max_sd){
                max_sd = sd; 
            }
        } 
    
        int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno != EINTR)){ 
            write(1 , "select error" , 13); 
        } 
 
        if (FD_ISSET(main_socket, &readfds)) 
        { 
            if ((new_socket = accept(main_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){ 
                write(1 , "accept" , 7); 
                exit(EXIT_FAILURE); 
            } 
            
            write(1 , "--------------------\n" , 21); 
            write(1 , "New connection , socket fd is " , 31);
            write(1 , itoa_main(new_socket_s , new_socket) , strlen(itoa_main(new_socket_s , new_socket)));
            write(1 , " , ip is: " , 11);
            write(1 , inet_ntoa(address.sin_addr) , strlen(inet_ntoa(address.sin_addr)));
            write(1 , " , port : " , 11);
            write(1 , itoa_main(addr_port_s , ntohs(address.sin_port)) , strlen(itoa_main(addr_port_s , ntohs(address.sin_port))));
            write(1 , "\n" , 2); 
         
            if(send(new_socket, "You're now live! \r\n", strlen("You're now live! \r\n"), 0) != strlen("You're now live! \r\n")){ 
                write(1 , "send" , 5); 
            } 
                 
            for (int i = 0; i < MAX_CLIENTS; i++){ 
                if( client_socket[i] == 0){ 
                    client_socket[i] = new_socket; 
                    write(1 , "Adding to list of sockets as " , 30);
                    write(1 , itoa_main(i_s , i) , strlen(itoa_main(i_s , i)));
                    write(1 , "\n" , 2);
                    write(1 , "--------------------\n" , 22); 
                    break; 
                } 
            } 
        }   
         
        for (int i = 0; i < MAX_CLIENTS; i++){ 
            sd = client_socket[i]; 
            if (FD_ISSET(sd , &readfds)){
                if ((valread = read( sd , buffer, 1024)) == 0){ 
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    write(1 , "++++++++++++++++++++\n" , 21);
                    write(1 , "Client disconnected , ip " , 24);
                    write(1 , inet_ntoa(address.sin_addr) , strlen(inet_ntoa(address.sin_addr)));
                    write(1 , " , port " , 9);
                    write(1 , itoa_main(addr_port_s , ntohs(address.sin_port)) , strlen(itoa_main(addr_port_s , ntohs(address.sin_port))));
                    write(1 , "\n" , 2);
                    write(1 , "++++++++++++++++++++\n" , 21);  
                        
                    close(sd); 
                    client_socket[i] = 0; 
                }   
                else{ 
                    buffer[valread] = '\0';
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    write(1 , "CLIENT port " , 13);
                    write(1 , itoa_main(addr_port_s , ntohs(address.sin_port)) , strlen(itoa_main(addr_port_s , ntohs(address.sin_port))));
                    write(1 , ": " , 3);
                    write(1 , buffer , strlen(buffer));
                    write(1 , "\n" , 2);   
                } 
            } 
        } 
    } 
        
    return 0; 
} 
