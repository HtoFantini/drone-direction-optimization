#include <iostream>
#include <array>
#include <random>
#include <iomanip>
#include "drone_class.cpp"

// Motor de randomização
std::mt19937& genRandMotor() {
    static std::mt19937 motor(std::random_device{}());
    return motor;
}

// Gera uma posição aleatoria
std::array<double, 2> genInitRandPosition() {
    std::uniform_real_distribution<double> distrib_latitude(-90.0,90.0);
    std::uniform_real_distribution<double> distrib_longitude(-180.0,180.0);

    double lat = distrib_latitude(genRandMotor());
    double lon = distrib_longitude(genRandMotor());

    return {lat, lon};
}

// Gera um angulo aleatorio
float genInitRandAngle() {
    std::uniform_real_distribution<float> distrib_angle(-180.0f,180.0f);
    return distrib_angle(genRandMotor());
}

// Cria o Drone com nome conforme o PID do processo e inicia sua execução
void startDrone() {
    Drone d(13);
    d.start();
}