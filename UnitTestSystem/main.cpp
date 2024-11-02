#include <thread>

#include "UnitTestSystem.h"

TEST_MODULE_BEGIN(FirstModule)

TEST_METHOD(CorrectCode) {
    MUST_BE_TRUE(true);
    MUST_BE_TRUE(1 + 1 > 0);
    
    MUST_BE_FALSE(false);
    MUST_BE_FALSE(3 == 4);
    
    MUST_BE_EQUAL(3+12, 15);
    
    MUST_BE_CLOSE_DOUBLES(1.234567, 1 + 0.234566);
    
    MUST_THROW_EXCEPTION(throw 123);
    
    std::vector<int> vec;
    MUST_THROW_SPECIFIC_EXCEPTION(std::out_of_range, vec.at(3));
    
    MUST_ASSERT(ASSERT(1 == 2));
}

TEST_METHOD(MUST_BE_TRUE_error) {
    MUST_BE_TRUE(1 < 0);
}

TEST_METHOD(MUST_BE_FALSE_error) {
    MUST_BE_FALSE(1 != 0);
}

TEST_METHOD(MUST_BE_EQUAL_error) {
    MUST_BE_EQUAL(12 + 5, 1 + sizeof(char));
}

TEST_METHOD(MUST_BE_CLOSE_DOUBLES_error) {
    MUST_BE_CLOSE_DOUBLES(1.1, 1.0 + 0.01);
}

TEST_METHOD(MUST_THROW_EXCEPTION_error) {
    MUST_THROW_EXCEPTION(1+1);
}

TEST_METHOD(MUST_THROW_SPECIFIC_EXCEPTION_error1) {
    MUST_THROW_SPECIFIC_EXCEPTION(std::out_of_range, 1+1);
}

TEST_METHOD(MUST_THROW_SPECIFIC_EXCEPTION_error2) {
    std::vector<int> vec;
    MUST_THROW_SPECIFIC_EXCEPTION(std::bad_cast, vec.at(0));
}

TEST_METHOD(MUST_ASSERT_error1) {
    MUST_ASSERT(1+1);
}

TEST_METHOD(MUST_ASSERT_error2) {
    MUST_ASSERT(ASSERT(1 == 1));
}

TEST_METHOD(RandomException) {
    throw 123;
}

TEST_METHOD(MemoryLeak) {
    auto a = new char[10];
}

TEST_METHOD(NoMemoryLeak) {
    auto a = new char;
    delete a;
    
    auto arr = new int[13];
    delete [] arr;
}

TEST_TIME_MEASURING_METHOD(Time) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.1s);
}

TEST_TIME_MEASURING_METHOD(TimeNoIfError) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.1s);
    MUST_BE_TRUE(false);
}

TEST_TIME_MEASURING_METHOD(TimeNoIfMemoryLeak) {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(0.1s);
    auto a = new char;
}

TEST_MODULE_END


TEST_MODULE_BEGIN(SecondEmptyModule)
TEST_MODULE_END


int main(int argc, const char * argv[]) {
    FirstModule::Run();
    SecondEmptyModule::Run();
    return 0;
}
