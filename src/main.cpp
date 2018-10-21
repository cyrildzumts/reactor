
#include <string>
#include <iostream>
#include <caf/all.hpp>
#include "command.h"
#include "greetings.h"
#include "circuitbreaker.h"
#include <thread>
using namespace std;
//using std::endl;
//using std::string;
//using namespace caf;

//using greeting_service = atom_constant<atom("greetings")>;
//using calculus_service = atom_constant<atom("calculus")>;

//caf::behavior mirror(event_based_actor *self){
//    // This is an event based function :
    
//    return {
//        [=](greeting_service ,const string &what) -> std::string {
//            caf::aout(self) << "Greetings Service selected" << endl;
//            caf::aout(self) << what << endl;
//            //caf::aout(self) << "X : " << x << endl;
//            return string(what.rbegin(), what.rend());
//        },
//        [=](const string &what)->string {
//            caf::aout(self) << "Unknown Service selected" << endl;
//            return string("Unknown service");
//        }
//    };
//}

//void hello_actor(event_based_actor *self, const actor &buddy){
//    for(int i = 0; i < 20; i++){
//        self->request(buddy, std::chrono::microseconds(50), atom_greetings::value, "Hello Buddy!").
//        then([=](const std::string &what){
//            aout(self) << what << endl;
//        },
//        [=](const error &err){
//            aout(self) << "Error on last request" <<  " ==> "
//                    << err.code()
//                    << endl;
//        });
//    }
//}





int main(int argc, char const *argv[])
{
    std::cout << "Hello Reactor" << std::endl;
    // create an environment 
    //actor_system_config cfg;
    //actor_system sys{cfg};
    Service *service = new ConcreteService();
    CircuitBreaker cb{service};
    for(int i = 0; i <30; i++){
        cb.process_request(i-5);
        std::this_thread::sleep_for(time_ms_t(500));
    }
    ///sys.logger();
    // create a new actor which propose the mirror service
    //auto mirror_actor = sys.spawn(mirror);
    //auto cmd_actor = sys.spawn<CommandActor>();
    //auto greeting_actor = sys.spawn<GreetingActor>();
    // create an actor who consume the mirror service:
    //auto mirror_cusumer_actor = sys.spawn(hello_actor, mirror_actor);
    //auto mirror_cmd_actor = sys.spawn(hello_actor, cmd_actor);
    //auto greeting_buddy_actor = sys.spawn(hello_actor, greeting_actor);
    //std::cout << "Current running Actor : "
     //   << sys.registry().running() << std::endl;
    
    return 0;
}
