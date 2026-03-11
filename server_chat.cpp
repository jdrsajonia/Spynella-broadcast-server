#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <sstream>
#include <csignal>
#include <random>
#include <chrono>
#include <ctime>

#include <functional>
#include <unordered_map>
#include <list>
#include <vector>

#include <mutex>
#include <thread>

const std::string RED          = "\033[31m";
const std::string RED_BRIGHT   = "\033[91m";
const std::string GREEN        = "\033[32m";
const std::string GREEN_BRIGHT = "\033[92m";
const std::string BLUE         = "\033[34m";
const std::string BLUE_BRIGHT  = "\033[94m";
const std::string CYAN         = "\033[36m";
const std::string CYAN_BRIGHT  = "\033[96m";
const std::string MAGENTA      = "\033[35m";
const std::string MAGENTA_BRIGHT = "\033[95m";
const std::string YELLOW       = "\033[33m";
const std::string YELLOW_BRIGHT= "\033[93m";
const std::string PINK         = "\033[38;2;255;105;180m";
const std::string PINK_LIGHT   = "\033[38;2;255;182;193m";
const std::string PINK_DEEP    = "\033[38;2;215;45;95m";
const std::string PINK_BRIGHT  = "\033[38;2;232;74;124m";  // Rosa Brillante Espinela
const std::string WHITE        = "\033[97m";
const std::string GRAY         = "\033[90m";
const std::string RESET        = "\033[0m";




const std::vector<std::string> COLORS = {
    RED, RED_BRIGHT,
    GREEN, GREEN_BRIGHT,
    BLUE, BLUE_BRIGHT,
    CYAN, CYAN_BRIGHT,
    MAGENTA, MAGENTA_BRIGHT,
    YELLOW, YELLOW_BRIGHT,
    WHITE, GRAY,
    PINK, PINK_LIGHT, PINK_DEEP, PINK_BRIGHT
};


const std::string server_banner=
"                                       \n"
"                                       \n" +                                                                                                    
WHITE+u8"▄█████ ▄▄▄▄  ▄▄ ▄▄ "+PINK_BRIGHT+"▄▄  ▄▄ ▄▄▄▄▄ ▄▄    ▄▄     ▄▄▄ \n"+WHITE+ 
WHITE+u8"▀▀▀▄▄▄ ██▄█▀ ▀███▀ "+PINK_BRIGHT+"███▄██ ██▄▄  ██    ██    ██▀██\n"+WHITE+ 
WHITE+u8"█████▀ ██      █   "+PINK_BRIGHT+"██ ▀██ ██▄▄▄ ██▄▄▄ ██▄▄▄ ██▀██\n"+WHITE+
"                                       \n"+
RED+"Made by: jdsajonia & Spynella\n\n"+RESET; 
                                                  



struct client{
    int client_fd;
    std::string str_ip;
    std::string nickname;
    std::string color="";

    bool operator==(const client& other) const { // se define que dos clientes son iguales si tienen el mismo fd
        return client_fd == other.client_fd;
    }
};



std::string join_message_for(const client &c);
std::string exit_message_for(const client &c, bool b);
std::string client_prompt_for(const client &c);
void send_broadcast_except(const client &c, std::string str);
void send_broadcast(std::string str);
void handle_client(client &c);
std::vector<std::string> parse_command(std::string str);
void exec_command_for(client &c, std::vector<std::string> tokens);
void help_command(const client &c);
void whoami_command(const client &c);
void current_clients_command(const client &c);
void set_nickname_command(client &c, std::string str);
std::string random_colors();
std::vector<std::string> get_timestamp();



std::list<client> available_clients;
std::mutex mtx;

std::unordered_map<std::string, std::function<void(client&, std::string)>> commands =
{
    {"nick", set_nickname_command},
    {"list", [](client &c, std::string) {current_clients_command(c);}}, // esta funcion recibe parametros extra, se le hace un lambda
    {"help", [](client &c, std::string) {help_command(c);}},
    {"whoami", [](client &c, std::string) {whoami_command(c);}}
};



//UTILS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string random_colors(){
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, COLORS.size()-1);
    std::string color=COLORS[dist(rng)];
    return color;
}

std::vector<std::string> get_timestamp(){
    std::vector<std::string> ddmmyy_hhmmss;

    auto now=std::chrono::system_clock::now();
    std::time_t time=std::chrono::system_clock::to_time_t(now);
    std::tm *tm=std::localtime(&time);

    char buffer_ddmmyy[32];
    char buffer_hhmmss[32];

    std::strftime(buffer_ddmmyy, sizeof(buffer_ddmmyy), "%d/%m/%Y", tm);
    std::strftime(buffer_hhmmss, sizeof(buffer_hhmmss), "%H:%M:%S", tm);

    ddmmyy_hhmmss.push_back(std::string(buffer_ddmmyy));
    ddmmyy_hhmmss.push_back(std::string(buffer_hhmmss));

    return ddmmyy_hhmmss;


}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END




//MESSAGES
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string join_message_for(const client &cli){
    return GREEN+std::to_string(cli.client_fd)+" se ha unido al chat"+WHITE+"\n";
}


std::string exit_message_for(const client &cli, bool error=false){
    if (error){
        return RED+std::to_string(cli.client_fd)+" ha sufrido un error de conexion"+WHITE+"\n";
    }
    return RED+std::to_string(cli.client_fd)+" ha abandonado el chat"+WHITE+"\n";
}


