#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<int> vect = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    for (int i = 0; i < vect.size(); ++i) {
        int y = i / 3;
        int x = i % 3;
        cout << x << ", " << y << "\n";
    }
}