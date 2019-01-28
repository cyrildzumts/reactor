#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>


/***********************************************************************
 * Autor : Cyrille Ngassam Nkwenga
 * 2019
 * Description : This class generates random integer numbers
 * in the interval [1, LIMIT].
 * By default LIMIT is set to 100
************************************************************************/


template<int LIMIT=100>
class Generator{

private:
    std::mt19937 *gen;
    std::uniform_int_distribution<int> distribution;

    Generator(){
        gen = new std::mt19937(0);
        //gen = new std::mt19937(std::random_device()());
        distribution = std::uniform_int_distribution(1, LIMIT);
    }
public:
    ~Generator(){

    }
    static Generator *instance(){
        static Generator _instance;
        return &_instance;
    }

    /**
     * @brief generate generate a a random integer value.
     * @return
     */
    int generate(){
        return distribution(*gen);
    }
};


#endif //GENERATOR_H
