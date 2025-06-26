#include "KeyGenerator.hpp"
#include <cmath>

std::vector<int> KeyGenerator::generateVandermondeVector(int seed, int m, int q) {
    std::vector<int> v(m);
    int power = 1;
    for (int i = 0; i < m; ++i) {
        v[i] = power % q;
        power = (power * seed) % q;
    }
    return v;
}

std::vector<int> KeyGenerator::computePrivateKey(const std::vector<int>& V, const Matrix& M, int p) {
    std::vector<int> result = M.multiply(V);
    for (int& val : result) val %= p;
    return result;
}
