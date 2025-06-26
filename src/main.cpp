#include <iostream>
#include "BYka.hpp"
#include "Node.hpp"

int main() {
    int m = 6, N = 2, eta = 3, p = 31, q = 61;

    BYka scheme(m, N, eta, p, q);

    Node A(10, m, eta, N, p, q);
    Node B(20, m, eta, N, p, q);

    scheme.assignKeysToNode(A);
    scheme.assignKeysToNode(B);

    std::vector<int> keyA = scheme.derivePairwiseKey(A, B);
    std::vector<int> keyB = scheme.derivePairwiseKey(B, A);

    std::cout << "Clave A: ";
    int sumA = 0;
    for (int k : keyA) {
        std::cout << k << " ";
        sumA += k;
    }
    std::cout << "\nSuma Clave A: " << sumA << "\n";
    std::cout << "Clave B: ";
    int sumB = 0;
    for (int k : keyB) {
        std::cout << k << " ";
        sumB += k;
    }
    std::cout << "\nSuma Clave B: " << sumB << "\n";
    if (sumA == sumB) {
        std::cout << "Las claves derivadas son iguales." << std::endl;
    } else {
        std::cout << "Las claves derivadas son diferentes." << std::endl;
    }
    std::cout << std::endl;

    return 0;
}
