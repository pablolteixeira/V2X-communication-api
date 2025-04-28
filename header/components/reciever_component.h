#ifndef RECIEVER_COMPONENT_H
#define RECIEVER_COMPONENT_H

#include "component.h"
#include "vehicle.h"

// Message receiver component
class MessageReceiver : public Component {
    public:
        MessageReceiver(const std::string& name, EthernetCommunicator* comm) 
            : Component(name), _communicator(comm), _msg_count(0) {}
        
        int message_count() const { return _msg_count; }
    
    protected:
        void run() override {
            std::cout << "MessageReceiver " << _name << " started\n";
            
            Message msg(1024); // Buffer to hold incoming messages
            
            while (_running && running) {
                if (_communicator->receive(&msg)) {
                    _msg_count++;
                    
                    // Process message based on type
                    MessageType* type_ptr = msg.get_data<MessageType>();
                    switch(*type_ptr) {
                        case SENSOR_DATA:
                            process_sensor(msg.get_data<SensorMessage>());
                            break;
                        default:
                            std::cout << "Unknown message type: " << *type_ptr << std::endl;
                    }
                }
            }
            
            std::cout << "MessageReceiver " << _name << " stopped\n";
            std::cout << "Received " << _msg_count << " messages in total\n";
        }
        
        void process_sensor(SensorMessage* msg) {
            // Process sensor data - typically would update a world model
            // For this demo, we'll just log it occasionally
            if (_msg_count % 150 == 0) {
                std::cout << "\nReceived sensor data from ";
                switch(msg->sensor_type) {
                    case SensorMessage::LIDAR: std::cout << "LIDAR"; break;
                    case SensorMessage::CAMERA: std::cout << "CAMERA"; break;
                    case SensorMessage::RADAR: std::cout << "RADAR"; break;
                    case SensorMessage::ULTRASONIC: std::cout << "ULTRASONIC"; break;
                }
                std::cout << " - first reading: " << msg->readings[0] << "\n";
            }
        }
    
    private:
        EthernetCommunicator* _communicator;
        std::atomic<int> _msg_count;
    };

#endif