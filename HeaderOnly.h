#pragma once
#include <chrono>
#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

namespace UnitTestSystem
{
using namespace std::chrono;

class Timer {
  private:
    time_point<high_resolution_clock> _lastTimePoint;
  public:
    Timer() : _lastTimePoint(high_resolution_clock::now()) {}
    
    void Restart() {
        _lastTimePoint = high_resolution_clock::now();
    }
    
    uint64_t GetNanoseconds() const {
        const auto now = high_resolution_clock::now();
        return duration_cast<nanoseconds>(now - _lastTimePoint).count();
    }
    
    uint64_t GetMicroseconds() const {
        return GetNanoseconds() / 1e3;
    }
    
    uint64_t GetMilliseconds() const {
        return GetNanoseconds() / 1e6;
    }
    
    uint64_t GetSeconds() const {
        return GetNanoseconds() / 1e9;
    }
    
    uint64_t GetNanosecondsAndRestart() {
        const auto time = GetNanoseconds();
        Restart();
        return time;
    }
    
    uint64_t GetMicrosecondsAndRestart() {
        return GetNanosecondsAndRestart() / 1e3;
    }
    
    uint64_t GetMillisecondsAndRestart() {
        return GetNanosecondsAndRestart() / 1e6;
    }
    
    uint64_t GetSecondsAndRestart() {
        return GetNanosecondsAndRestart() / 1e9;
    }
};

class MemoryAllocator {
  private:
    static uint64_t _used_bytes;
    MemoryAllocator() {}
  public:
    static void AddUsedBytes(uint64_t bytes) {
        _used_bytes += bytes;
    }
    
    static void RemoveUsedBytes(uint64_t bytes) {
        _used_bytes -= bytes;
    }
    
    static void ResetUsedBytes() {
        _used_bytes = 0;
    }
    
    static uint64_t GetUsedBytes() {
        return _used_bytes;
    }
};
uint64_t MemoryAllocator::_used_bytes = 0;

class TestClassBase {
  protected:
    TestClassBase() {}
    
    struct Error {
        uint64_t line = 0;
        std::string code;
        std::string extraMessage;

        Error() {}
        Error(uint64_t line, const std::string& code, const std::string& extraMessage)
        : line(line), code(code), extraMessage(extraMessage) {}
        
        bool Empty() const { return (line == 0) && code.empty() && extraMessage.empty(); }
        bool NotEmpty() const { return !Empty(); }
    };
    
    struct MethodResult {
        std::string methodName;
        Error error;
        uint64_t timeElapsedNanoseconds = 0;
        uint64_t bytesLeaked = 0;
        bool isTimeMeasuring = false;
        
        std::string GetMessage(size_t width) const {
            std::stringstream ss;
            ss << methodName << std::setw((int)(width + 1 - methodName.length())) << ' ';
            
            if (IsSuccess() && isTimeMeasuring) {
                ss << "PASSED <-- " << (double)timeElapsedNanoseconds / 1000000.0 << "ms elapsed\n";
                return ss.str();
            }
            
            if (error.NotEmpty()) {
                ss << "FAILED" << " Line " << error.line << ": "
                << error.code << " <-- " << error.extraMessage << '\n';
                return ss.str();
            }
            
            if (bytesLeaked > 0) {
                ss << "MEMORY LEAK <-- " << bytesLeaked << " bytes leaked\n";
                return ss.str();
            }

            return "";
        }
        
        bool IsSuccess() const {
            return error.Empty() && bytesLeaked == 0;
        }
    };
    
    void PrintResults() {
        const auto successfulMethodsCount = SuccessfulMethodsCount();
        const auto allMethodsCount = _methodResults.size();
        const auto bytesLeaked = GetBytesLeaked();
        const auto longestMethodNameWidth = GetLongestMethodNameWidth();
        
        std::cout << "\n( " << successfulMethodsCount << " / " << allMethodsCount << " )"
                  << " in " << (double)GetTimeElapsed() / 1000000000.0 << "s with " << bytesLeaked << " bytes leaked\n";
        
        std::cout << "=================================================\n";
        for (const auto& result : _methodResults)
            std::cout << result.GetMessage(longestMethodNameWidth);
        std::cout << "=================================================\n\n";
    }
    
