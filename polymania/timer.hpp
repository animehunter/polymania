#ifndef timer_h__
#define timer_h__

/*
 * Abstract High Resolution Timer
 */

class Timer {
public:
    virtual double Seconds() = 0;
};

#endif // timer_h__

