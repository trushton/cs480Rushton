#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <sys/time.h>
#include <stdexcept>
#include <iostream>

using namespace std;


class Timer {
  public:
  Timer();
  void start() throw (runtime_error);
  void stop() throw (logic_error);
  void pause() throw (logic_error);
  void resume() throw (logic_error);
  long getElapsedTime() throw (logic_error);
  bool isStarted() {return timerWasStarted;};

  private:
  // You should change the data types for your clocks based 
  // upon what timer you use ... and include the right .h file
  timeval beginTime;
  timeval duration;
  timeval pausedTime;
  bool timerWasStarted;
};

#endif	// ifndef TIMER_H