#include <iostream>
#include <array>
#include <random>
#include <iomanip>
#include <chrono>        
#include <thread>
#include "drone_functions.cpp"


class Drone {
private:
    std::array<double, 2> position;
    std::array<double, 2> antena_position;
    float angle;
    int mqtt_channel;
    bool reached_antena;
    

public:
    Drone(int mqtt_channel) {
        position = genInitRandPosition();
        antena_position = {0.0,0.0}
        angle = genInitRandAngle();
        this->mqtt_channel = mqtt_channel;
        reached_antena = false;
    }

    std::array<double, 2> getPosition() {
        return this->position;
    }

    void setPosition(std::array<double, 2> position) {
        this->position = position;
    }

    std::array<double, 2> getAntenaPosition() {
        return this->position;
    }

    void setAntenaPosition(std::array<double, 2> antena_position) {
        this->antena_position = antena_position;
    }

    int getAngle() {
        return this->angle;
    }

    void setAngle(float angle) {
        this->angle = angle;
    }

    int getMqttChannel() {
        return this->mqtt_channel;
    }

    void setMqttChannel(int mqtt_channel) {
        this->mqtt_channel = mqtt_channel;
    }

    bool getReachedAntena() {
        return this->reached_antena;
    }
    
    void setReachedAntena(bool reached_antena) {
        this->reached_antena = reached_antena;
    }
    
    // Anda uma quantidade X
    void walk() {

    }

    // Envia uma mensagem pedindo para iniciar e aguarda a resposta da antena
    void waitAntenaSync() {
        // Envia mensagem MQTT para acentral com uma mensagem "Pronto"
        // while(msg_recebida[msg] != "Ok") {
        // Fica lendo buffer de recebindo de msg
        // }
        // setAntenaPosition(msg_cebida[position])
    }

    // Verifica se esta na mesma posição da antena
    bool checkEnd() {
        if (getPosition() == getAntenaPosition()){
            setReachedAntena(true);
        }
        return getReachedAntena();
    }

    // Anda e verifica se esta na mesma posicao da antena
    void walkAndCheckEnd() {
        walk();
        checkEnd();
    }

    // Envia msg MQTT para a central
    void sendMessage() {

    }

    // Recebe mensagem MQTT da central
    void receiveMessage() {

    }

    // Recebe a mensagem e atualiza o angulo corrigido
    void receiveAndUpdate() {

    }

    // Inicia o Drone
    void start() {

        waitAntenaSync();

        while(not getReachedAntena()){

            walkAndCheckEnd();
            sendMessage();
            std::this_thread::sleep_for(std::chrono::seconds(1));

            receiveAndUpdate();
            walkAndCheckEnd();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }
};


int main() {
    Drone d(13);
    d.start();
}