#ifndef SENSOR_COMPONENT_H
#define SENSOR_COMPONENT_H

#include "component.h"
#include "vehicle.h"

// Sensor component
class Sensor : public Component {
    public:
        Sensor(const std::string& name, EthernetCommunicator* comm, 
               SensorMessage::SensorType type, int update_rate_ms) 
            : Component(name), _communicator(comm), _type(type), _update_rate_ms(update_rate_ms) {}
    
    protected:
        void run() override {
            std::cout << "Sensor " << _name << " started\n";
            
            Message msg(sizeof(SensorMessage));
            SensorMessage* data = msg.get_data<SensorMessage>();
            data->type = SENSOR_DATA;
            data->sensor_type = _type;
            
            while (_running && running) {
                // Generate simulated sensor data
                
                // Simulate different sensor readings
                for (int i = 0; i < 16; i++) {
                    // Generate some pseudo-random but consistent readings
                    data->readings[i] = sin(get_timestamp() * 0.001 + i * 0.5) * 10.0 + 20.0;
                }
                
                msg.size(sizeof(SensorMessage));
                _communicator->send(&msg);
                
                // Wait for next update cycle
                std::this_thread::sleep_for(std::chrono::milliseconds(_update_rate_ms));
            }
            
            std::cout << "Sensor " << _name << " stopped\n";
        }
    
    private:
        EthernetCommunicator* _communicator;
        SensorMessage::SensorType _type;
        int _update_rate_ms;
    };

#endif