#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>

template<int LIMIT=100>
class Generator{
private:
    //std::random_device device;
    std::mt19937 *gen;
    std::uniform_int_distribution<int> distribution;


    Generator(){
        //gen = new std::mt19937(std::random_device()());
        gen = new std::mt19937(0);
        distribution = std::uniform_int_distribution(1, LIMIT);
    }
public:
    ~Generator(){

    }
    static Generator *instance(){
        static Generator _instance;
        return &_instance;
    }

    int generate(){
        return distribution(*gen);
    }
};


#endif //GENERATOR_H
