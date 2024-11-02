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
        
        std::string GetMessage(size_t methodWidth, size_t descriptionWidth) const
        {
            if (!IsPrint())
                return "";
            
            std::stringstream ss;
            
            const auto description = GetDesription();
            const auto extra = GetExtra();
            const std::string arrow = " <-- ";
            
            ss << methodName << std::setw((int)(methodWidth + 1 - methodName.length())) << ' ';
            ss << description << std::setw((int)(descriptionWidth + arrow.length() - description.length())) << arrow << extra << '\n';
                
            return ss.str();
        }
        
        bool IsPrint() const { return IsFailed() || (IsSuccess() && isTimeMeasuring);}
        
        bool IsSuccess() const { return error.Empty(); }
        bool IsFailed() const { return !IsSuccess(); }
        
        std::string GetDesription() const 
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
                return error.extraMessage;
            } else {
                std::stringstream ss;
                ss << (double)timeElapsedNanoseconds / 1000000.0 << "ms elapsed";
                return ss.str();
            }
        }
    };
    
    void PrintResults() {
        const auto methodWidth = GetLongestMethodNameWidth();
        const auto descriptionWidth = GetLongestDescriptionWidth();
        std::cout << "\n( " << SuccessfulMethodsCount() << " / " << _methodResults.size() << " )"
                  << " in " << (double)GetTimeElapsed() / 1000000000.0 << "s with " << _bytesLeaked << " bytes leaked\n";
        
        std::cout << "=================================================\n";
        for (const auto& result : _methodResults)
            std::cout << result.GetMessage(methodWidth, descriptionWidth);
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
    
    size_t GetLongestDescriptionWidth() const {
        size_t longest = 0;
        for (const auto& result : _methodResults)
            longest = std::max(longest, result.GetDesription().length());
        return longest;
    }
};

} // namespace UnitTestSystem

