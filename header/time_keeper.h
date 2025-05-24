#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include <chrono>
#include "types.h"

class TimeKeeper 
{
public:
    TimeKeeper();
    ~TimeKeeper();

    U64 get_system_timestamp();
    U64 get_local_timestamp();
    void update_time_keeper(U64 system_timestamp, U64 local_timestamp);
private:

private:
    U64 offset = 0;
    bool is_t1 = true;
    U64 t1 = 0;
    U64 t2 = 0;
    U64 t3 = 0;
    U64 t4 = 0;
}

#endif // TIME_KEEPER_H 