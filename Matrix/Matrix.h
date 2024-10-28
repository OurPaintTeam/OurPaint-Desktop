#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <initializer_list>
#include <random>
#include <type_traits>
#include <vector>

template <typename T = double>
class Matrix {
protected:
    using size_type = size_t;
    using iterator_type = size_t;

public:
    Matrix();
    Matrix(size_type row, size_type col);
    Matrix(const Matrix& other);
    Matrix(Matrix&& other) noexcept;
    Matrix(std::initializer_list<std::initializer_list<T>> values);
    ~Matrix();

    Matrix<T>& operator=(const Matrix<T>& other);
    Matrix<T>& operator=(Matrix<T>&& other) noexcept;
    Matrix<T>& operator=(std::initializer_list<std::initializer_list<T>> values); 

    template <typename V>
    friend const Matrix<V> operator+(const Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend const Matrix<V> operator-(const Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend const Matrix<V> operator*(const Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend const Matrix<V> operator*(const Matrix<V>& A, const V& scalar);

    template <typename V>
    friend const Matrix<V> operator/(const Matrix<V>& A, const V& scalar);


    template <typename V>
    friend Matrix<V>& operator+=(Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend Matrix<V>& operator-=(Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend Matrix<V>& operator*=(Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend Matrix<V>& operator*=(Matrix<V>& A, const V& scalar);

    template <typename V>
    friend Matrix<V>& operator/=(Matrix<V>& A, const V& scalar);

    template <typename V>
    friend bool operator==(const Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend bool operator==(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> values);

    template <typename V>
    friend bool operator!=(const Matrix<V>& A, const Matrix<V>& B);

    template <typename V>
    friend bool operator!=(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> values);

    // Element Access
    T& operator()(iterator_type row, iterator_type col);
    T operator()(iterator_type row, iterator_type col) const;


    // Matrix function

    Matrix transpose() const;
    void setTranspose();

    T determinant() const;

    Matrix inverse() const;
    void setInverse();

    Matrix adjoint() const; // TODO

    // Matrix Decomposition
    Matrix svd() const;// TODO
    Matrix lu() const;// TODO
    Matrix cholesky() const;// TODO

    Matrix pseudoInverse() const; // TODO

    T trace() const;
    static T trace(Matrix mat);

    std::vector<T> diag() const;
    static std::vector<T> diag(const Matrix<T>& mat);

    static Matrix ones(size_type size);
    static Matrix ones(size_type row, size_type col);
    void setOnes();

    static Matrix zeroes(size_type size);
    static Matrix zeroes(size_type row, size_type col);
    void setZeroes();

    static Matrix random(size_type size, T leftNum = 0.0, T rightNum = 1.0);
    static Matrix random(size_type row, size_type col, T leftNum = 0.0, T rightNum = 1.0);
    void setRandom(T leftNum = 0.0, T rightNum = 1.0);

    [[nodiscard]] inline size_type row_size() const { return row; }
    [[nodiscard]] inline size_type col_size() const { return col; }

private:
    size_type row;
    size_type col;
    T** matrix;
};

template <typename T>
inline Matrix<T>::Matrix() :row(0), col(0), matrix(nullptr) {};

template <typename T>
inline Matrix<T>::Matrix(size_type row, size_type col) : row(row), col(col) {
    matrix = new T *[row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = T();
        }
    }
};

template <typename T>
inline Matrix<T>::Matrix(const Matrix<T>& other) : row(other.row), col(other.col) {
    matrix = new T * [row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = other.matrix[i][j];
        }
    }
}

template <typename T>
inline Matrix<T>::Matrix(Matrix<T>&& other) noexcept : row(other.row), col(other.col), matrix(other.matrix) {
    other.row = 0;
    other.col = 0;
    other.matrix = nullptr;
}

template<typename T>
inline Matrix<T>::Matrix(std::initializer_list<std::initializer_list<T>> values)
{
    row = values.size();
    col = values.begin()->size();
    matrix = new T * [row];

    typename Matrix<T>::iterator_type i = 0;
    for (const auto& row_values : values) {
        if (row_values.size() != col) {
            throw std::invalid_argument("All rows must have the same number of columns.");
        }
        matrix[i] = new T[col];
        typename Matrix<T>::iterator_type j = 0;
        for (const auto& val : row_values) {
            matrix[i][j] = val;
            ++j;
        }
        ++i;
    }
}

template <typename T>
inline Matrix<T>::~Matrix() {
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++)
    {
        delete[] matrix[i];
    }
    if (row != 0) delete[] matrix;
};

template<typename T>
inline Matrix<T>& Matrix<T>::operator=(const Matrix<T>& other)
{
    if (this == &other) return *this;
    Matrix<T> temp(other);
    std::swap(row, temp.row);
    std::swap(col, temp.col);
    std::swap(matrix, temp.matrix);
    return *this;
}

template<typename T>
inline Matrix<T>& Matrix<T>::operator=(Matrix<T>&& other) noexcept
{
    if (this == &other) return *this;
    Matrix<T> temp(std::move(other));
    std::swap(row, temp.row);
    std::swap(col, temp.col);
    std::swap(matrix, temp.matrix);
    return *this;
}

template<typename T>
inline Matrix<T>& Matrix<T>::operator=(std::initializer_list<std::initializer_list<T>> values)
{
    for (typename Matrix<T>::iterator_type i = 0; i < row; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;

    row = values.size();
    col = values.begin()->size();
    matrix = new T * [row];

    typename Matrix<T>::iterator_type i = 0;
    for (auto& row_list : values) {
        if (row_list.size() != col) {
            throw std::invalid_argument("All rows must have the same number of columns.");
        }
        matrix[i] = new T[col];
        typename Matrix<T>::iterator_type j = 0;
        for (auto& value : row_list) {
            matrix[i][j++] = value;
        }
        i++;
    }

    return *this;
}

template<typename V>
inline const Matrix<V> operator+(const Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.col_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    Matrix<V> result(A);
    return result += B;
}

template<typename V>
inline const Matrix<V> operator-(const Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.col_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for subtract.");
    }
    Matrix<V> result(A);
    return result -= B;
}

template<typename V>
inline const Matrix<V> operator*(const Matrix<V>& A, const Matrix<V>& B)
{
    Matrix<V> result(A);
    return result *= B;
}

template<typename V>
inline const Matrix<V> operator*(const Matrix<V>& A, const V& scalar)
{
    Matrix<V> res(A);
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < A.col_size(); j++) {
            res(i, j) *= scalar;
        }
    }
    return res;
}

template<typename V>
inline const Matrix<V> operator/(const Matrix<V>& A, const V& scalar)
{
    if (scalar == V())
    {
        throw std::runtime_error("scalar shouldn't be zero");
    }
    Matrix<V> res(A);
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < A.col_size(); j++) {
            res(i, j) /= scalar;
        }
    }
    return res;
}

