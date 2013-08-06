#include <unistd.h>
#include <sys/time.h>

#include "../timer.hpp"
#include "timer_rpi.hpp"

double RaspberryPiTimer::Seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double) tv.tv_sec + (double) tv.tv_usec * 0.000001;
}
