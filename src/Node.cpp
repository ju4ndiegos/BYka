#include "Node.hpp"
#include "KeyGenerator.hpp"

Node::Node(int id, int m, int eta, int N, int p, int q)
    : id(id), m(m), eta(eta), N(N), p(p), q(q) {}

void Node::generatePublicKeys() {
    publicKeys.clear();
    for (int i = 0; i < eta; ++i) {
        int seed = id + i; // ID como base del vector público
        publicKeys.push_back(KeyGenerator::generateVandermondeVector(seed, m, q));
    }
}

void Node::setPrivateKeys(const std::vector<std::vector<int>>& keys) {
    privateKeys = keys;
}

std::vector<int> Node::derivePairwiseKey(const Node& other) {
    std::vector<int> result;
    const auto& otherPub = other.getPublicKeys();

    for (const auto& priv : privateKeys)
        for (const auto& pub : otherPub) {
            int dot = 0;
            for (int i = 0; i < m; ++i)
                dot = (dot + priv[i] * pub[i]) % p;
            result.push_back(dot);
        }

    return result;
}

int Node::getID() const { return id; }

const std::vector<std::vector<int>>& Node::getPublicKeys() const {
    return publicKeys;
}

const std::vector<std::vector<int>>& Node::getPrivateKeys() const {
    return privateKeys;
}
