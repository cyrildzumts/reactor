#include "actuators.h"

int ActuatorController::actuator_counter = 0;

Actuator::~Actuator(){

}


void Fan::up()
{

    speed_level = levels[current_level_index];
    current_level_index = (current_level_index + 1) % SPEED_LEVEL_STEPS;
    LOG("FAN", " FAN turned up", "current level :", speed_level, "thread id : ", std::this_thread::get_id());
}

void Fan::down()
{   if(current_level_index == 0){
        current_level_index = SPEED_LEVEL_STEPS;
    }
    current_level_index--;
    speed_level = levels[current_level_index];

    LOG("FAN", " FAN turned down", " current level :", speed_level, "thread id : ", std::this_thread::get_id());
}

Fan::Fan():  levels{I, II, III, IV, OFF }, current_level_index{0}
{
    activated = false;
    speed_level = SPEED_LEVEL::OFF;
}

void Fan::activate()
{
    if(activated){
        LOG("FAN", " FAN is already activated");

    }
    else{
        LOG("FAN", " FAN being activated");
        activated = true;
    }
    up();
}

void Fan::deactivate()
{
    if(activated){
        LOG("FAN", " FAN is beaing deactivated ");
        activated = false;
    }
    else{
        LOG("FAN", " FAN is already  deactivated ");
    }
    down();
}

void Valve::turn_valve_left()
{
    if(current_step_index == 0){
        current_step_index = VALVE_STEPS;
    }
    current_step_index--;
    current_valve_step = steps[current_step_index];

    LOG("VALVE", " Valve turned left", "current valve step : ", current_valve_step);
}

void Valve::turn_valve_right()
{

    current_valve_step = steps[current_step_index];
    current_step_index = (current_step_index + 1) % VALVE_STEPS;
    LOG("VALVE", " Valve turned right", "current valve step : ", current_valve_step);
}

Valve::Valve()
{
    activated = false;
    current_step_index = 0;
    current_valve_step = VALVE_SPIN_STEPS::LOCKED;
}

Valve::~Valve()
{
    LOG("VALVE", " Valve is being shut down ");
    deactivate();
}

void Valve::activate()
{
    if(activated){
        LOG("VALVE", " Valve is already activated", " thread id : ", std::this_thread::get_id());

    }
    else{
        LOG("VALVE", " Valve being activated ");
        activated = true;
    }
    turn_valve_right();
}

void Valve::deactivate()
{
    if(activated){
        LOG("VALVE", " Valve is being  deactivated", " thread id : ", std::this_thread::get_id());

    }
    else{
        LOG("VALVE", " Valve is already deactivated", " thread id : ", std::this_thread::get_id());
    }
    turn_valve_left();
    activated = false;
    LOG("VALVE", " Valve deactivated");
}


void Heater::heat_up()
{
    LOG("HEATER", " Heating UP ", "thread id : ", std::this_thread::get_id());
}

void Heater::heat_down()
{
    LOG("HEATER", " Heating DOWN ", "thread id : ", std::this_thread::get_id());
}

Heater::Heater()
{
    activated = false;
}

Heater::~Heater()
{
    deactivate();
}

void Heater::activate()
{
    LOG("HEATER", " Heating activated", " thread id : ", std::this_thread::get_id());
}

void Heater::deactivate()
{
    LOG("HEATER", " Heating deactivated", " thread id : ", std::this_thread::get_id());
}

bool ActuatorController::getQuit() const
{
    return quit;
}

void ActuatorController::setQuit(bool value)
{
    LOG("ACTUATORCONTROLLER ", " set quit to : " , value, " thread id : ", std::this_thread::get_id());
    quit = value;
}

void ActuatorController::operator()()
{
    LOG("ACTUATORCONTROLLER ", " controller started", " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
    LOG("ACTUATORCONTROLLER ", " controller initialized");
    LOG("ACTUATORCONTROLLER ", " controller entered  run");
    run();
}

int ActuatorController::getActuator_controller_id() const
{
    return actuator_controller_id;
}

void ActuatorController::setActuator_controller_id(int value)
{
    actuator_controller_id = value;
}

ActuatorController::ActuatorController()
{
    actuator_counter++;
    actuator_controller_id = actuator_counter;
    quit = false;
    LOG("ACTUATORCONTROLLER ", " Default controller created", " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
}

ActuatorController::ActuatorController(const ActuatorController &other)
{
    actuator_counter++;
    actuator_controller_id = actuator_counter;
    quit = false;
    LOG("ACTUATORCONTROLLER ", " copy controller created", " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
}

ActuatorController::~ActuatorController()
{
    LOG("ACTUATORCONTROLLER ", " controller destructed", " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
}

void ActuatorController::activateFan()
{
    LOG("ACTUATORCONTROLLER ", " activate FAN", " thread id : ", std::this_thread::get_id());
    fan.activate();
}

void ActuatorController::activateHeater()
{
    LOG("ACTUATORCONTROLLER ", " activate HEATER", " thread id : ", std::this_thread::get_id());
    heater.activate();
}

void ActuatorController::activatedValve()
{
    LOG("ACTUATORCONTROLLER ", " activate VALVE", " thread id : ", std::this_thread::get_id());
    valve.activate();
}

void ActuatorController::deactivateFan()
{
    LOG("ACTUATORCONTROLLER ", " deactivate FAN", " thread id : ", std::this_thread::get_id());
    fan.deactivate();
}

void ActuatorController::deactivateHeater()
{
    LOG("ACTUATORCONTROLLER ", " deactivate HEATER", " thread id : ", std::this_thread::get_id());
    heater.deactivate();
}

void ActuatorController::deactivatedValve()
{
    LOG("ACTUATORCONTROLLER ", " deactivate VALVE", " thread id : ", std::this_thread::get_id());
}

void ActuatorController::dispatch(double temp)
{
    LOG("ACTUATORCONTROLLER ", " dispatch ", "TEMPERATURE", temp, " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
    if(temp < 15){
        activateHeater();
        activatedValve();
        deactivateFan();
    }
    else if(temp < 22){
        activateHeater();
        deactivatedValve();
        deactivateFan();
    }
    else if(temp < 27 ){
        deactivateHeater();
        deactivatedValve();
    }
    else if(temp > 28 ){
        deactivateHeater();
        deactivatedValve();
        activateFan();
    }
}

void ActuatorController::run()
{
    LOG("ACTUATORCONTROLLER ", " starting run", " thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
    while(!quit){
    std::this_thread::sleep_for(SLEEP_MS(500));
    std::this_thread::yield();
    }
    LOG("ACTUATORCONTROLLER ", " terminating run", "thread id : ", std::this_thread::get_id(), " controller : ", actuator_controller_id);
}
