#pragma once
#include "Timer.h"
#include "MemoryAllocator.h"
#include <iostream>
#include <iomanip>

namespace UnitTestSystem
{

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
        bool isTimeMeasuring = false;
        
        std::string GetMessage(size_t width) const 
        {
            std::stringstream ss;
            ss << methodName << std::setw((int)(width + 1 - methodName.length())) << ' ';
            
            if (IsFailed()) {
                ss << "FAILED" << " Line " << error.line << ": "
                << error.code << " <-- " << error.extraMessage << '\n';
                return ss.str();
            }
            
            if (IsSuccess() && isTimeMeasuring) {
                ss << "PASSED <-- " << (double)timeElapsedNanoseconds / 1000000.0 << "ms elapsed\n";
                return ss.str();
            }
            
            return "";
        }
        
        bool IsSuccess() const {
            return error.Empty();
        }
        
        bool IsFailed() const {
            return !IsSuccess();
        }
    };
    
    void PrintResults() {
        const auto successfulMethodsCount = SuccessfulMethodsCount();
        const auto allMethodsCount = _methodResults.size();
        const auto longestMethodNameWidth = GetLongestMethodNameWidth();
        
        std::cout << "\n( " << successfulMethodsCount << " / " << allMethodsCount << " )"
                  << " in " << (double)GetTimeElapsed() / 1000000000.0 << "s with " << _bytesLeaked << " bytes leaked\n";
        
        std::cout << "=================================================\n";
        for (const auto& result : _methodResults)
            std::cout << result.GetMessage(longestMethodNameWidth);
        std::cout << "=================================================\n\n";
    }
    
    std::string _name;
    uint64_t _bytesLeaked;
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
    
    size_t GetLongestMethodNameWidth() const {
        size_t longest = 0;
        for (const auto& result : _methodResults)
            longest = std::max(longest, result.methodName.length());
        return longest;
    }
};

} // namespace UnitTestSystem

