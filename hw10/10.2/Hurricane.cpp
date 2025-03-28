#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "../../compile-time-regular-expressions/single-header/ctre.hpp"

// Structure to store annual hurricane strength data
struct AnnualData {
    int year;
    double tropical_storm_days = 0.0;  
    double cat1_days = 0.0;            
    double cat2_days = 0.0;            
    double cat3_days = 0.0;            
    double cat4_days = 0.0;            
    double cat5_days = 0.0;            
    
    double getTotalHurricaneDays() const {
        return cat1_days + cat2_days + cat3_days + cat4_days + cat5_days;
    }
    
    double getMajorHurricaneDays() const {
        return cat3_days + cat4_days + cat5_days;
    }
    
    double getTotalDays() const {
        return tropical_storm_days + getTotalHurricaneDays();
    }
};

// Map wind speed to Saffir-Simpson category and add to appropriate counter
void addWindSpeedToAnnual(int wind_speed, AnnualData& data) {
    const double dayFraction = 0.25;
    
    if (wind_speed >= 34 && wind_speed <= 63) {
        data.tropical_storm_days += dayFraction;
    } else if (wind_speed >= 64 && wind_speed <= 82) {
        data.cat1_days += dayFraction;
    } else if (wind_speed >= 83 && wind_speed <= 95) {
        data.cat2_days += dayFraction;
    } else if (wind_speed >= 96 && wind_speed <= 112) {
        data.cat3_days += dayFraction;
    } else if (wind_speed >= 113 && wind_speed <= 136) {
        data.cat4_days += dayFraction;
    } else if (wind_speed >= 137) {
        data.cat5_days += dayFraction;
    }
}

std::map<int, AnnualData> processDataWithStdRegex(const std::string& data) {
    std::map<int, AnnualData> annual_data;
    
    std::regex date_regex(R"((\d{2})/(\d{2})/(\d{4}))");
    
    std::regex windspeed_regex(R"(\*(\d+)\s+\d+\s+(\d+)\s+\d+\*(\d+)\s+\d+\s+(\d+)\s+\d+\*)");
    
    std::string line;
    std::istringstream iss(data);
    int current_year = 0;
    
    while (std::getline(iss, line)) {
        if (line.size() < 10 || line.substr(0, 5) == "00000") {
            continue;
        }
        
        std::smatch date_match;
        if (std::regex_search(line, date_match, date_regex)) {
            current_year = std::stoi(date_match[3].str());
            
            if (annual_data.find(current_year) == annual_data.end()) {
                annual_data[current_year].year = current_year;
            }
            
            continue;
        }
        
        std::smatch windspeed_match;
        if (std::regex_search(line, windspeed_match, windspeed_regex)) {
            for (int i = 1; i <= 4; i++) {
                int wind_speed = std::stoi(windspeed_match[i].str());
                addWindSpeedToAnnual(wind_speed, annual_data[current_year]);
            }
        }
    }
    
    return annual_data;
}

std::map<int, AnnualData> processDataWithCTRE(const std::string& data) {
    std::map<int, AnnualData> annual_data;
    
    constexpr auto date_pattern = ctll::fixed_string(R"((\d{2})/(\d{2})/(\d{4}))");
    constexpr auto windspeed_pattern = ctll::fixed_string(R"(\*(\d+)\s+\d+\s+(\d+)\s+\d+\*(\d+)\s+\d+\s+(\d+)\s+\d+\*)");
    
    std::string line;
    std::istringstream iss(data);
    int current_year = 0;
    
    while (std::getline(iss, line)) {
        if (line.size() < 10 || line.substr(0, 5) == "00000") {
            continue;
        }
        
        auto date_match = ctre::match<date_pattern>(line);
        if (date_match) {
            current_year = std::stoi(date_match.get<3>().to_string());
            
            if (annual_data.find(current_year) == annual_data.end()) {
                annual_data[current_year].year = current_year;
            }
            
            continue;
        }
        
        auto windspeed_match = ctre::match<windspeed_pattern>(line);
        if (windspeed_match) {
            if (windspeed_match.get<1>()) {
                int wind_speed = std::stoi(windspeed_match.get<1>().to_string());
                addWindSpeedToAnnual(wind_speed, annual_data[current_year]);
            }
            
            if (windspeed_match.get<2>()) {
                int wind_speed = std::stoi(windspeed_match.get<2>().to_string());
                addWindSpeedToAnnual(wind_speed, annual_data[current_year]);
            }
            
            if (windspeed_match.get<3>()) {
                int wind_speed = std::stoi(windspeed_match.get<3>().to_string());
                addWindSpeedToAnnual(wind_speed, annual_data[current_year]);
            }
            
            if (windspeed_match.get<4>()) {
                int wind_speed = std::stoi(windspeed_match.get<4>().to_string());
                addWindSpeedToAnnual(wind_speed, annual_data[current_year]);
            }
        }
    }
    
    return annual_data;
}

void printResults(const std::map<int, AnnualData>& annual_data) {
    std::cout << std::setw(6) << "Year" 
              << std::setw(8) << "TS" 
              << std::setw(8) << "Cat 1" 
              << std::setw(8) << "Cat 2" 
              << std::setw(8) << "Cat 3" 
              << std::setw(8) << "Cat 4" 
              << std::setw(8) << "Cat 5" 
              << std::setw(10) << "Hurr Days" 
              << std::setw(10) << "Major Days" 
              << std::setw(10) << "Total Days" 
              << std::endl;
    
    std::cout << std::string(84, '-') << std::endl;
    
    std::vector<int> years;
    for (const auto& entry : annual_data) {
        years.push_back(entry.first);
    }
    std::sort(years.begin(), years.end());
    
    for (int year : years) {
        const AnnualData& data = annual_data.at(year);
        std::cout << std::setw(6) << data.year
                  << std::fixed << std::setprecision(1)
                  << std::setw(8) << data.tropical_storm_days
                  << std::setw(8) << data.cat1_days
                  << std::setw(8) << data.cat2_days
                  << std::setw(8) << data.cat3_days
                  << std::setw(8) << data.cat4_days
                  << std::setw(8) << data.cat5_days
                  << std::setw(10) << data.getTotalHurricaneDays()
                  << std::setw(10) << data.getMajorHurricaneDays()
                  << std::setw(10) << data.getTotalDays()
                  << std::endl;
    }
}

int main() {
    std::ifstream file("hurdat_atlantic_1851-2011.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open data file." << std::endl;
        return 1;
    }
    
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    auto std_regex_results = processDataWithStdRegex(data);
    std::cout << "Results using std::regex:" << std::endl;
    printResults(std_regex_results);
    
    std::cout << "\n\n";
    
    auto ctre_results = processDataWithCTRE(data);
    std::cout << "Results using CTRE:" << std::endl;
    printResults(ctre_results);
    
    return 0;
}