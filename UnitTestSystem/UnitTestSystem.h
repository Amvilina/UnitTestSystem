#pragma once
#include "TestClassBase.h"
#include <cmath>

namespace UnitTestSystem
{
    class Assert {};
} // namespace UnitTestSystem

#define ASSERT(expr) if(!(expr)) throw UnitTestSystem::Assert()


#define TEST_MODULE(name)                                                                                          \
class name : public UnitTestSystem::Base<name> {                                                                   \
  public:                                                                                                          \
    static std::string GetName() { return #name; }                                                                 \
};                                                                                                                 \
namespace UnitTestSystem::internal_namespace_##name  {                                                             \
using CurrentModule = name;                                                                                        \
}                                                                                                                  \
namespace UnitTestSystem::internal_namespace_##name                                                                \

#define TEST_FUNCTION_BASE(name, timeMeasuring)                                                                    \
void name();                                                                                                       \
static UnitTestSystem::FunctionRegister<CurrentModule> register_##name(#name, name, timeMeasuring);                \
void name()                                                                                                        \

#define TEST_FUNCTION(name) TEST_FUNCTION_BASE(name, false)
#define TEST_FUNCTION_TIME_MEASURING(name) TEST_FUNCTION_BASE(name, true)


#define MUST_BE_TRUE(exp) MustBeTrue(exp, __LINE__, #exp)
#define MUST_BE_FALSE(exp) MustBeFalse(exp, __LINE__, #exp)

#define MUST_BE_EQUAL(a, b) MustBeEqual(a, b, __LINE__, #a, #b)
#define MUST_BE_CLOSE_DOUBLES(a, b) MustBeCloseDoubles(a, b, __LINE__, #a, #b)

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

#define MUST_ASSERT(...)                                                                                           \
try {                                                                                                              \
    __VA_ARGS__;                                                                                                   \
    throw Error(__LINE__, #__VA_ARGS__, "There were no asserts");                                                  \
}                                                                                                                  \
catch(const Error& e) {throw;}                                                                                     \
catch(const UnitTestSystem::Assert& e) { }                                                                         \
catch(...){throw;}'.'                                                                                              \
//This character '.' is to force user to write MUST_ASSERT(...); <- with semicolon at the end
