#ifndef KEYGENERATOR_HPP
#define KEYGENERATOR_HPP

#include <vector>
#include "Matrix.hpp"

class KeyGenerator {
public:
    static std::vector<int> generateVandermondeVector(int seed, int m, int q);
    static std::vector<int> computePrivateKey(
        const std::vector<int>& V, const Matrix& M, int p);
};

#endif // KEYGENERATOR_HPP
