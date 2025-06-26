#include "Matrix.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

Matrix::Matrix(int size, int mod) : size(size), mod(mod), data(size, std::vector<int>(size)) {}

void Matrix::set(int i, int j, int value) {
    data[i][j] = value % mod;
    if (i != j) data[j][i] = value % mod; // simétrica
}

int Matrix::get(int i, int j) const {
    return data[i][j];
}

void Matrix::generateRandomSymmetric() {
    std::srand(std::time(nullptr));
    for (int i = 0; i < size; ++i) {
        for (int j = i; j < size; ++j) {
            int value = std::rand() % mod;
            set(i, j, value);
        }
    }
}

std::vector<int> Matrix::multiply(const std::vector<int>& vec) const {
    std::vector<int> result(size, 0);
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            result[i] = (result[i] + data[i][j] * vec[j]) % mod;
    return result;
}

void Matrix::print() const {
    for (const auto& row : data) {
        for (int val : row)
            std::cout << val << " ";
        std::cout << "\n";
    }
}

void Matrix::saveToFile(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "No se pudo abrir el archivo para escritura: " << filename << "\n";
        return;
    }

    out << size << " " << mod << "\n";
    for (const auto& row : data) {
        for (int val : row)
            out << val << " ";
        out << "\n";
    }
    out.close();
}

bool Matrix::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "No se pudo abrir el archivo para lectura: " << filename << "\n";
        return false;
    }

    int newSize, newMod;
    in >> newSize >> newMod;

    std::vector<std::vector<int>> newData(newSize, std::vector<int>(newSize));
    for (int i = 0; i < newSize; ++i)
        for (int j = 0; j < newSize; ++j)
            in >> newData[i][j];

    size = newSize;
    mod = newMod;
    data = newData;
    in.close();
    return true;
}

