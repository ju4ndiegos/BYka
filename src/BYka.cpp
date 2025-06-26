#include "BYka.hpp"
#include "KeyGenerator.hpp"
#include <string> 

BYka::BYka(int m, int N, int eta, int p, int q)
    : m(m), N(N), eta(eta), p(p), q(q) {
    generateMasterKeys();
}

void BYka::generateMasterKeys() {
    masterKeys.clear();
    for (int i = 0; i < N; ++i) {
        Matrix M(m, p);
        if (M.loadFromFile("master_key_" + std::to_string(i) + ".txt")){
            masterKeys.push_back(M);
        } else {
            M.generateRandomSymmetric();
            masterKeys.push_back(M);
            M.saveToFile("master_key_" + std::to_string(i) + ".txt");
        }
    }
}

void BYka::assignKeysToNode(Node& node) {
    node.generatePublicKeys();
    std::vector<std::vector<int>> privKeys;

    for (int i = 0; i < eta; ++i) {
        const auto& V = node.getPublicKeys()[i];
        for (int j = 0; j < N; ++j) {
            auto priv = KeyGenerator::computePrivateKey(V, masterKeys[j], p);
            privKeys.push_back(priv);
        }
    }

    node.setPrivateKeys(privKeys);
}

std::vector<int> BYka::derivePairwiseKey(Node& a, Node& b) {
    return a.derivePairwiseKey(b); // a y b generan lo mismo
}