template<typename V>
inline Matrix<V>& operator+=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.row_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    for (size_t i = 0; i < A.row_size(); i++) {
        for (size_t j = 0; j < B.col_size(); j++) {
            A(i, j) += B(i, j);
        }
    }
    return A;
}

template<typename V>
inline Matrix<V>& operator-=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.row_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    for (size_t i = 0; i < A.row_size(); i++) {
        for (size_t j = 0; j < B.col_size(); j++) {
            A(i, j) -= B(i, j);
        }
    }
    return A;
}

template<typename V>
inline Matrix<V>& operator*=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.col_size() != B.row_size()) {
        throw std::invalid_argument("Matrices must have compatible dimensions for multiplication");
    }

    Matrix<V> C(A.row_size(), B.col_size());
    for (size_t i = 0; i < A.row_size(); i++) {
        for (size_t j = 0; j < B.col_size(); j++) {
            for (size_t k = 0; k < A.col_size(); k++) {
                C(i, j) += A(i, k) * B(k, j);
            }
        }
    }
    A = C;
    return A;
}

template<typename V>
inline Matrix<V>& operator*=(Matrix<V>& A, const V& scalar)
{
    A = A * scalar;
    return A;
}

template<typename V>
inline Matrix<V>& operator/=(Matrix<V>& A, const V& scalar)
{
    A = A / scalar;
    return A;
}

