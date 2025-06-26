#ifndef NODE_HPP
#define NODE_HPP

#include <vector>

class Node {
public:
    Node(int id, int m, int eta, int N, int p, int q);

    void generatePublicKeys();
    void setPrivateKeys(const std::vector<std::vector<int>>& keys);
    std::vector<int> derivePairwiseKey(const Node& other);

    int getID() const;
    const std::vector<std::vector<int>>& getPublicKeys() const;
    const std::vector<std::vector<int>>& getPrivateKeys() const;

private:
    int id, m, eta, N, p, q;
    std::vector<std::vector<int>> publicKeys;
    std::vector<std::vector<int>> privateKeys;
};

#endif // NODE_HPP
