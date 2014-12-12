#include "timer.h"
using namespace std;

//------------------------------------------------

/**
 * Default constructor. 
 * 
 * Default constructor of timer class.
 * Sets data members to zero and false.
*/
Timer::Timer()
{
   beginTime.tv_sec = 0;
   duration.tv_sec = -1;
   pausedTime.tv_sec = 0;
   timerWasStarted = false;
}


/**
 * Start timer. 
 * 
 * Begins keeping track of the time passing, until stop.
 * Throws an exception if timer could not start.
 * 
 * @exception throws error if timer did not start
*/
void Timer::start() throw( runtime_error )
{
   /// gets the time of the day
   if( gettimeofday( &beginTime, NULL) == -1 )
   {
      /// throw runtime error if timer did not start
      return;
   }

   /// set that the timer has been started
   timerWasStarted = true;
}

/**
 * Stops timer. 
 * 
 * Stops timer incrementation and calculates
 * durantion that timer counter was on. 
 * 
 * @exception throws logic error if timer never started
*/
void Timer::stop() throw( logic_error )
{
   /// if the timer hasn't been started, throw exception
   if( !timerWasStarted )
   {
      return;
   }
   
   /// instantiate time value holder variables
   long time1;
   long time2;
   
   /// set the current time to the duration 
   gettimeofday(&duration, NULL);
   
   /// gets the values of the microseconds
   time1 = beginTime.tv_sec * 1000000 + beginTime.tv_usec;
   time2 = duration.tv_sec * 1000000 + duration.tv_usec;
   
   /// sets duration to difference of start and end times
   duration.tv_sec = time2 - time1;
   
   /// sets that timer is not running anymore
   timerWasStarted = false;
}




/**
  * Returns elapsed time
  * 
  * Calculates elapsed time by converting
  * microseconds to seconds. Returns this value.
  * Throws a logic error if the duration was never set because the
  * timer had not been turned on.
  * 
  * @exception Throws logic error if duration not set.
*/ 
long Timer::getElapsedTime() throw( logic_error )
{
   /// throws a logic error if duration was never set 
   if( !timerWasStarted )
   {
      return 0;
   }

   /// instantiate time value holder variables
   long time1;
   long time2;
   
   /// set the current time to the duration 
   gettimeofday(&duration, NULL);
   
   /// gets the values of the microseconds
   time1 = beginTime.tv_sec * 1000000 + beginTime.tv_usec;
   time2 = duration.tv_sec * 1000000 + duration.tv_usec;
   
   /// sets duration to difference of start and end times
   duration.tv_sec = time2 - time1;
   
   /// return the value of duration in microseconds
   return pausedTime.tv_sec + duration.tv_sec;
}


void Timer::pause() throw (logic_error)
{
   /// if the timer hasn't been started, throw exception
   if( !timerWasStarted )
   {
    return;
       }
   
   /// instantiate time value holder variables
   long time1;
   long time2;
   
   /// set the current time to the duration 
   gettimeofday(&duration, NULL);
   
   /// gets the values of the microseconds
   time1 = beginTime.tv_sec * 1000000 + beginTime.tv_usec;
   time2 = duration.tv_sec * 1000000 + duration.tv_usec;
   
   /// sets duration to difference of start and end times
   pausedTime.tv_sec = time2 - time1;
   
   /// sets that timer is not running anymore
   timerWasStarted = false;
}


void Timer::resume()  throw (logic_error)
{

   /// gets the time of the day
   if( gettimeofday( &beginTime, NULL) == -1 )
   {
    return;
   }

   /// set that the timer has been started
   timerWasStarted = true;

}