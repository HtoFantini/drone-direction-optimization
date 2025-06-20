#include <iostream>
#include <array>
#include <random>
#include <iomanip>
#include <chrono>        
#include <thread>
#include <cmath>
#include <unistd.h>

#include "drone_functions.cpp"
#include "mqtt.cpp"
#include "nlohmann/json.hpp"   // Para JSON


using json = nlohmann::json;


static const double WALK_DISTANCE = 10.0;
static const float MIN_DIST_TO_ANTENA = 2.0;


class Drone {
private:
    std::array<double, 2> position;
    std::array<double, 2> antena_position;
    float angle;
    bool reached_antena;

    // TODO: ATRIBUTOS ADICIONADOS SEM CHECAGEM (para o MQTT)
    pid_t pid;
    MqttManager mqtt;


public:
    Drone(const std::string& broker_address) : mqtt(broker_address) {
        this->position = genInitRandPosition();
        this->antena_position = {0.0,0.0};
        this->angle = genInitRandAngle();
        this->reached_antena = false;
        this->pid = getpid();

        // TODO: AQUI PRA BAIXO NAO ENTENDI NADA, TEM QUE LER
        // ACHO QUE AQUI VAI TER QUE JOGAR O CONSTRUTOR PRA BAIXO
        mqtt.set_on_message([this](const std::string& payload) {
            this->handle_incoming_message(payload);
        });

        std::string command_topic = "drone/" + std::to_string(pid) + "/para_drone";
        mqtt.connect_and_subscribe(command_topic);
    }

    std::array<double, 2> getPosition() {
        return this->position;
    }

    void setPosition(std::array<double, 2> position) {
        this->position = position;
    }

    std::array<double, 2> getAntenaPosition() {
        return this->antena_position;
    }

    void setAntenaPosition(std::array<double, 2> antena_position) {
        this->antena_position = antena_position;
    }

    float getAngle() {
        return this->angle;
    }

    void setAngle(float angle) {
        this->angle = angle;
    }

    bool getReachedAntena() {
        return this->reached_antena;
    }
    
    void setReachedAntena(bool reached_antena) {
        this->reached_antena = reached_antena;
    }
    
    // Anda uma quantidade X
    void walk() {

        // Aplica uma variacao ao angulo de visada com um valor sorteado dentro de um range
        // this->angle += uniform_real_distribution(de x ate z)

        // Converter o ângulo de graus para radianos
        double angle_rad = this->angle * M_PI / 180.0;

        // Calcular o deslocamento em X (longitude) e Y (latitude)
        double delta_x = WALK_DISTANCE * cos(angle_rad);
        double delta_y = WALK_DISTANCE * sin(angle_rad);

        // Calcular a nova posição somando o deslocamento à posição atual
        // Assumindo que position[0] = latitude (eixo Y) e position[1] = longitude (eixo X)
        std::array<double, 2> new_position = {
            this->position[0] + delta_y,
            this->position[1] + delta_x
        };

        // Atualizar a posição do drone
        this->setPosition(new_position);

        std::cout << "[Drone " << this->pid << "] Andou. "
                  << "Angulo: " << this->angle << " deg. "
                  << "Nova Posicao: (Lat: " << new_position[0] 
                  << ", Lon: " << new_position[1] << ")" << std::endl;
    }


    // Verifica se esta na mesma posição da antena
    bool checkEnd() {
        double dx = getPosition()[1] - getAntenaPosition()[1];
        double dy = getPosition()[0] - getAntenaPosition()[0];
        double distance_to_target = std::sqrt(dx*dx + dy*dy);
        
        // Verifica se a distancia do target esta dentro do range definido
        if (distance_to_target < MIN_DIST_TO_ANTENA) {
            this->reached_antena = true;
        }
        return this->reached_antena
    }

    // Anda e verifica se esta na mesma posicao da antena
    bool walkAndCheckEnd() {
        walk();
        return checkEnd();
    }

    // Envia msg MQTT para a central
    // TODO: APENAS COPIEI E COLEI, NAO CHEQUEI NADA
    void sendMessage() {
        json msg_json;
        msg_json["tipo"] = "posicao";
        msg_json["dados"]["lat"] = this->position[0];
        msg_json["dados"]["lon"] = this->position[1];
        msg_json["dados"]["angulo"] = this->angle;

        std::string topic = "drone/" + std::to_string(this->pid) + "/para_antena";
        std::string payload = msg_json.dump();

        std::cout << "[DRONE " << pid << "] Enviando: " << payload << " para o topico " << topic << std::endl;
        mqtt.publish(topic, payload);
    }

    // Recebe mensagem MQTT da central
    // TODO: APENAS COPIEI E COLEI, NAO CHEQUEI NADA
    json receiveMessage(const std::string& payload) {
        std::cout << "[DRONE " << pid << "] Mensagem Recebida: " << payload << std::endl;
        try {
            return json::parse(payload);
        } catch (json::parse_error& e) {
            std::cerr << "[DRONE " << pid << "] Erro ao parsear JSON: " << e.what() << '\n';
            // Retorna um JSON nulo em caso de erro para evitar que o programa quebre
            return json(); 
        }
    }

    // Recebe a mensagem e atualiza o angulo corrigido
    // TODO: ELA NAO ESTA RECEBENDO A MSG DIRETAMENTE. TEM QUE VERIFICAR SE ELE PERDE O FATOR ASSINCRONO CASO ISSO ACONTECA
    void receiveAndUpdate(const json& msg_json) {
        if (msg_json.is_null()) {
            return;
        }
        // Adicionando checagens de segurança para evitar erros se a chave não existir
        if (msg_json.contains("tipo") && msg_json.contains("acao") &&
            msg_json["tipo"] == "comando" && msg_json["acao"] == "corrigir_angulo") {
            
            this->angle = msg_json["valor"];
            std::cout << "[DRONE " << pid << "] Angulo corrigido para: " << this->angle << std::endl;
        }
    }

    void handle_incoming_message(const std::string& payload) {
        json parsed_message = receiveMessage(payload);
        receiveAndUpdate(parsed_message);
    }

    // Envia uma mensagem pedindo para iniciar e aguarda a resposta da antena
    void waitAntenaSync() {
        // Envia mensagem MQTT para acentral com uma mensagem "Pronto"
        // while(msg_recebida[msg] != "Ok") {
        // Fica lendo buffer de recebindo de msg
        // }
        // setAntenaPosition(msg_cebida[position])
    }


    // Inicia o Drone
    void start() {

        waitAntenaSync();

        while(not this->reached_antena){

            walkAndCheckEnd();
            sendMessage();
            std::this_thread::sleep_for(std::chrono::seconds(1));

            receiveAndUpdate();
            walkAndCheckEnd();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};

// Simulacao do funcionamento
int main() {
    Drone d(13);

    // Configura o ponto flutuante para mostrar 4 casas decimais na saída
    std::cout << std::fixed << std::setprecision(4);

    std::cout << "--- ESTADO INICIAL ---\n";
    std::cout << "Posicao Inicial: (Lat: " << d.getPosition()[0] << ", Lon: " << d.getPosition()[1] << ")\n";
    std::cout << "Angulo Inicial: " << d.getAngle() << " graus\n\n";

    d.walk();

    std::cout << "\n--- ESTADO FINAL ---\n";
    std::cout << "Posicao Final: (Lat: " << d.getPosition()[0] << ", Lon: " << d.getPosition()[1] << ")\n";

    return 0;
}