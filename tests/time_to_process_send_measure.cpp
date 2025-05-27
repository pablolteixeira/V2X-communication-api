#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

#include "../header/types.h" 
#include "../header/message.h"

const int NUM_MESSAGES = 10;
const int DELAY_BETWEEN_MESSAGES_MS = 10;

int main() {
    pid_t pid = getpid();
    std::string id = "NIC" + std::to_string(pid);
    int correct_packets = 0;

    EthernetNIC* nic = new EthernetNIC(id);
    EthernetProtocol* protocol = EthernetProtocol::get_instance();
    protocol->register_nic(nic);

    EthernetProtocol::Address addr(nic->address(), 1);
    EthernetCommunicator* comm = new EthernetCommunicator(EthernetProtocol::get_instance(), addr);
    std::vector<std::pair<std::chrono::high_resolution_clock::time_point,
                          std::chrono::high_resolution_clock::time_point>> timestamps;
                          
    Ethernet::Address address;
    EthernetProtocol::Address from(address, 1);
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);
    
    Message* msg = new Message();
    Message::ResponseMessage payload;
    msg->set_payload(payload);
    msg->set_type(Message::INTEREST);

    for(int i = 0; i < NUM_MESSAGES; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        if(comm->send(msg, from, to)) {
            auto end = std::chrono::high_resolution_clock::now();
            timestamps.emplace_back(start, end);
            correct_packets++;
        } 
        // std::this_thread::sleep_until(std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(DELAY_BETWEEN_MESSAGES_MS));
        // std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_BETWEEN_MESSAGES_MS));
    }

    // Save timestamp pairs to file
    std::ofstream tsfile("timestamps.txt");
    for (const auto &pair : timestamps) {
        auto start_ms = std::chrono::time_point_cast<std::chrono::microseconds>(pair.first).time_since_epoch().count();
        auto end_ms = std::chrono::time_point_cast<std::chrono::microseconds>(pair.second).time_since_epoch().count();
        tsfile << start_ms << " " << end_ms << "\n";
    }
    tsfile.close();

    // Compute average latency in microseconds
    double total_latency = 0;
    for (const auto &pair : timestamps) {
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(pair.second - pair.first).count();
        total_latency += latency;
    }

    double avg_latency = total_latency / timestamps.size();

    // Save average to file
    std::ofstream outfile("latency.txt");
    outfile << avg_latency << std::endl;
    outfile.close();

    // Print result
    std::cout << "Average latency: " << avg_latency << " us" << std::endl;
    std::cout << "Correct Packets: "  << correct_packets << std::endl;

    return 0;
}
