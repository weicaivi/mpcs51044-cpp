#include<iostream>
#include<vector>
#include <cmath>
#define FMT_HEADER_ONLY
#include <fmt/format.h>
using namespace std;
using namespace fmt;

int getMaxNumberWidth(int numRows) {
    int maxNum = 1;
    for (int n = numRows - 1, k = 0; k <= n / 2; ++k) {
        maxNum = max(maxNum, (k == 0 ? 1 : maxNum * (n - k + 1) / k));
    }
    int width = static_cast<int>(log10(maxNum) + 1);
    return width % 2 == 0 ? width + 1 : width;  // Ensure width is odd
}

void printPascalsTriangle(int numRows) {
    vector<vector<int>> pascal(numRows);
    int maxWidth = getMaxNumberWidth(numRows);

    for (int i = 0; i < numRows; ++i) {
        pascal[i].resize(i + 1);
        pascal[i][0] = pascal[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            pascal[i][j] = pascal[i - 1][j - 1] + pascal[i - 1][j];
        }
    }

    for (int i = 0; i < numRows; ++i) {
        // Calculate initial spacing for brick-wall effect
        cout << string((numRows - i - 1) * (maxWidth / 2 + 1), ' ');
        for (int j = 0; j <= i; ++j) {
            // Print each number centered in a brick of maxWidth
            cout << fmt::format("{:^{}}", pascal[i][j], maxWidth);
            if (j != i) cout << " ";  // Add space between bricks only if not the last element in the row
        }
        cout << endl;
    }
}

int main() {
    int numRows = 8;
    printPascalsTriangle(numRows);
    return 0;
}