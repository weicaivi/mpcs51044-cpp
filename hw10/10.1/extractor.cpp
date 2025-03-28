#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include "../../compile-time-regular-expressions/single-header/ctre.hpp"

// Structure to hold the results
struct DecimalNumber {
    std::string before;
    std::string after;
};

std::vector<DecimalNumber> extractUsingStdRegex(const std::string& input) {
    std::vector<DecimalNumber> results;
    
    // Capture groups: (\d+)\.(\d+) captures digits before and after the decimal point
    std::regex pattern(R"((\d+)\.(\d+))");
    
    auto words_begin = std::sregex_iterator(input.begin(), input.end(), pattern);
    auto words_end = std::sregex_iterator();
    
    // Process each match
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        DecimalNumber num;
        num.before = match[1].str(); 
        num.after = match[2].str(); 
        results.push_back(num);
    }
    
    return results;
}

std::vector<DecimalNumber> extractUsingCTRE(const std::string& input) {
    std::vector<DecimalNumber> results;
    
    constexpr auto pattern = ctll::fixed_string(R"((\d+)\.(\d+))");
    
    for (auto match : ctre::range<pattern>(input)) {
        DecimalNumber num;
        num.before = match.get<1>().to_string(); 
        num.after = match.get<2>().to_string(); 
        results.push_back(num);
    }
    
    return results;
}

void printResults(const std::vector<DecimalNumber>& numbers) {
    for (const auto& num : numbers) {
        std::cout << num.before << " is before the decimal and " 
                  << num.after << " is after the decimal" << std::endl;
    }
}

int main() {
    std::string input = "Here are some numbers: 1.23, 4, 5.6, 7.89";
    
    std::cout << "Input string: " << input << std::endl << std::endl;
    
    std::cout << "Results using std::regex:" << std::endl;
    auto stdRegexResults = extractUsingStdRegex(input);
    printResults(stdRegexResults);
    
    std::cout << std::endl;
    
    std::cout << "Results using CTRE:" << std::endl;
    auto ctreResults = extractUsingCTRE(input);
    printResults(ctreResults);
    
    return 0;
}