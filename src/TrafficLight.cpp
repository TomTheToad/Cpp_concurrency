#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive() {
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    
    // queue modification
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_messages.empty(); });

    // remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg;

}

template <typename T>
void MessageQueue<T>::send(T &&msg) {
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    // sleep to save the cpu
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // vector modification
    std::lock_guard<std::mutex> uLock(_mutex);

    // add vector to queue
    _messages.push_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight() {
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen() {
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (queue.receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase() {
    return _currentPhase;
}

void TrafficLight::TrafficLightPhaseToggle() {
    TrafficLight::_currentPhase = (TrafficLight::getCurrentPhase() == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
    // Test log
    std::cout << "Traffic Light is " << TrafficLight::getCurrentPhase() << " now.";
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

    /*
        Note: I made some changes to the loop cycle sleep pattern. It did not make much sense to take up processor time
        adding many extra variables to track and maintain the light cycle duration.
        Instead I calculated the random light duration in milliseconds and added it to the sleep cycle.
    */

    // Fields
    long _lightPhaseDuration;

    // Keep track of number of loop cycles for sleep and work
    int _loopCycle = 0;

    while(true) {

        // Change _lightPhaseDuration
        _lightPhaseDuration = rand() % 3000 + 4000;
        std::cout << "Light duration = " << _lightPhaseDuration << std::endl;

        // Every two cycles do work
        _loopCycle++;
        if (_loopCycle >= 2) {
                
            // Toggle light color
            TrafficLightPhaseToggle();

            // Send current phase to msg queue
            queue.send(std::move(getCurrentPhase()));

            // Reset loop cycle
            _loopCycle = 0;

            // Sleep to keep processor load reasonable
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + _lightPhaseDuration));

        }

    }
}