template<typename V>
inline bool operator==(const Matrix<V>& A, const Matrix<V>& B) 
{
    if (A.row_size() == B.row_size() && A.col_size() == B.col_size()) 
    {
        for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); ++i)
        {
            for (typename Matrix<V>::iterator_type j = 0; j < A.col_size(); ++j)
            {
                if (A(i, j) != B(i, j)) 
                {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

template<typename V>
inline bool operator==(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> values)
{
    if (A.row_size() != values.size() || A.col_size() != values.begin()->size())
    {
        throw std::invalid_argument("Matrices must have the same size");
    }\
    auto row_it = values.begin();
    for (typename Matrix<V>::size_type i = 0; i < A.row_size(); ++i, ++row_it) 
    {
        auto col_it = row_it->begin();
        for (typename Matrix<V>::size_type j = 0; j < A.col_size(); ++j, ++col_it) 
        {
            if (A(i, j) != *col_it) {
                return false;
            }
        }
    }
    return true;
}

template<typename V>
inline bool operator!=(const Matrix<V>& A, const Matrix<V>& B)
{
    return !(A == B);
}

template<typename V>
inline bool operator!=(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> B)
{
    return !(A == B);
}

template <typename T>
inline T& Matrix<T>::operator()(iterator_type row, iterator_type col) {
    if (row >= this->row || col >= this->col) {
        throw std::out_of_range("Index out of range");
    }
    return matrix[row][col];
}

template <typename T>
inline T Matrix<T>::operator()(iterator_type row, iterator_type col) const {
    if (row >= this->row || col >= this->col) {
        throw std::out_of_range("Index out of range");
    }
    return matrix[row][col];
}

template<typename T>
inline Matrix<T> Matrix<T>::transpose() const
{
    Matrix<T> tra(col, row);
    for (typename Matrix<T>::iterator_type i = 0; i < row; ++i) {
        for (typename Matrix<T>::iterator_type j = 0; j < col; ++j) {
            tra.matrix[j][i] = matrix[i][j];
        }
    }
    return tra;
}

template<typename T>
inline T Matrix<T>::determinant() const
{
    if (row != col) {
        throw std::runtime_error("row != col: matrix is cannot be to find determinant");
    }
    size_type s = row;
    T** tempMatrix = new T*[s];
    for (size_t i = 0; i < s; ++i) {
        tempMatrix[i] = new T[s];
        for (size_t j = 0; j < s; ++j) {
            tempMatrix[i][j] = matrix[i][j];
        }
    }

    T d = T(1);
    T eps = T(10e-20);
    for (size_t i = 0; i < s; ++i) {
        if (std::abs(tempMatrix[i][i]) < eps) {
            for (size_t k = i + 1; k < s; ++k) {
                if (std::abs(tempMatrix[k][i]) > eps) {
                    for (size_t j = 0; j < s; ++j) {
                        std::swap(tempMatrix[i][j], tempMatrix[k][j]);
                    }
                    d *= T(-1);
                    break;
                }
            }
        }
        if (std::abs(tempMatrix[i][i]) < eps) {
            return 0;
        }
        for (size_t k = i + 1; k < s; ++k) {
            T factor = tempMatrix[k][i] / tempMatrix[i][i];
            for (size_t j = i; j < s; ++j) {
                tempMatrix[k][j] -= factor * tempMatrix[i][j];
            }
        }
        d *= tempMatrix[i][i];
    }

    for (typename Matrix<T>::iterator_type i = 0; i < s; i++)
    {
        delete[] tempMatrix[i];
    }
    if (row != 0) delete[] tempMatrix;

    return d;
}

template<typename T>
inline void Matrix<T>::setTranspose()
{
    (*this) = (*this).transpose();
}

template<typename T>
inline Matrix<T> Matrix<T>::inverse() const
{
    if (row != col) {
        throw std::invalid_argument("Inverse can only be computed for square matrices.");
    }

    T det = (*this).determinant();
    if (det == 0) {
        throw std::runtime_error("Matrix is singular and cannot be inverted.");
    }

    T** invMat = new T * [row];
    for (size_t i = 0; i < row; ++i) {
        invMat[i] = new T[col];
    }

    T** tempMatrix = new T * [row];
    for (size_t i = 0; i < row; ++i) {
        tempMatrix[i] = new T[row];
        for (size_t j = 0; j < row; ++j) {
            tempMatrix[i][j] = matrix[i][j];
        }
    }

    for (size_t i = 0; i < row; ++i) {
        for (size_t j = 0; j < row; ++j) {
            invMat[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (size_t i = 0; i < row; ++i) {
        if (tempMatrix[i][i] == 0.0) {
            for (size_t j = i + 1; j < row; ++j) {
                if (tempMatrix[j][i] != 0.0) {
                    for (size_t k = 0; k < row; ++k) {
                        std::swap(tempMatrix[i][k], tempMatrix[j][k]);
                        std::swap(invMat[i][k], invMat[j][k]);
                    }
                    break;
                }
            }
        }

        T factor = tempMatrix[i][i];
        for (size_t j = 0; j < row; ++j) {
            tempMatrix[i][j] /= factor;
            invMat[i][j] /= factor;
        }

        for (size_t j = 0; j < row; ++j) {
            if (j != i) {
                factor = tempMatrix[j][i];
                for (size_t k = 0; k < row; ++k) {
                    tempMatrix[j][k] -= factor * tempMatrix[i][k];
                    invMat[j][k] -= factor * invMat[i][k];
                }
            }
        }
    }

    Matrix<T> result(row, col);
    for (size_t i = 0; i < row; i++) {
        for (size_t j = 0; j < row; j++) {
            result(i, j) = invMat[i][j];
        }
    }

    for (typename Matrix<T>::iterator_type i = 0; i < row; i++)
    {
        delete[] invMat[i];
        delete[] tempMatrix[i];
    }
    delete[] invMat;
    delete[] tempMatrix;

    return result;
}

template<typename T>
inline void Matrix<T>::setInverse()
{
    (*this) = (*this).inverse();
}

template<typename T>
inline Matrix<T> Matrix<T>::ones(size_type size)
{
    Matrix<T> result(size, size);
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            result(i, j) = T(1);
        }
    }
    return result;
}

template<typename T>
inline T Matrix<T>::trace() const
{
    T result = 0.0;
    size_type min_dim = std::min(row, col);
    for (size_type i = 0; i < min_dim; i++)
    {
        result += matrix[i][i];
    }
    return result;
}

template<typename T>
inline T Matrix<T>::trace(Matrix<T> mat)
{
    T result = 0.0;
    size_type min_dim = std::min(mat.row, mat.col);
    for (size_type i = 0; i < min_dim; i++)
    {
        result += mat(i, i);
    }
    return result;
}

template<typename T>
inline std::vector<T> Matrix<T>::diag() const 
{
    std::vector<T> result;
    size_type min_dim = std::min(row, col);
    for (size_type i = 0; i < min_dim; i++) 
    {
        result.push_back(matrix[i][i]);
    }
    return result;
}

template<typename T>
inline std::vector<T> Matrix<T>::diag(const Matrix<T>& mat) 
{
    std::vector<T> result;
    size_type min_dim = std::min(mat.row_size(), mat.col_size());
    for (size_type i = 0; i < min_dim; i++) 
    {
        result.push_back(mat(i, i));
    }
    return result;
}

template<typename T>
inline Matrix<T> Matrix<T>::ones(size_type row, size_type col)
{
    Matrix<T> result(row, col);
    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            result(i, j) = T(1);
        }
    }
    return result;
}

template<typename T>
inline void Matrix<T>::setOnes()
{
    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            matrix[i][j] = T(1);
        }
    }
}

