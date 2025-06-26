#ifndef MATRIX_HPP
#define MATRIX_HPP
#include <string>
#include <vector>

class Matrix {
public:
    Matrix(int size, int mod);
    void set(int i, int j, int value);
    int get(int i, int j) const;
    void generateRandomSymmetric();
    std::vector<int> multiply(const std::vector<int>& vec) const;
    void print() const;
    void saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);

private:
    int mod;
    int size;
    std::vector<std::vector<int>> data;
};

#endif // MATRIX_HPP
