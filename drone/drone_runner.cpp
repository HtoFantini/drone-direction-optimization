#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <string>

#include "drone_class.h"

// Quantidade de drones a serem lançados
const int NUM_DRONES = 5;


void drone_runner() {
    // Inicializa a biblioteca mosquitto em cada novo processo
    mosqpp::lib_init();

    // Gera um ID de cliente único para o MQTT usando o PID do processo
    std::string client_id = std::to_string(getpid());
    const char* host = "localhost";
    int port = 1883;

    std::cout << "[RUNNER] Iniciando Drone " << client_id << std::endl;

    // Cria a instância do Drone
    Drone drone(client_id.c_str(), host, port);

    drone.start();

    // Limpa os recursos da biblioteca ao finalizar
    mosqpp::lib_cleanup();

    std::cout << "[DRONE " << getpid() << "] Processo finalizado." << std::endl;
}


int main() {
    std::cout << "[MAIN] Lançando " << NUM_DRONES << " drones..." << std::endl;

    std::vector<pid_t> pids;

    // Loop para criar os processos filhos usando fork()
    for (int i = 0; i < NUM_DRONES; ++i) {
        pid_t pid = fork();

        if (pid < 0) {
            std::cerr << "Falha ao criar processo (fork)." << std::endl;
            return 1;
        } else if (pid == 0) {
            drone_runner();
            exit(0);
        } else {
            // Guarda o PID do filho
            pids.push_back(pid); 
        }
    }

    std::cout << "[MAIN] Todos os drones foram lançados. Aguardando finalização..." << std::endl;

    // O processo pai espera que todos os filhos terminem
    for (pid_t pid : pids) {
        waitpid(pid, NULL, 0);
    }

    std::cout << "[MAIN] Todos os processos de drone finalizaram. Encerrando." << std::endl;

    return 0;
}