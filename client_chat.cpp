#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string>
#include <csignal>
#include <mutex>
#include <thread>
#include <termios.h>

int client_fd;

std::string saved_input_buffer;
std::mutex cout_mutex;

termios global_termios_config;


void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &global_termios_config);
}


void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &global_termios_config);
    atexit(disable_raw_mode);
    termios raw = global_termios_config;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


void cout_in_client_raw(char *buffer_received){
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::string out;
    out+="\r\033[2K";
    out+=buffer_received;
    if (out.back()!='\n'){
        out+='\n';
    }
    out+=">> ";
    out+=saved_input_buffer;
    write(STDOUT_FILENO, out.c_str(), out.size());
}


void rewrite_input(){
    std::string output="\r\033[2K>> "+saved_input_buffer;
    write(STDOUT_FILENO, output.c_str(), output.size());
}


void handle_exit(int signal){
    std::cout<<"Cerrando el cliente\n";
    close(client_fd);
    disable_raw_mode();
    exit(0);

}


void receive_messages(){
    while (true){
        char buffer[2048]={};
        int bytes=recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (bytes>0){
            cout_in_client_raw(buffer);
        }
        else{
            write(STDERR_FILENO, "Error al recibir datos\n", 23);
            return;
        }
    }
}


int main(){
    signal(SIGINT, handle_exit);

    client_fd=socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd<0){
        write(STDERR_FILENO, "Error al crear el socket del cliente\n", 38);
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(1234);
    inet_pton(AF_INET, "192.168.1.14", &server_address.sin_addr);
    
    int connect_state=connect(client_fd,(sockaddr*) &server_address, sizeof(server_address));
    if (connect_state<0){
        write(STDERR_FILENO, "Error al conectarse al servidor\n", 33);
        return 1;
    }

    enable_raw_mode();
    std::thread receive_thread(receive_messages);
    receive_thread.detach();

    write(STDOUT_FILENO, ">> ", 3);
    char c;

    while (true){

        read(STDIN_FILENO, &c, 1);
        std::lock_guard<std::mutex> lock(cout_mutex);

        if (c=='\n'){
            write(STDOUT_FILENO, "\n", 1);
            if (!saved_input_buffer.empty()){ // send something just if input contains characters
                send(client_fd, saved_input_buffer.c_str(), saved_input_buffer.size(), 0); 
                saved_input_buffer.clear();
            }
            write(STDOUT_FILENO, ">> ", 3);
        }

        else if (c==127 || c==8){
            if (!saved_input_buffer.empty()){
                saved_input_buffer.pop_back();
                rewrite_input();
            }
        }

        else if (c>=32){
            saved_input_buffer+=c;
            write(STDOUT_FILENO, &c, 1);
        }

        // else if (c==) handle arrow keys to move backward and foward
    }

    disable_raw_mode();
    return 0;
}