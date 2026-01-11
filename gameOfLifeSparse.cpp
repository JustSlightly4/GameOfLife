/*
*   Ryan Montgomery
*   01/08/26
*   Conways Game Of Life
*/

#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <random>
using namespace std;

struct pairHash {
    size_t operator()(const std::pair<int,int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

double gettime() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

class GameOfLife {
    private:
        int size;
        int innerSize;
        int totalSize;
        unordered_set<pair<int, int>, pairHash> board;
        unordered_set<pair<int, int>, pairHash> boardNext;
    
    public:
        GameOfLife(int size){
            this->innerSize = size;
            this->size = size+2;
            this->totalSize = (size+2) * (size+2);
        }

        void setTestCase1() {
            board.clear();
            boardNext.clear();

            int center = size/2;
            board.insert({center, center-1});
            board.insert({center-1, center-1});
            board.insert({center+1, center-1});

            board.insert({center, center});
            board.insert({center-1, center});
            board.insert({center+1, center});

            board.insert({center, center+1});
            board.insert({center-1, center+1});
        }

        void setTestCase2() {
            board.clear();
            boardNext.clear();

            int center = size/2;
            board.insert({1, 1});
            board.insert({1, 2});
            board.insert({1, 3});
        }

        void setInfiniteCase() {
            board.clear();
            boardNext.clear();
            int center = size/2;
            board.insert({center-1, center-1});
            board.insert({center, center-1});
            board.insert({center+1, center-1});
            board.insert({center, center});
            board.insert({center, center+1});

        }

        void setRandomCase(int percentLive) {
            board.clear();
            boardNext.clear();

            // Random number generation
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(1, size - 2); // inner board: 1..size-2

            int totalCells = (size - 2) * (size - 2); // only inner cells
            int numLive = totalCells * percentLive / 100;

            for (int i = 0; i < numLive; ++i) {
                int x = dist(gen);
                int y = dist(gen);

                board.insert({x, y});
            }
        }

        bool advanceGeneration() {
            unordered_map<pair<int,int>, int, pairHash> neighborCount;
            boardNext.clear();

            for (const auto& cell : board) {
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        if (i == 0 && j == 0) continue;
                        int nx = cell.first + i;
                        int ny = cell.second + j;
                        if (nx < 1 || nx > innerSize || ny < 1 || ny > innerSize) continue;
                        neighborCount[{cell.first + i, cell.second + j}]++;
                    }
                }
            }

            bool changed = false;

            for (const auto& [cell, count] : neighborCount) {
                bool alive = board.count(cell);

                if (cell.first < 1 || cell.first > innerSize || cell.second < 1 || cell.second > innerSize) continue;

                if (count == 3 || (count == 2 && alive)) {
                    boardNext.insert(cell);
                    if (!alive) changed = true;     // birth
                } else if (alive) {
                    changed = true;                 // death
                }
            }

            if (!changed && boardNext.size() != board.size()) changed = true;
            swap(board, boardNext);
            return changed;
        }



        void printBoard() {
            for (int i = 0; i < size; ++i) {
                for (int j = 0; j < size; ++j) {
                    cout << (board.find({i, j}) != board.end()) << " ";
                }
                cout << "\n";
            }
        }
};

int main(int argc, char **argv) {

    //Check to make sure that their are three command line arguments
    if (argc != 3) {
        printf("Usage: ./a.out <sizeOfBoard> <generations>\n");
        return 1;
    }

    //Initialize Values
    int const sizeOfBoard = atoi(argv[1]);
    int const generations = atoi(argv[2]);

    if (sizeOfBoard < 3) {
        printf("Error: <sizeOfBoard> must be 3 or more!\n");
        return 2;
    }

    if (generations < 0) {
        printf("Error: <generations> must be 0 or more!\n");
        return 3;
    }

    GameOfLife gameOfLife(sizeOfBoard);
    gameOfLife.setRandomCase(40);
    int i = 0;
    double startTime = gettime();
    for (i; i < generations; ++i) {
        if (!gameOfLife.advanceGeneration()) break;
    }
    double endTime = gettime();

    cout << "Stopped at " << i << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    //gameOfLife.printBoard();


    return 0;

}