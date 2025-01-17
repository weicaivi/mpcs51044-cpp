#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <cmath>

using namespace std;

double square(double x) {
    return x * x;
}

int main() {
    vector<double> numbers = {1.5, 2.7, 3.2, 4.9, 5.1};
    
    vector<double> squared(numbers.size());
    
    // Use transform to square each element
    transform(numbers.begin(), numbers.end(), squared.begin(), square);
    
    cout << "Original vector: ";
    for (const auto& num : numbers) {
        cout << fixed << setprecision(2) << num << " ";
    }
    cout << "\n";
    
    cout << "Squared vector: ";
    for (const auto& num : squared) {
        cout << fixed << setprecision(2) << num << " ";
    }
    cout << "\n";
    
    // Method 1: Calculate distance using accumulate
    double sum_of_squares = accumulate(squared.begin(), squared.end(), 0.0);
    double distance1 = sqrt(sum_of_squares);
    
    // Method 2: Calculate distance using inner_product
    double distance2 = sqrt(inner_product(numbers.begin(), numbers.end(), 
                                                  numbers.begin(), 0.0));
    
    // Method 3: Calculate distance using accumulate with binary operations
    double distance3 = sqrt(accumulate(numbers.begin(), numbers.end(), 0.0,
                               [](double acc, double val) { return acc + val * val; }));
    
    cout << "Distance from origin (using accumulate): " 
              << fixed << setprecision(2) << distance1 << "\n";
    cout << "Distance from origin (using inner_product): " 
              << fixed << setprecision(2) << distance2 << "\n";
    cout << "Distance from origin (using accumulate with lambda): " 
              << fixed << setprecision(2) << distance3 << "\n";
    
    return 0;
}