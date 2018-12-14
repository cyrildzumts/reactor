#ifndef TEST_SAVER_H
#define TEST_SAVER_H
#include <iostream>
#include <fstream>
#include <string>
#include<vector>

class Testsaver{
private:
    std::fstream file;
    std::string filename;
    Testsaver();

public:
    static Testsaver *instance();
    ~Testsaver();
    void save(const std::vector<int> &values, const std::string &description);

};

#endif //TEST_SAVER_H
