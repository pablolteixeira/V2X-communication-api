#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include <chrono>
#include <random>

#include "u64_type.h"
#include "ethernet.h"
#include "console_logger.h"

#define SYNC_TIMEOUT 50;


class TimeKeeper 
{
public:
    typedef Ethernet::Footer::SyncState SyncState;
    typedef Ethernet::Footer::PacketOrigin PacketOrigin;

    TimeKeeper();
    ~TimeKeeper();

    U64 get_system_timestamp();
    U64 get_local_timestamp();
    SyncState get_sync_state();
    PacketOrigin get_packet_origin();

    void update_packet_origin(PacketOrigin packet_origin);
    void update_sync_state(SyncState sync_sate);
    void update_time_keeper(U64 system_timestamp, U64 local_timestamp);
    void update_sync_status();

private:
    bool is_t1 = true;
    bool synchronize_time = true;
    int fake_offset;
    
    U64 offset = 0;
    U64 last_update;
    U64 t1 = 0;
    U64 t2 = 0;
    U64 t3 = 0;
    U64 t4 = 0;
    
    PacketOrigin _packet_origin = PacketOrigin::OTHERS;
    SyncState sync_state = SyncState::NOT_SYNCHRONIZED;
    U64 sync_timeout = SYNC_TIMEOUT;
};

#endif // TIME_KEEPER_H 