template<typename T>
inline Matrix<T> Matrix<T>::zeroes(size_type size)
{
    Matrix<T> result(size, size);
    for (size_t i = 0; i < size; i++)
    {
        for (size_t j = 0; j < size; j++)
        {
            result(i, j) = T(0);
        }
    }
    return result;
}


template<typename T>
inline Matrix<T> Matrix<T>::zeroes(size_type row, size_type col)
{
    Matrix<T> result(row, col);
    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            result(i, j) = T(0);
        }
    }
    return result;
}

template<typename T>
inline void Matrix<T>::setZeroes()
{
    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            matrix[i][j] = T(0);
        }
    }
}

template<typename T>
inline Matrix<T> Matrix<T>::random(size_type size, T min, T max) {
    Matrix<T> result(size, size);
    // random generator
    std::random_device rd;  // start random number
    std::mt19937 gen(rd()); // Mersenne Twister for generation
    if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        for (size_type i = 0; i < size; ++i) {
            for (size_type j = 0; j < size; ++j) {
                result(i, j) = dist(gen);
            }
        }
    }
    else if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        for (size_type i = 0; i < size; ++i) {
            for (size_type j = 0; j < size; ++j) {
                result(i, j) = dist(gen);
            }
        }
    }
    else {
        static_assert(std::is_arithmetic<T>::value, "Matrix random generation requires an arithmetic type");
    }
    return result;
}

template<typename T>
inline Matrix<T> Matrix<T>::random(size_type row, size_type col, T min, T max) {
    Matrix<T> result(row, col);
    // random generator
    std::random_device rd;  // start random number
    std::mt19937 gen(rd()); // Mersenne Twister for generation
    if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        for (size_type i = 0; i < row; ++i) {
            for (size_type j = 0; j < col; ++j) {
                result(i, j) = dist(gen);
            }
        }
    }
    else if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        for (size_type i = 0; i < row; ++i) {
            for (size_type j = 0; j < col; ++j) {
                result(i, j) = dist(gen);
            }
        }
    }
    else {
        static_assert(std::is_arithmetic<T>::value, "Matrix random generation requires an arithmetic type");
    }
    return result;
}

template<typename T>
inline void Matrix<T>::setRandom(T min, T max)
{
    // random generator
    std::random_device rd;  // start random number
    std::mt19937 gen(rd()); // Mersenne Twister for generation
    if constexpr (std::is_floating_point<T>::value) {
        std::uniform_real_distribution<T> dist(min, max);
        for (size_type i = 0; i < row; ++i) {
            for (size_type j = 0; j < col; ++j) {
                matrix(i, j) = dist(gen);
            }
        }
    }
    else if constexpr (std::is_integral<T>::value) {
        std::uniform_int_distribution<T> dist(min, max);
        for (size_type i = 0; i < row; ++i) {
            for (size_type j = 0; j < col; ++j) {
                matrix(i, j) = dist(gen);
            }
        }
    }
    else {
        static_assert(std::is_arithmetic<T>::value, "Matrix random generation requires an arithmetic type");
    }
}

#endif // MATRIX_H