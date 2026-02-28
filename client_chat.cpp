#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <csignal>


// #include <mutex>
#include <thread>

int client_fd;

void handle_exit(int signal){
    std::cout<<"Cerrando el cliente\n";
    close(client_fd);
    exit(0);

}

void receive_messages(){
    while (true){
        char buffer[2048]={};
        int bytes=recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (bytes>0){
            std::cout<<buffer;
        }
        else{
            std::cerr<<"Error al recibir datos";
            return;
        }
    }
}

int main(){
    signal(SIGINT, handle_exit);

    client_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd<0){
        std::cerr<<"Error al crear el socket del cliente";
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(1234);
    inet_pton(AF_INET, "192.168.1.14", &server_address.sin_addr);
    

    int connect_state=connect(client_fd,(sockaddr*) &server_address, sizeof(server_address));
    if (connect_state<0){
        std::cerr<<"Error al conectarse al servidor";
        return 1;
    }

    
    
    std::thread receive_thread(receive_messages);
    receive_thread.detach();

    while (true){
        std::string msg;
        // std::cout<<">> ";
        getline(std::cin, msg);
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
    
    return 0;
}