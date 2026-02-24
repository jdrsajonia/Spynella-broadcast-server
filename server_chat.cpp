#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <csignal>

std::string red_id="\033[31m";
std::string blue_id="\033[34m";
std::string reset_id="\033[0m";

std::string server_banner=
"                                       \n"
"                                       \n" +
reset_id + u8"▄█████ ▄▄▄▄  ▄▄ ▄▄ " + red_id + u8"▄▄  ▄▄ ▄▄▄▄▄ ▄▄▄▄▄▄\n" + reset_id +
reset_id + u8"▀▀▀▄▄▄ ██▄█▀ ▀███▀ " + red_id + u8"███▄██ ██▄▄    ██  \n" + reset_id +
reset_id + u8"█████▀ ██      █   " + red_id + u8"██ ▀██ ██▄▄▄   ██  \n" + reset_id +
"                                       \n"+
blue_id+"Made by: jdsajonia & Spynella\n\n"+reset_id;



int main(){

    int server_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd<0){
        std::cerr<<"Error al crear el socket";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_port=htons(1234);
    inet_pton(AF_INET, "192.168.1.14", &address.sin_addr);
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt));
    // address.sin_addr.s_addr=INADDR_ANY;
    
    

    int bind_state = bind(server_fd, (sockaddr*)&address, sizeof(address));
    if (bind_state<0){
        std::cerr<<"Error al unir el socket con la dirección IP";
        return 1;
    }

    int listen_state=listen(server_fd, 5);
    if (listen_state<0){
        std::cerr<<"Error al escuchar en el puerto 1234";
        return 1;
    }

    std::cout<<"Escuchando en el puerto 1234...\n";

    while (true){

        socklen_t addr_len=sizeof(address);
        int client_fd=accept(server_fd, (sockaddr*)&address, &addr_len);
        if (client_fd<0){
            std::cerr<<"Error al conectar con el cliente";
            return 1;
        }
        
        std::cout<<"Alguien se ha unido al chat\n";
        

        // std::string msg="Hey! esto es una prueba de chat por terminalees";
        send(client_fd, server_banner.c_str(), server_banner.size(), 0);
        //close(client_fd);



        while (true){            
            char buffer[1024]={};
            
            int bytes = recv(client_fd, buffer, sizeof(buffer)-1, 0);
            if (bytes>0){
                std::cout<<buffer<<std::endl;
                std::string received(buffer);
                std::string msg="tu respuesta fue: "+ received;
                send(client_fd, msg.c_str(), msg.size(), 0);

            }
            else if (bytes==0){
                close(client_fd);
                break;
            }

        

        }
        
        
        close(client_fd);


    }

    close(server_fd);




    return 0;
}