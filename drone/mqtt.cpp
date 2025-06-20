#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>

#include "mqtt/async_client.h" // Da biblioteca Paho

// TODO: ALTERAR NOMES DAS FUNCOES PARA CAMEL CASE
// TODO:
class MqttManager : public virtual mqtt::callback {
    mqtt::async_client client;
    std::function<void(const std::string&)> on_message_callback;

    void message_arrived(mqtt::const_message_ptr msg) override {
        if (on_message_callback) {
            on_message_callback(msg->get_payload_str());
        }
    }

public:
    MqttManager(const std::string& server_address) : client(server_address, "") {
        client.set_callback(*this);
    }

    void connect_and_subscribe(const std::string& topic) {
        try {
            std::cout << "Conectando ao broker MQTT..." << std::endl;
            client.connect()->wait();
            std::cout << "Conectado! Inscrevendo-se no topico: " << topic << std::endl;
            client.subscribe(topic, 1);
        } catch (const mqtt::exception& exc) {
            std::cerr << "Erro: " << exc.what() << std::endl;
            exit(1);
        }
    }

    void publish(const std::string& topic, const std::string& payload) {
        client.publish(topic, payload);
    }

    void set_on_message(std::function<void(const std::string&)> callback) {
        on_message_callback = callback;
    }
};