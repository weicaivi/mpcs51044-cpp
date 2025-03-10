#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace std;

// Part 1: Basic median using sort
double median_sort(vector<double> vec) {
    sort(vec.begin(), vec.end());
    
    size_t size = vec.size();
    if (size % 2 == 0) {
        return (vec[size/2 - 1] + vec[size/2]) / 2.0;
    } else {
        return vec[size/2];
    }
}

// Part 2: Median using partial_sort
double median_partial_sort(vector<double> vec) {    
    size_t size = vec.size();
    size_t mid = size / 2;
    
    if (size % 2 == 0) {
        // sort up to the middle two elements
        partial_sort(vec.begin(), vec.begin() + mid + 1, vec.end());
        return (vec[mid - 1] + vec[mid]) / 2.0;
    } else {
        // only need to sort up to the middle element
        partial_sort(vec.begin(), vec.begin() + mid + 1, vec.end());
        return vec[mid];
    }
}

// Part 3: Median using nth_element
double median_nth_element(vector<double> vec) {
    size_t size = vec.size();
    size_t mid = size / 2;
    
    if (size % 2 == 0) {
        // need both middle elements
        nth_element(vec.begin(), vec.begin() + mid, vec.end());
        nth_element(vec.begin(), vec.begin() + mid - 1, vec.begin() + mid);
        return (vec[mid - 1] + vec[mid]) / 2.0;
    } else {
        // only need the middle element
        nth_element(vec.begin(), vec.begin() + mid, vec.end());
        return vec[mid];
    }
}

// Part 4 & 5: Template function for any appropriate type
template<typename T>
T median(vector<T> vec) {
    size_t size = vec.size();
    size_t mid = size / 2;
    
    if (size % 2 == 0) {
        nth_element(vec.begin(), vec.begin() + mid, vec.end());
        nth_element(vec.begin(), vec.begin() + mid - 1, vec.begin() + mid);
        return (vec[mid - 1] + vec[mid]) / T(2);  // Explicit conversion to type T
    } else {
        nth_element(vec.begin(), vec.begin() + mid, vec.end());
        return vec[mid];
    }
}

int main() {
    vector<double> data = {3.1, 1.4, 7.2, 4.9, 2.8};
    vector<double> even_data = {3.1, 1.4, 7.2, 4.7, 2.8, 5.5};
    
    cout << "Testing with odd number of elements: ";
    for (double x : data) cout << x << " ";
    cout << "\n";
    
    cout << "Median (sort): " << median_sort(data) << "\n";
    cout << "Median (partial_sort): " << median_partial_sort(data) << "\n";
    cout << "Median (nth_element): " << median_nth_element(data) << "\n";
    cout << "Median (template): " << median(data) << "\n\n";
    
    cout << "Testing with even number of elements: ";
    for (double x : even_data) cout << x << " ";
    cout << "\n";
    
    cout << "Median (sort): " << median_sort(even_data) << "\n";
    cout << "Median (partial_sort): " << median_partial_sort(even_data) << "\n";
    cout << "Median (nth_element): " << median_nth_element(even_data) << "\n";
    cout << "Median (template): " << median(even_data) << "\n";
    
    return 0;
}