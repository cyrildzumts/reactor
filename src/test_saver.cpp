#include "test_saver.h"
#include <log.h>
#include <algorithm>

Testsaver::Testsaver()
{
    filename = "log/saved_values.m";
    std::ios_base::openmode mode = file.app|file.in|file.out;
    //file = std::fstream(filename, mode );
    file.open(filename, mode);
    if(!file.is_open()){
        LOG("Testsaver : ", " unable to open ", filename);
        exit(EXIT_FAILURE);
    }
}

Testsaver *Testsaver::instance()
{
    static Testsaver saver;
    return &saver;
}

Testsaver::~Testsaver()
{
    if(file.is_open()){
        file.clear();
        file.close();
    }
}

void Testsaver::save(const std::vector<int> &values, const std::string &description)
{
    file << description << "\n";
    file << "values = [ ";
    std::for_each(values.begin(), values.end(), [&](int value){
        file << value << " ";
    });
    file << "];\n";
}
