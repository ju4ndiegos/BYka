#ifndef BYKA_HPP
#define BYKA_HPP

#include <vector>
#include "Matrix.hpp"
#include "Node.hpp"

class BYka {
public:
    BYka(int m, int N, int eta, int p, int q);

    void generateMasterKeys();
    void assignKeysToNode(Node& node);
    std::vector<int> derivePairwiseKey(Node& a, Node& b);

private:
    int m, N, eta, p, q;
    std::vector<Matrix> masterKeys;
};

#endif // BYKA_HPP
