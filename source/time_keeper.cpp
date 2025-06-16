#include "../header/time_keeper.h"

TimeKeeper::TimeKeeper() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int> dist(0, 500);
    fake_offset = dist(gen);
    sync_timeout *= 1000;
}

U64 TimeKeeper::get_system_timestamp() {
    return get_local_timestamp() + fake_offset + offset;
}

TimeKeeper::SyncState TimeKeeper::get_sync_state() {
    return sync_state;
}

void TimeKeeper::update_sync_state(SyncState new_state) {
    sync_state = new_state;
}

U64 TimeKeeper::get_local_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto timestamp = std::chrono::duration_cast<std::chrono::microseconds>(duration);

    return timestamp.count();
}

void TimeKeeper::update_time_keeper(U64 system_timestamp, U64 local_timestamp) {
    if (is_t1) {
        t1 = system_timestamp;
        t2 = local_timestamp;
    } else {
        t3 = system_timestamp;
        t4 = local_timestamp;
        
        U64 delay = ((t4 - t3) + (t2  - t1)) / 2;
        ConsoleLogger::log("Delay: " + std::to_string(delay));

        offset = (t2 - t1) - delay;
        last_update = local_timestamp;
        sync_state = SyncState::SYNCHRONIZED;
    }
    is_t1 = !is_t1;
    ConsoleLogger::log("Fake Offset: " + std::to_string(fake_offset) + " | Offset: " + std::to_string(offset));
}

void TimeKeeper::update_sync_status() {
    auto elapsed_time = get_local_timestamp() - last_update;
    if (elapsed_time > sync_timeout) {
        sync_state = SyncState::NOT_SYNCHRONIZED;
    }
}