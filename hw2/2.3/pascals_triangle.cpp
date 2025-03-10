#include <iostream>
#include <vector>
#include <cmath>
#define FMT_HEADER_ONLY
#include <fmt/format.h>

using namespace std;

class PascalsTriangle {
private:
    int numRows;
    vector<vector<int>> triangle;
    int maxWidth;

    int calculateMaxWidth() const {
        int maxNum = 1;
        for (int n = numRows - 1, k = 0; k <= n / 2; ++k) {
            maxNum = max(maxNum, (k == 0 ? 1 : maxNum * (n - k + 1) / k));
        }
        int width = static_cast<int>(log10(maxNum) + 1);
        return width % 2 == 0 ? width + 1 : width;
    }

    void generateTriangle() {
        triangle.resize(numRows);
        for (int i = 0; i < numRows; ++i) {
            triangle[i].resize(i + 1);
            triangle[i][0] = triangle[i][i] = 1;
            for (int j = 1; j < i; ++j) {
                triangle[i][j] = triangle[i - 1][j - 1] + triangle[i - 1][j];
            }
        }
    }

public:
    explicit PascalsTriangle(int rows) : numRows(rows) {
        maxWidth = calculateMaxWidth();
        generateTriangle();
    }

    // Getter for a specific value in the triangle
    int getValue(int row, int col) const {
        if (row >= 0 && row < numRows && col >= 0 && col <= row) {
            return triangle[row][col];
        }
        return -1; // Error value
    }

    // Getter for a complete row
    vector<int> getRow(int row) const {
        if (row >= 0 && row < numRows) {
            return triangle[row];
        }
        return vector<int>(); // Empty vector for invalid row
    }

    void print() const {
        for (int i = 0; i < numRows; ++i) {
            // Calculate initial spacing for brick-wall effect
            cout << string((numRows - i - 1) * (maxWidth / 2 + 1), ' ');
            
            for (int j = 0; j <= i; ++j) {
                // Print each number centered in a brick of maxWidth
                cout << fmt::format("{:^{}}", triangle[i][j], maxWidth);
                if (j != i) cout << " ";
            }
            cout << endl;
        }
    }
};

int main() {
    PascalsTriangle pascal(8);
    pascal.print();
    return 0;
}