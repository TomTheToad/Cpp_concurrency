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

    // Fields
    // Init stop watch
    std::chrono::time_point<std::chrono::system_clock> _lastUpdate;

    // Declare and initialize starting time between light cycles (TrafficLightPhaseToggle) in seconds
    int _lightPhaseDuration = 4;

    // Keep track of number of loop cycles for sleep and work
    int _loopCycle = 0;

    while(true) {

        // Check cycle
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - _lastUpdate).count();

        // Every two cycles do work
        _loopCycle++;
        if (_loopCycle >= 2) {

            // 1) Toggle traffic light phase when time threshhold reached (convert from seconds to milliseconds)
            if (timeSinceLastUpdate >= (_lightPhaseDuration * 1000 )) {
                // Test log
                std::cout << "timeSinceLastUpdate: " << (timeSinceLastUpdate) << std::endl;
                // Toggle light red vs green
                TrafficLightPhaseToggle();
                // TODO: update method to msg queue
                // Set time of update
                _lastUpdate = std::chrono::system_clock::now();
                }

            // 2) Sleep to keep processor load reasonable
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            // 3) Reset loop cycle
            _loopCycle = 0;

            // 4) Change _lightPhaseDuration
            _lightPhaseDuration = rand() % 6 + 4;
        }


    }
}