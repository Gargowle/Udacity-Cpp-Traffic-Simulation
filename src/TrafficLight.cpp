#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

 // constants  
constexpr double cycle_duration_min = 4.0;
constexpr double cycle_duration_max = 6.0;

// utility functions
std::chrono::duration<double> GenerateNewCycleDuration()
{
  return std::chrono::duration<double>(
    cycle_duration_min + static_cast<double>(std::rand()) / (static_cast<double> (RAND_MAX/(cycle_duration_max - cycle_duration_min)))
  );
}

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> uLock(_mutex);
  
    // wait until notified that queue is not empty anymore
    // furthermore, a lambda is supplied to make sure that the wake-up was not spurious
    _condition.wait(uLock, [this] {return !_queue.empty();});

    T msg = _queue.front();
    _queue.pop_front();
  
    return msg;    
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}



/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
      if(_messageQueue.receive() == TrafficLightPhase::green)
        return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

  
    // initialize variables
    std::chrono::duration<double> cycle_duration = GenerateNewCycleDuration();
    std::chrono::high_resolution_clock::time_point time_last_toggled = std::chrono::high_resolution_clock::now();
  
    while(true)
    {
      if(std::chrono::high_resolution_clock::now() - time_last_toggled > cycle_duration)
      {
        // toggle _currentPhase
        _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
        _messageQueue.send(std::move(_currentPhase));
        
        // update cycle duration
        cycle_duration = GenerateNewCycleDuration();
        // reset last_time_toggled
        time_last_toggled = std::chrono::high_resolution_clock::now();
      }
      else
      {
        // sleep a milli second to reduce cpu load
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
}
