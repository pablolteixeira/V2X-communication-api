#include "../header/time_keeper.h"

TimeKeeper::TimeKeeper() {}

TimeKeeper::U64 TimeKeeper::get_system_timestamp() {
    return get_local_timestamp() - offset;
}

TimeKeeper::U64 TimeKeeper::get_local_timestamp() {
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
        offset = (t2 - t1) - delay;
    }
    is_t1 = !is_t1;
}