#ifndef INTEREST_TABLE_H
#define INTEREST_TABLE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <sstream>
#include "../header/type_definitions.h"

// Structure to hold interest information
struct InterestRecord {
    Origin origin;                              // MAC address and port
    ComponentDataType type;                             // Interest type
    InterestBroadcastType broadcast_type;
    std::chrono::microseconds period;          // Period of the interest
    std::chrono::system_clock::time_point last_updated; // Timestamp of last update
    
    // Constructor
    InterestRecord(Origin& org, ComponentDataType t, std::chrono::microseconds p, InterestBroadcastType bc_t)
        : origin(org), type(t), period(p), broadcast_type(bc_t), 
          last_updated(std::chrono::system_clock::now()) {}
};

// Class to manage interest registrations
class InterestTable {
private:
    // Separate registries for internal and external interests
    std::unordered_map<std::string, InterestRecord> registry;
    
    // Helper function to create unique key
    std::string create_key(Origin& origin, ComponentDataType type) {
        std::stringstream ss;
        ss << Ethernet::address_to_string(origin.mac);
        ss << ":" << static_cast<int>(origin.port) << ":" << type;
        return ss.str();
    }
    
    // Helper to get the appropriate registry
    std::unordered_map<std::string, InterestRecord>& get_registry() {
        return registry;
    }
    
    const std::unordered_map<std::string, InterestRecord>& get_registry() const {
        return registry;
    }
    
public:
    // Register or update an interest
    void register_interest(Origin& origin, ComponentDataType type, std::chrono::microseconds period, bool is_internal) {
        std::string key = create_key(origin, type);
        auto& registry = get_registry();
        
        InterestBroadcastType broadcast_type = is_internal ? InterestBroadcastType::INTERNAL : InterestBroadcastType::EXTERNAL;

        auto it = registry.find(key);
        if (it != registry.end()) {
            // Update existing record
            it->second.period = period;
            it->second.last_updated = std::chrono::system_clock::now();
        } else {
            // Insert new record
            registry.emplace(key, InterestRecord(origin, type, period, broadcast_type));
        }
    }
    
    // Get all interests for internal or external
    std::vector<InterestRecord> get_interests() {
        std::vector<InterestRecord> results;
        auto& registry = get_registry();
        
        for (const auto& [key, record] : registry) {
            results.push_back(record);
        }
        
        return results;
    }
    
    // Calculate GCD of all periods for internal or external
    std::chrono::microseconds calculate_gcd_period() {
        auto interests = get_interests();
        
        if (interests.empty()) {
            return std::chrono::microseconds(0);
        }
        
        // Start with the first period
        auto gcd_period = interests[0].period.count();
        
        // Calculate GCD with all other periods
        for (size_t i = 1; i < interests.size(); ++i) {
            gcd_period = std::__gcd(gcd_period, interests[i].period.count());
        }
        
        return std::chrono::microseconds(gcd_period);
    }
    
    // Get specific interest record
    InterestRecord* get_interest(Origin& origin, ComponentDataType type) {
        std::string key = create_key(origin, type);
        auto& registry = get_registry();
        auto it = registry.find(key);
        
        if (it != registry.end()) {
            return &it->second;
        }
        
        return nullptr;
    }
    
    // Remove old interests (cleanup)
    void cleanup_old_interests(std::chrono::seconds timeout = std::chrono::seconds(300)) {
        auto now = std::chrono::system_clock::now();
        
        // Cleanup registry
        auto it = registry.begin();
        while (it != registry.end()) {
            if (now - it->second.last_updated > timeout) {
                it = registry.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Clear all registrations
    void clear() {
        registry.clear();
    }
    
    // Get registry sizes
    size_t size() const {
        return registry.size();
    }
};

#endif //INTEREST_TABLE_H