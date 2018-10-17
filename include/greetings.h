#ifndef GREETINGS_H
#define GREETINGS_H
#include <string>
#include <iostream>
#include <caf/all.hpp>

int constexpr LIMIT_INALID_ERROR = -5;
int constexpr LIMIT_OK   = 0;

using atom_greetings = caf::atom_constant<caf::atom("greeting")>;
using atom_greetings_limit = caf::atom_constant<caf::atom("greetlimit")>;
using cell = caf::typed_actor<caf::reacts_to<atom_greetings, std::string>,
                                   caf::replies_to<atom_greetings>::with<std::string>>;
struct cell_state{
    int value = 0;
};
/*
cell::behavior_type type_checked_cell(cell::stateful_pointer<cell_state> self){
    return {
        [=](atom_greetings, const std::string &what){

        }
    };
}
*/
class GreetingActor : public caf::event_based_actor {
public:
    GreetingActor(caf::actor_config &cfg):caf::event_based_actor{cfg}{
        usage_counter = 0;
        max_limit = 10;
        current_limit = max_limit;
    }

    caf::behavior make_behavior() override{

        return {
            [=](atom_greetings, const std::string &what){

                if(usage_counter <= current_limit){
                    usage_counter++;
                    caf::aout(this) << "Greeting Actor Name " << this->name() << std::endl;
                    caf::aout(this) << "Greetings from GreetingActor " << std::endl;
                    caf::aout(this) << "Received message : " << what << std::endl;
                    //caf::aout(this) << "usage frequency : " << (++usage_counter) << std::endl;
                    return std::string("your current usage is " + std::to_string(usage_counter));
                }
                else{
                    caf::aout(this) << "GreetingActor out of service " << std::endl;
                    caf::aout(this) << "Received message : " << what << std::endl;
                    //caf::aout(this) << "usage frequency : " << (++usage_counter) << std::endl;
                    return std::string("your current usage is " + std::to_string(++usage_counter));
                }
            },
            [=](atom_greetings_limit,const int usage_limit){
                int flag = LIMIT_OK;
                if((usage_limit > 0) && (usage_limit <= max_limit)){
                    current_limit = usage_limit;
                }
                else{
                    flag = LIMIT_INALID_ERROR;
                }
                return flag;
            }
        };
    }


private:
    int current_limit;
    int usage_counter;
    int max_limit;
};

#endif // GREETINGS_H