    std::string _name;
    std::vector<MethodResult> _methodResults;
  private:
    size_t SuccessfulMethodsCount() const {
        size_t count = 0;
        for (const auto& result : _methodResults)
            if (result.IsSuccess())
                ++count;
        return count;
    }
    
    uint64_t GetTimeElapsed() const {
        size_t timeElapsed = 0;
        for (const auto& result : _methodResults)
            timeElapsed += result.timeElapsedNanoseconds;
        return timeElapsed;
    }
    
    uint64_t GetBytesLeaked() const {
        size_t bytesLeaked = 0;
        for (const auto& result : _methodResults)
            bytesLeaked += result.bytesLeaked;
        return bytesLeaked;
    }
    
    size_t GetLongestMethodNameWidth() const {
        size_t longest = 0;
        for (const auto& result : _methodResults)
            longest = std::max(longest, result.methodName.length());
        return longest;
    }
};

} // namespace UnitTestSystem

void * operator new(size_t n)
{
    void* ptr = malloc(n + sizeof(n));
    size_t* dataPtr = (size_t*)ptr;
    dataPtr[0] = n;
    ptr = (void*)(++dataPtr);
    UnitTestSystem::MemoryAllocator::AddUsedBytes(n);
    return ptr;
}

void operator delete(void * ptr) throw()
{
    size_t* dataPtr = (size_t*)ptr;
    --dataPtr;
    size_t n = *dataPtr;
    ptr = (void*)(dataPtr);
    UnitTestSystem::MemoryAllocator::RemoveUsedBytes(n);
    free(ptr);
}

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
#define TEST_METHOD_END                                                                 \
                catch(const Error& error) {                                             \
                    methodResult.error = error;                                         \
                }                                                                       \
                catch(...) {                                                            \
                    Error error(0, "", "Unknown exception occured!");                   \
                    methodResult.error = error;                                         \
                }                                                                       \
                methodResult.timeElapsedNanoseconds = timer.GetNanoseconds();           \
            }                                                                           \
            if (methodResult.IsSuccess())                                               \
                methodResult.bytesLeaked = MemoryAllocator::GetUsedBytes();             \
            _methodResults.push_back(methodResult);                                     \
            if (_methodResults.size() > 1) {                                            \
                if (methodResult.IsSuccess())                                           \
                    std::cout << 'T';                                                   \
                else                                                                    \
                    std::cout << 'F';                                                   \
            }                                                                           \
        }                                                                               \

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

#define MUST_BE_TRUE(exp) if(!(exp)) throw(Error(__LINE__, #exp, "Expected True but was False"))
#define MUST_BE_FALSE(exp) if(exp) throw(Error(__LINE__, #exp, "Expected False but was True"))

#define MUST_BE_CLOSE_DOUBLES(a, b)                                                  \
if (fabs((a) - (b)) > (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * 1e-5)                \
    throw( Error( __LINE__,                                                          \
                  std::string(#a) + std::string(" ~= ") + std::string(#b),           \
                  std::to_string(a) + std::string (" != ") + std::to_string(b) ))    \

#define MUST_THROW_EXCEPTION(...)                                                                                  \
try {                                                                                                              \
    __VA_ARGS__;                                                                                                   \
    throw Error(__LINE__, #__VA_ARGS__, "There were no exceptions");                                               \
}                                                                                                                  \
catch(const Error& e) {throw;}                                                                                     \
catch(...) { }                                                                                                     \

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
//This character '.' is to force user to write MUST_THROW_EXCEPTION(...); <- with semicolon at the end

