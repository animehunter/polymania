#pragma once

/*
 * Abstract high precision RTC
 */
class Timer {
public:
    virtual double Seconds() = 0;
};
