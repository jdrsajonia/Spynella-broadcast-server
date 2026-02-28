#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
// #include <arpa/inet.h>
#include <string>
#include <csignal>

#include <list>
#include <mutex>
#include <thread>

std::string red_id="\033[31m";
std::string green_id="\033[92m";
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

std::list<int> clients;
std::mutex mtx;

std::string join_message_for(int client_fd){
    return green_id+std::to_string(client_fd)+" se ha unido al chat"+reset_id+"\n";
}



std::string exit_message_for(int client_fd, bool error=false){
    if (error){
        return red_id+std::to_string(client_fd)+" ha sufrido un error de conexion"+reset_id+"\n";

    }
    return red_id+std::to_string(client_fd)+" ha abandonado el chat"+reset_id+"\n";
}


void send_broadcast(int client_fd, std::string message){
    std::lock_guard<std::mutex> lock(mtx);
    for (int client : clients){
        if (client==client_fd){
            continue;
        }
        send(client, message.c_str(), message.size(), 0);
    }
}


void handle_client(int client_fd){
    while (true){            
        char buffer[1024]={};
        int bytes = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (bytes>0){
            std::cout<<buffer<<std::endl;
            std::string received(buffer);
            std::string msg=std::to_string(client_fd)+"@spynella.net: "+ received+"\n";
            send_broadcast(client_fd, msg);
        }

        else if (bytes==0){
            {
                std::lock_guard<std::mutex> lock(mtx);
                clients.remove(client_fd);
            }
            
            std::string msg_exit=exit_message_for(client_fd);
            std::cout<<msg_exit;
            send_broadcast(-1, msg_exit);
            close(client_fd);
            break;
        }
        else{
            {
                std::lock_guard<std::mutex> lock(mtx);
                clients.remove(client_fd);
            } 
            
            std::string msg_exit_err=exit_message_for(client_fd, true);
            std::cerr<<msg_exit_err;
            send_broadcast(-1, msg_exit_err);
            close(client_fd);
            break;
        }
    }
}



int main(){

    int server_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd<0){
        std::cerr<<"Error al crear el socket";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_port=htons(1234);
    address.sin_addr.s_addr=INADDR_ANY;
    int opt=1;
    setsockopt(server_fd, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt));
    
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

        sockaddr_in client_address{};
        socklen_t addr_len=sizeof(client_address);
        int client_fd=accept(server_fd, (sockaddr*)&client_address, &addr_len);
        if (client_fd<0){
            std::cerr<<"Error al conectar con el cliente";
            return 1;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            clients.push_front(client_fd);
        }
        
        std::string msg_join=join_message_for(client_fd);
        std::cout<<msg_join;
        send(client_fd, server_banner.c_str(), server_banner.size(), 0);
        send_broadcast(-1, msg_join);
        
        std::thread client_thread(handle_client, client_fd);
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}