std::string client_prompt_for(const client &cli){
    return "["+cli.color+cli.nickname+RESET+"@"+cli.color+cli.str_ip+RESET+"]: " ;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END





//COMMANDS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void current_clients_command(const client &cli){
    std::string buffer="Clientes conectados!\n";
    {   
        std::lock_guard<std::mutex> lock(mtx);
        for (const client &current_cli : available_clients){
            buffer+=client_prompt_for(current_cli)+"\n";
        }
    }
    send(cli.client_fd, buffer.c_str(), buffer.size(), 0);
}


void set_nickname_command(client &cli, std::string new_nickname){
    if (new_nickname.size()>25 || new_nickname.size()==0){
        send(cli.client_fd, "NICKNAME INVALIDO: debe ser menor a 25 caracteres", 50, 0);
        return;
    }

    bool taken=false;
    std::string old_nickname;
    {
        std::lock_guard<std::mutex> lock(mtx);
        old_nickname=cli.nickname;
        for (client &current_cli : available_clients){
            if (current_cli.nickname==new_nickname){
                taken=true;
                break;
            }
        }
        if (!taken) cli.nickname=new_nickname;
        
    }
    if (taken){
        send(cli.client_fd, "NICKNAME INVALIDO: alquien ya lo tiene", 39, 0);
    }
    else{
        std::string changed_nickname_message=old_nickname+" ha cambiado su nickname a "+new_nickname;
        send_broadcast(changed_nickname_message);
    }
}


void help_command(const client &cli){
    std::string help_message=
    R"(
help: available commands
    
    /list: list current users connected to the server
    /nick <name>: set a nickname to your own user
    /whoami: displays who are you with your nickname and your ipv4 address
    /help: displays this info
        
        )";

    send(cli.client_fd, help_message.c_str(), help_message.size(), 0);

}


void whoami_command(const client &cli){
    std::string whoareyou="You are "+cli.color+cli.nickname+RESET+" with ip: "+cli.color+cli.str_ip+RESET+"\n";
    send(cli.client_fd, whoareyou.c_str(), whoareyou.size(), 0);
}


std::vector<std::string> parse_command(std::string message){
    std::vector<std::string> tokens;
    std::istringstream stream(message.substr(1));
    std::string token;

    while (stream >> token){
        tokens.push_back(token);
    }
    return tokens;
}


void exec_command_for(client &cli, std::vector<std::string> tokens){

    if (tokens.size()==0){
        //error
        return;
    }

    std::string cmd=tokens[0];

    if (commands.empty()){
        //call help messasge
        return;
    }

    if (!commands.count(cmd)){
        //call mising command
        // std::string msg= 

        // // send();

        return;
    }

    std::string arg = (tokens.size() > 1) ? tokens[1] : "";
    commands[cmd](cli, arg);    
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END



//CLIENT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void send_broadcast_except(const client &sender, std::string message){
    std::lock_guard<std::mutex> lock(mtx);
    for (const client &current_cli : available_clients){
        if (current_cli.client_fd==sender.client_fd){
            continue;
        }
        send(current_cli.client_fd, message.c_str(), message.size(), 0);
    }
}



void send_broadcast(std::string message){
    std::lock_guard<std::mutex> lock(mtx);
    for (const client &current_cli : available_clients){
        send(current_cli.client_fd, message.c_str(), message.size(), 0);
    }
}


void handle_client(client &cli){
    int client_fd=cli.client_fd;
    while (true){            
        char buffer[1024]={};
        int bytes = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (bytes>0){
            std::cout<<buffer<<std::endl;
            std::string received(buffer);

            if (buffer[0]=='/'){
                std::cout<<"COMANDO EXECUTED\n";
                std::vector<std::string> tokend_command = parse_command(buffer);
                exec_command_for(cli, tokend_command);
            }
            else{
                std::string time=get_timestamp()[1];
                std::string msg="["+time+"]"+client_prompt_for(cli)+ received+"\n";
                send_broadcast_except(cli, msg);
            }
        }

        else if (bytes==0){
            {
                std::lock_guard<std::mutex> lock(mtx);
                available_clients.remove(cli);
            }
            
            std::string msg_exit=exit_message_for(cli);
            std::cout<<msg_exit;
            send_broadcast(msg_exit);
            close(client_fd);
            break;
        }
        else{
            {
                std::lock_guard<std::mutex> lock(mtx);
                available_clients.remove(cli);
            } 
            
            std::string msg_exit_err=exit_message_for(cli, true);
            std::cerr<<msg_exit_err;
            send_broadcast(msg_exit_err);
            close(client_fd);
            break;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//END



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

    std::cout<<server_banner<<std::endl;
    std::cout<<"Escuchando en el puerto [1234]:\n";


    while (true){

        sockaddr_in client_address{};
        socklen_t addr_len=sizeof(client_address);
        int client_fd=accept(server_fd, (sockaddr*)&client_address, &addr_len);
        if (client_fd<0){
            std::cerr<<"Error al conectar con el cliente";
            return 1;
        }

        client new_client;
        new_client.client_fd=client_fd;
        new_client.str_ip=inet_ntoa(client_address.sin_addr);
        new_client.nickname=std::to_string(client_fd)+"anonymous";
        new_client.color=random_colors();

        {
            std::lock_guard<std::mutex> lock(mtx);
            available_clients.push_front(new_client);
        }
        
        std::string msg_join=join_message_for(new_client); 
        std::cout<<msg_join;
        send(client_fd, server_banner.c_str(), server_banner.size(), 0); //?
        send_broadcast(msg_join);
        
        
        std::thread client_thread(handle_client, std::ref(available_clients.front())); //!
        client_thread.detach();
    }

    close(server_fd);
    return 0;
}