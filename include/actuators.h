#include "sensor.h"
#include "safe_queue.h"
#include <log.h>
#include <array>
#include <thread>
#include <chrono>

using SLEEP_MS = std::chrono::milliseconds;

#define SPEED_LEVEL_STEPS 5
#define VALVE_STEPS 4

class Actuator{

protected:
    bool activated;

public:
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual ~Actuator();
};


class Fan : public Actuator{
public:
    enum SPEED_LEVEL{
        I = 0, II, III, IV, OFF
    };
private:
    SPEED_LEVEL speed_level;
    std::array<SPEED_LEVEL, SPEED_LEVEL_STEPS> levels;
    size_t current_level_index;
    void setSpeed(SPEED_LEVEL speed_level);
    void up();
    void down();


public:
    Fan();
    // Actuator interface
public:
    virtual void activate() override;
    virtual void deactivate() override;
};


class Valve : Actuator{
private:
    enum VALVE_SPIN_STEPS{
        LOW, MIDDLE, HIGH, LOCKED
    };

    VALVE_SPIN_STEPS current_valve_step;
    std::array<VALVE_SPIN_STEPS, VALVE_STEPS> steps;
    size_t current_step_index;
    void turn_valve_left();
    void turn_valve_right();


public:
    Valve();
    ~Valve() override;
    // Actuator interface
public:
    virtual void activate() override;
    virtual void deactivate() override;
};


class Heater : public Actuator{
private:
    void heat_up();
    void heat_down();


public:
    Heater();
    ~Heater() override;
    // Actuator interface
public:
    virtual void activate() override;
    virtual void deactivate() override;
};


class ActuatorController{
private:
    Fan fan;
    Heater heater;
    Valve valve;
    volatile bool quit;
    int  actuator_controller_id;
    static int actuator_counter;
public:
    ActuatorController();
    ActuatorController(const ActuatorController &other);
    ~ActuatorController();
    void activateFan();
    void activateHeater();
    void activatedValve();

    void deactivateFan();
    void deactivateHeater();
    void deactivatedValve();
    void dispatch(double temp);
    void run();
    bool getQuit() const;
    void setQuit(bool value);
    void operator()();
    int getActuator_controller_id() const;
    void setActuator_controller_id(int value);
};
