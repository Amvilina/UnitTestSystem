#pragma once
#include <chrono>
#include <cstdint>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <functional>
#include <cmath>

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

} // namespace UnitTestSystem

namespace UnitTestSystem
{

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

namespace UnitTestSystem
{

struct Error {
    uint64_t line = 0;
    std::string code;
    std::string message;

    Error() {}
    Error(uint64_t line, const std::string& code, const std::string& message)
    : line(line), code(code), message(message) {}
    
    bool Empty() const { return (line == 0) && code.empty() && message.empty(); }
    bool NotEmpty() const { return !Empty(); }
};

struct Assert {
    uint64_t line;
    std::string code;
    
    Assert(uint64_t line, const std::string& code)
    : line(line), code(code) {}
};

struct FunctionResult {
    std::string name;
    Error error;
    uint64_t timeElapsedNanoseconds = 0;
    bool isTimeMeasuring = false;
    
    bool IsPrint() const { return IsFailed() || (IsSuccess() && isTimeMeasuring);}
    bool IsSuccess() const { return error.Empty(); }
    bool IsFailed() const { return !IsSuccess(); }
    
    std::string GetMessage(size_t longestNameLength, size_t longestDescriptionLength) const
    {
        if (!IsPrint())
            return "";
        
        std::stringstream ss;
        
        const auto description = GetDescription();
        const auto extra = GetExtra();
        const std::string arrow = " <-- ";
        
        ss << name << std::setw((int)(longestNameLength + 1 - name.length())) << ' ';
        ss << description << std::setw((int)(longestDescriptionLength + arrow.length() - description.length())) << arrow << extra << '\n';
            
        return ss.str();
    }
    
    std::string GetDescription() const
    {
        if (IsFailed()) {
            std::stringstream ss;
            ss << "FAILED Line " << error.line << ": " << error.code;
            return ss.str();
        } else {
            return "PASSED ";
        }
        
    }
    
    std::string GetExtra() const
    {
        if (IsFailed()) {
            return error.message;
        } else {
            std::stringstream ss;
            ss << (double)timeElapsedNanoseconds / 1e6 << "ms elapsed";
            return ss.str();
        }
    }
};

template <class T>
class FunctionRegister {
  public:
    FunctionRegister(const std::string& name, const std::function<void()>& testFunction, bool timeMeasuring) {
        T::AddTestFunction(name, testFunction, timeMeasuring);
    }
};

template <class T>
class Base {
  private:
    friend class FunctionRegister<T>;
    
    struct FunctionInfo {
        std::string name;
        std::function<void()> function;
        bool timeMeasuring;
    };
    static inline std::vector<FunctionInfo> _functionsInfo;
    
    static void AddTestFunction(const std::string& name, const std::function<void()>& testFunction, bool timeMeasuring) {
        _functionsInfo.push_back({name, testFunction, timeMeasuring});
    }
  public:
    static void Run() {
        const auto results = RunAndGetResults();
        const auto stats = GetStats(results);
        
        std::cout << T::GetName() << ": ";
        std::cout << "( " << stats.successfulCount << " / " << stats.allCount << " )"
        << " in " << (double)stats.timeElapsed / 1e9 << "s " << (stats.IsSuccess() ? "PASSED\n": "FAILED\n");
        
        const auto lineLength = 6 + stats.longestNameLength + stats.longestDescriptionLength + stats.longestExtraLength;
        PrintLine(lineLength);
        for (const auto& result : results)
            std::cout << result.GetMessage(stats.longestNameLength, stats.longestDescriptionLength);
        PrintLine(lineLength);
        std::cout << std::endl;
    }
  private:
    static std::vector<FunctionResult> RunAndGetResults() {
        std::vector<FunctionResult> results;
        Timer timer;
        
        for (const auto& info : _functionsInfo) {
            FunctionResult result;
            result.name = info.name;
            result.isTimeMeasuring = info.timeMeasuring;
            
            MemoryAllocator::ResetUsedBytes();
            timer.Restart();
            
            try { info.function(); }
            catch (const Error& error) { result.error = error; }
            catch (const Assert& assert) {
                Error error(assert.line, assert.code, "Assert triggered!");
                result.error = error;
            }
            catch (...) {
                Error error(0, "", "Unknown exception occured!");
                result.error = error;
            }
            
            result.timeElapsedNanoseconds = timer.GetNanoseconds();
            
            if (result.IsSuccess()) {
                const auto bytesLeaked = MemoryAllocator::GetUsedBytes();
                if (bytesLeaked > 0){
                    Error error(0, "", "Memory leak: " + std::to_string(bytesLeaked) + " byte(s)");
                    result.error = error;
                }
            }
            
            results.push_back(result);
        }
            
        return results;
    }
    
    struct Stats {
        size_t allCount = 0;
        size_t successfulCount = 0;
        uint64_t timeElapsed = 0;
        size_t longestNameLength = 0;
        size_t longestDescriptionLength = 0;
        size_t longestExtraLength = 0;
        
        bool IsSuccess() const {
            return successfulCount == allCount;
        }
    };
    
    static Stats GetStats(const std::vector<FunctionResult>& results) {
        Stats stats;
        stats.allCount = results.size();
        
        for (const auto& result : results) {
            if (result.IsSuccess())
                ++stats.successfulCount;
            stats.timeElapsed += result.timeElapsedNanoseconds;
            stats.longestNameLength = std::max(stats.longestNameLength, result.name.length());
            stats.longestDescriptionLength = std::max(stats.longestDescriptionLength, result.GetDescription().length());
            stats.longestExtraLength = std::max(stats.longestExtraLength, result.GetExtra().length());
        }
        
        return stats;
    }
    
    static void PrintLine(size_t count) {
        for (size_t i = 0; i < count; ++i)
            std::cout << "=";
        std::cout << '\n';
    }
};

void MustBeTrue(bool a, uint64_t line, const std::string& code) {
    if (!a)
        throw Error(line, code, "Expected True but was False");
}

void MustBeFalse(bool a, uint64_t line, const std::string& code) {
    if (a)
        throw Error(line, code, "Expected False but was True");
}

template <class T1, class T2>
void MustBeEqual(T1 a, T2 b, uint64_t line, const std::string& aCode, const std::string& bCode) {
    if (a != b)
        throw Error(line, aCode + " == " + bCode, std::to_string(a) + " != " + std::to_string(b));
}

void MustBeCloseDoubles(double a, double b, uint64_t line, const std::string& aCode, const std::string& bCode) {
    if (fabs(a - b) > std::max(fabs(a), fabs(b)) * 1e-5)
        throw Error(line, aCode + " ~= " + bCode, std::to_string(a) + " != " + std::to_string(b));
}

} // namespace UnitTestSystem

#define ASSERT(exp) if(!(exp)) throw UnitTestSystem::Assert(__LINE__, #exp)

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
    throw Error(__LINE__, #__VA_ARGS__, "There were no assert triggers");                                          \
}                                                                                                                  \
catch(const Error& e) {throw;}                                                                                     \
catch(const UnitTestSystem::Assert& e) { }                                                                         \
catch(...){throw;}'.'                                                                                              \
//This character '.' is to force user to write MUST_ASSERT(...); <- with semicolon at the end
