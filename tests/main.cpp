 //#define BOOST_TEST_DYN_LINK
 #define BOOST_TEST_MODULE "BaseClassModule"

#include <boost/test/unit_test.hpp>

class TestObject{
private:
    bool tested;
    int count;

public:
    TestObject():tested{false}, count{0}{

    }

    bool isTested() const{
        return tested;
    }
    void test(){
        tested = !tested;
        count++;
    }
    int getCount() const{
        return count;
    }
};

BOOST_AUTO_TEST_CASE(countTest){
    TestObject t1;
    BOOST_TEST(!t1.isTested());
    t1.test();
    BOOST_TEST(t1.isTested());
    BOOST_CHECK(t1.getCount() == 1);
}

