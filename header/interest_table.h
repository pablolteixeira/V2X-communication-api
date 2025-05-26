#ifndef INTEREST_TABLE_H
#define INTEREST_TABLE_H

#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <sstream>
#include "message.h"
#include "component_types.h"

// Structure to hold interest information
struct InterestRecord {
    Origin origin;                              // MAC address and port
    ComponentDataType type;                             // Interest type
    std::chrono::microseconds period;          // Period of the interest
    std::chrono::system_clock::time_point last_updated; // Timestamp of last update
};

// Class to manage interest registrations
class InterestTable {
private:
    // Separate registries for internal and external interests
    std::unordered_map<std::string, InterestRecord*> internal_table;
    std::unordered_map<std::string, InterestRecord*> external_table;
    
    // Helper function to create unique key
    std::string create_key(const Origin& origin, ComponentDataType type) {
        std::stringstream ss;
        // Convert MAC to string
        for (int i = 0; i < 6; ++i) {
            ss << std::hex << static_cast<int>(origin.mac[i]);
            if (i < 5) ss << ":";
        }
        ss << ":" << static_cast<int>(origin.port) << ":" << static_cast<int>(type);
        return ss.str();
    }
    
    // Helper to get the appropriate table
    std::unordered_map<std::string, InterestRecord*>& get_table(bool is_internal) {
        return is_internal ? internal_table : external_table;
    }
    
    const std::unordered_map<std::string, InterestRecord*>& get_table(bool is_internal) const {
        return is_internal ? internal_table : external_table;
    }
    
public:
    // Register or update an interest
    void register_interest(const Origin& origin, ComponentDataType type, std::chrono::microseconds period, bool is_internal) {
        std::string key = create_key(origin, type);
        auto& table = get_table(is_internal);
        
        auto it = table.find(key);
        if (it != table.end()) {
            // Update existing record
            it->second->period = period;
            it->second->last_updated = std::chrono::system_clock::now();
        } else {
            InterestRecord* int_rec = new InterestRecord;
            int_rec->origin = origin;
            int_rec->type = type;
            int_rec->period = period;
            int_rec->last_updated = std::chrono::system_clock::now();
            // Insert new record
            table.emplace(key, int_rec);
        }
    }
    
    // Get all interests for internal or external
    std::vector<InterestRecord*> get_interests(bool is_internal) {
        std::vector<InterestRecord*> results;
        const auto& table = get_table(is_internal);
        
        for (const auto& [key, record] : table) {
            results.push_back(record);
        }
        
        return results;
    }
    
    // Calculate GCD of all periods for internal or external
    std::chrono::microseconds calculate_gcd_period(bool is_internal) {
        auto interests = get_interests(is_internal);
        
        if (interests.empty()) {
            return std::chrono::microseconds(0);
        }
        
        // Start with the first period
        auto gcd_period = interests[0]->period.count();
        
        // Calculate GCD with all other periods
        for (size_t i = 1; i < interests.size(); ++i) {
            gcd_period = std::__gcd(gcd_period, interests[i]->period.count());
        }
        
        return std::chrono::microseconds(gcd_period);
    }
    
    // Get specific interest record
    InterestRecord* get_interest(const Origin& origin, ComponentDataType type, bool is_internal) {
        std::string key = create_key(origin, type);
        const auto& table = get_table(is_internal);
        auto it = table.find(key);
        
        if (it != table.end()) {
            return it->second;
        }
        
        return nullptr;
    }
    
    // Remove old interests (cleanup)
    void cleanup_old_interests(std::chrono::seconds timeout = std::chrono::seconds(300)) {
        auto now = std::chrono::system_clock::now();
        
        // Cleanup internal table
        auto it = internal_table.begin();
        while (it != internal_table.end()) {
            if (now - it->second->last_updated > timeout) {
                it = internal_table.erase(it);
            } else {
                ++it;
            }
        }
        
        // Cleanup external table
        it = external_table.begin();
        while (it != external_table.end()) {
            if (now - it->second->last_updated > timeout) {
                it = external_table.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    // Get all registered interests (both internal and external)
    std::vector<InterestRecord*> get_all_interests() {
        std::vector<InterestRecord*> results;
        
        // Add internal interests
        for (const auto& [key, record] : internal_table) {
            results.push_back(record);
        }
        
        // Add external interests
        for (const auto& [key, record] : external_table) {
            results.push_back(record);
        }
        
        return results;
    }
    
    // Clear all registrations
    void clear() {
        internal_table.clear();
        external_table.clear();
    }
    
    // Get table sizes
    size_t size() const {
        return internal_table.size() + external_table.size();
    }
    
    size_t internal_size() const {
        return internal_table.size();
    }
    
    size_t external_size() const {
        return external_table.size();
    }
};

#endif // INTEREST_TABLE_H