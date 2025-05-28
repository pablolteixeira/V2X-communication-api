#include "../header/time_keeper.h"

TimeKeeper::TimeKeeper() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(-100, 100);
    fake_offset = dist(gen);
    sync_timeout *= 1000;
}

U64 TimeKeeper::get_system_timestamp() {
    return get_local_timestamp() - offset;
}

TimeKeeper::SyncState TimeKeeper::get_sync_state() {
    return _sync_state;
}

void TimeKeeper::update_sync_state(SyncState new_state) {
    _sync_state = new_state;
}

TimeKeeper::PacketOrigin TimeKeeper::get_packet_origin() {
    return  _packet_origin;
}

void TimeKeeper::update_packet_origin(PacketOrigin packet_origin) {
    _packet_origin = packet_origin;
}

U64 TimeKeeper::get_local_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration) + 
                     std::chrono::microseconds(fake_offset) +
                     std::chrono::microseconds(offset);

    return timestamp.count();
}

void TimeKeeper::update_time_keeper(U64 system_timestamp, U64 local_timestamp) {
    auto elapsed_time = local_timestamp - last_update;
    if (elapsed_time > sync_timeout * 10/100) {    
        if (is_t1) {
            t1 = system_timestamp;
            t2 = local_timestamp;
        } else {
            t3 = system_timestamp;
            t4 = local_timestamp;
            
            U64 delay = ((t4 - t3) + (t2  - t1)) / 2;
            offset = (t2 - t1) - delay;
            last_update = local_timestamp + offset;
        }
        is_t1 = !is_t1;
    }
}