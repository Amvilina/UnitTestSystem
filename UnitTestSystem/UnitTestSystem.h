#pragma once
#include "TestClassBase.h"
#include <cmath>

namespace UnitTestSystem
{
    class Assert {};
} // namespace UnitTestSystem

#define ASSERT(expr) if(!(expr)) throw UnitTestSystem::Assert()

#define TEST_MODULE_BEGIN(name)                                                         \
class name : public UnitTestSystem::TestClassBase {                                     \
  public:                                                                               \
    static void Run() { name{}.RunTests(); }                                            \
  private:                                                                              \
     name() {}                                                                          \
                                                                                        \
     void RunTests() {                                                                  \
        using namespace UnitTestSystem;                                                 \
        _name = #name;                                                                  \
        std::cout << _name << ": ";                                                     \
        _methodResults.clear();                                                         \
        _bytesLeaked = 0;                                                               \
        TEST_METHOD_START(Validation, false){}                                          \

#define TEST_METHOD_START(name, measureTime)                                            \
        {                                                                               \
            MethodResult methodResult;                                                  \
            methodResult.methodName = #name;                                            \
            methodResult.isTimeMeasuring = measureTime;                                 \
            MemoryAllocator::ResetUsedBytes();                                          \
            {                                                                           \
                Timer timer;                                                            \
                try                                                                     \
             // {
             // }
#define TEST_METHOD_END                                                                             \
                catch(const Error& error) {                                                         \
                    methodResult.error = error;                                                     \
                }                                                                                   \
                catch(...) {                                                                        \
                    Error error(0, "", "Unknown exception occured!");                               \
                    methodResult.error = error;                                                     \
                }                                                                                   \
                methodResult.timeElapsedNanoseconds = timer.GetNanoseconds();                       \
            }                                                                                       \
            if (methodResult.IsSuccess()){                                                          \
                const auto bytesLeaked = MemoryAllocator::GetUsedBytes();                           \
                _bytesLeaked += bytesLeaked;                                                        \
                if (bytesLeaked > 0){                                                               \
                    Error error(0, "", "Memory leak: " + std::to_string(bytesLeaked) + " byte(s)"); \
                    methodResult.error = error;                                                     \
                }                                                                                   \
            }                                                                                       \
            _methodResults.push_back(methodResult);                                                 \
            if (_methodResults.size() > 1) {                                                        \
                if (methodResult.IsSuccess())                                                       \
                    std::cout << 'T';                                                               \
                else                                                                                \
                    std::cout << 'F';                                                               \
            }                                                                                       \
        }                                                                                           \

#define TEST_MODULE_END                                                                 \
        TEST_METHOD_END                                                                 \
        _methodResults.erase(_methodResults.begin());                                   \
        PrintResults();                                                                 \
    }                                                                                   \
};                                                                                      \

#define TEST_METHOD(name)                 \
TEST_METHOD_END                           \
TEST_METHOD_START(#name, false)           \

#define TEST_TIME_MEASURING_METHOD(name)  \
TEST_METHOD_END                           \
TEST_METHOD_START(#name, true)            \

#define MUST_BE_TRUE(exp) if(!(exp)) throw Error(__LINE__, #exp, "Expected True but was False")
#define MUST_BE_FALSE(exp) if(exp) throw Error(__LINE__, #exp, "Expected False but was True")

#define MUST_BE_EQUAL(a, b)                                                                                   \
if ((a) != (b)) throw Error(__LINE__, std::string(#a) + std::string(" == ") + std::string(#b),                \
                                            std::to_string(a) + std::string(" != ") + std::to_string(b))      \

#define MUST_BE_CLOSE_DOUBLES(a, b)                                                  \
if (fabs((a) - (b)) > (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 1e-5)                \
    throw Error( __LINE__,                                                           \
                  std::string(#a) + std::string(" ~= ") + std::string(#b),           \
                  std::to_string(a) + std::string (" != ") + std::to_string(b) )     \

#define MUST_THROW_EXCEPTION(...)                                                                                  \
try {                                                                                                              \
    __VA_ARGS__;                                                                                                   \
    throw Error(__LINE__, #__VA_ARGS__, "There were no exceptions");                                               \
}                                                                                                                  \
catch(const Error& e) {throw;}                                                                                     \
catch(...) { }'.'                                                                                                  \
//This character '.' is to force user to write MUST_THROW_EXCEPTION(...); <- with semicolon at the end

#define MUST_THROW_SPECIFIC_EXCEPTION(exceptionClass, ...)                                                         \
try {                                                                                                              \
    __VA_ARGS__;                                                                                                   \
    throw Error(__LINE__, #__VA_ARGS__, "There were no exceptions");                                               \
}                                                                                                                  \
catch(const Error& e) {throw;}                                                                                     \
catch(const exceptionClass& e) { }                                                                                 \
catch(...){ throw Error( __LINE__,                                                                                 \
                         #__VA_ARGS__,                                                                             \
                         std::string("There were no exceptions of type ") + std::string(#exceptionClass)); }'.'    \
//This character '.' is to force user to write MUST_THROW_SPECIFIC_EXCEPTION(exeption_class, ...); <- with semicolon at the end

#define MUST_ASSERT(...)                                                         \
try {                                                                            \
    __VA_ARGS__;                                                                 \
    throw Error(__LINE__, #__VA_ARGS__, "There were no asserts");                \
}                                                                                \
catch(const Error& e) {throw;}                                                   \
catch(const UnitTestSystem::Assert& e) { }                                       \
catch(...){throw;}'.'                                                            \
//This character '.' is to force user to write MUST_ASSERT(...); <- with semicolon at the end
