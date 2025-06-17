#include <iostream>
#include <array>
#include <random>
#include <iomanip>


std::mt19937& genRandMotor() {
    static std::mt19937 motor(std::random_device{}());
    return motor;
}


std::array<double, 2> genInitRandPosition() {
    std::uniform_real_distribution<double> distrib_latitude(-90.0,90.0);
    std::uniform_real_distribution<double> distrib_longitude(-180.0,180.0);

    double lat = distrib_latitude(genRandMotor());
    double lon = distrib_longitude(genRandMotor());

    return {lat, lon};
}


float genInitRandAngle() {
    std::uniform_real_distribution<float> distrib_angle(-180.0f,180.0f);
    return distrib_angle(genRandMotor());
}


class Drone {
private:
    std::array<double, 2> position;
    float angle;
    int mqtt_channel;

public:
    Drone(int mqtt_channel) {
        position = genInitRandPosition();
        angle = genInitRandAngle();
        this->mqtt_channel = mqtt_channel;
    }

    std::array<double, 2> getPosition() {
        return this->position;
    }

    void setPosition(std::array<double, 2> position) {
        this->position = position;
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
    
};