#ifndef SENSOR_COMPONENT_H
#define SENSOR_COMPONENT_H

#include "component.h"
#include "../vehicle.h"

// Sensor component
class Sensor : public Component {
    public:
        Sensor(const std::string& id, 
               SensorMessage::SensorType type, int update_rate_ms) 
            : Component(id), _type(type), _update_rate_ms(update_rate_ms) {}
    
    protected:
        void run() override {
            std::cout << "Sensor " << _id << " started\n";
            
            Message msg(sizeof(SensorMessage));
            SensorMessage* data = msg.get_data<SensorMessage>();
            data->type = SENSOR_DATA;
            data->sensor_type = _type;
            
            while (_running) {
                // Generate simulated sensor data
                
                // Simulate different sensor readings
                for (int i = 0; i < 16; i++) {
                    // Generate some pseudo-random but consistent readings
                    //data->readings[i] = sin(get_timestamp_t() * 0.001 + i * 0.5) * 10.0 + 20.0;
                }
                
                msg.size(sizeof(SensorMessage));
                
                // Wait for next update cycle
                std::this_thread::sleep_for(std::chrono::milliseconds(_update_rate_ms));
            }
            
            std::cout << "Sensor " << _id << " stopped\n";
        }
    
    private:
        SensorMessage::SensorType _type;
        int _update_rate_ms;
    };

#endif