#include <string>
#include <iostream>
#include <caf/all.hpp>

using cmd_service = caf::atom_constant<caf::atom("commande")>;

class CommandActor : public caf::event_based_actor{

    public:
        CommandActor(caf::actor_config &cfg) : caf::event_based_actor(cfg){

        }

        caf::behavior make_behavior() override{
            caf::aout(this) << "inside CommandActor " << std::endl;
            return {
                [=](cmd_service,const std::string &what) -> std::string {
                caf::aout(this) << "Greetings Service selected" << std::endl;
                caf::aout(this) << what << std::endl;
                caf::aout(this) << "X : " << x << std::endl;
                return std::string(what.rbegin(), what.rend());
                },
                [=](const std::string &what)->std::string {
                    caf::aout(this) << "Unknown Service selected" << std::endl;
                    return std::string("Unknown service");
                }
            };
        }
    private:
        int x = 42;
};