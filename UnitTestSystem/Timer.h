#pragma once
#include <chrono>

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




