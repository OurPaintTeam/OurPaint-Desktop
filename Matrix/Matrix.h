#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <stdexcept>
#include <cmath>
#include <initializer_list>
#include <random>
#include <type_traits>
#include <vector>
#include <concepts>

// Main concept
template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

// Helping concept
template <typename T>
concept IsVector = requires {
    typename T::value_type;
    requires std::same_as<T, std::vector<typename T::value_type>>;
};

// Concept for vector of non-vector elements
template <typename T>
concept VectorType = IsVector<T> && (!IsVector<typename T::value_type>);

// Concept for vector of vectors
template <typename T>
concept VectorVectorType = IsVector<T> && IsVector<typename T::value_type>;

// The matrix class
template <Arithmetic T = double>
class Matrix {
protected:
    using size_type = size_t;
    using iterator_type = size_t;

public:
    Matrix();
    Matrix(const Matrix& other);
    Matrix(Matrix&& other) noexcept;
    ~Matrix();

    explicit Matrix(size_type size);
    explicit Matrix(size_type row, size_type col);
    explicit Matrix(size_type row, size_type col, T value);

    template <VectorVectorType V>
    explicit Matrix(V vec);

    template <VectorType V>
    explicit Matrix(V vec);

    Matrix(std::initializer_list<std::initializer_list<T>> values);


    // operators

    Matrix<T>& operator=(const Matrix<T>& other);
    Matrix<T>& operator=(Matrix<T>&& other) noexcept;
    Matrix<T>& operator=(std::initializer_list<std::initializer_list<T>> values); 

    // Matrix and Matrix operators

    template <Arithmetic V>
    friend const Matrix<V> operator+(const Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend const Matrix<V> operator-(const Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend const Matrix<V> operator*(const Matrix<V>& A, const Matrix<V>& B);


    template <Arithmetic V>
    friend Matrix<V>& operator+=(Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend Matrix<V>& operator-=(Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend Matrix<V>& operator*=(Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend bool operator==(const Matrix<V>& A, const Matrix<V>& B);

    template <Arithmetic V>
    friend bool operator!=(const Matrix<V>& A, const Matrix<V>& B);

    // Matrix and scalar operators

    template <Arithmetic V>
    friend const Matrix<V> operator+(const Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend const Matrix<V> operator-(const Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend const Matrix<V> operator*(const Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend const Matrix<V> operator/(const Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend Matrix<V>& operator+=(Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend Matrix<V>& operator-=(Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend Matrix<V>& operator*=(Matrix<V>& A, const V& scalar);

    template <Arithmetic V>
    friend Matrix<V>& operator/=(Matrix<V>& A, const V& scalar);

    // Matrix and List operators

    template <Arithmetic V>
    friend const Matrix<V> operator+(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend const Matrix<V> operator-(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend const Matrix<V> operator*(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);


    template <Arithmetic V>
    friend Matrix<V>& operator+=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend Matrix<V>& operator-=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend Matrix<V>& operator*=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend bool operator==(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    template <Arithmetic V>
    friend bool operator!=(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L);

    // Element Access
    T& operator()(iterator_type row, iterator_type col);
    T operator()(iterator_type row, iterator_type col) const;


    // Matrix function

    Matrix transpose() const;
    void setTranspose();

    T determinant() const;
    static T determinant(const Matrix<T>& mat);

    Matrix inverse() const;
    void setInverse();

    Matrix adjoint(const size_type& i, const size_type& j) const;                       // TODO
    static Matrix adjoint(const size_type& i, const size_type& j, const Matrix<T>& mat);// TODO

    T minor(const size_type& i, const size_type& j) const;
    static T minor(const size_type& i, const size_type& j, const Matrix<T>& mat);

    // Matrix Decomposition
    Matrix svd() const;// TODO
    Matrix lu() const;// TODO
    Matrix cholesky() const;// TODO
    Matrix pseudoInverse() const; // TODO

    T trace() const;
    static T trace(const Matrix& mat);

    std::vector<T> diag() const;
    static std::vector<T> diag(const Matrix<T>& mat);

    static Matrix ones(const size_type& size);
    static Matrix ones(const size_type& row, const size_type& col);
    void setOnes();

    static Matrix zeroes(const size_type& size);
    static Matrix zeroes(const size_type& row, const size_type& col);
    void setZeroes();

    static Matrix random(const size_type& size, const T& leftNum = 0.0, const T& rightNum = 1.0);
    static Matrix random(const size_type& row, const size_type& col, const T& leftNum = 0.0, const T& rightNum = 1.0);
    void setRandom(const T& leftNum = 0.0, const T& rightNum = 1.0);

    [[nodiscard]] inline size_type row_size() const { return row; }
    [[nodiscard]] inline size_type col_size() const { return col; }

private:
    size_type row;
    size_type col;
    T** matrix;
};

template <Arithmetic T>
inline Matrix<T>::Matrix() : row(0), col(0), matrix(nullptr) {};

template <Arithmetic T>
inline Matrix<T>::Matrix(size_type row, size_type col) : row(row), col(col) 
{
    matrix = new T *[row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = T();
        }
    }
};

template<Arithmetic T>
inline Matrix<T>::Matrix(size_type row, size_type col, T value) : row(row), col(col) 
{
    matrix = new T * [row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = value;
        }
    }
}

template<Arithmetic T>
inline Matrix<T>::Matrix(size_type size) : row(size), col(size)
{
    matrix = new T * [size];
    for (typename Matrix<T>::iterator_type i = 0; i < size; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < size; j++) {
            matrix[i][j] = T();
        }
    }
}

template<Arithmetic T>
template<VectorType V>
inline Matrix<T>::Matrix(V vec) : row(1), col(vec.size())
{
    matrix = new T * [row];
    matrix[0] = new T[col];
    for (typename Matrix<T>::iterator_type i = 0; i < col; i++) {
        matrix[0][i] = vec[i];
    }
}

template<Arithmetic T>
template<VectorVectorType V>
inline Matrix<T>::Matrix(V vec) : row(vec.size()), col(vec[0].size())
{
    matrix = new T * [row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = vec[i][j];
        }
    }
}

template <Arithmetic T>
inline Matrix<T>::Matrix(const Matrix<T>& other) : row(other.row), col(other.col) {
    matrix = new T * [row];
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++) {
        matrix[i] = new T[col];
        for (typename Matrix<T>::iterator_type j = 0; j < col; j++) {
            matrix[i][j] = other.matrix[i][j];
        }
    }
}

template <Arithmetic T>
inline Matrix<T>::Matrix(Matrix<T>&& other) noexcept : row(other.row), col(other.col), matrix(other.matrix) {
    other.row = 0;
    other.col = 0;
    other.matrix = nullptr;
}

template<Arithmetic T>
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

template <Arithmetic T>
inline Matrix<T>::~Matrix() {
    for (typename Matrix<T>::iterator_type i = 0; i < row; i++)
    {
        delete[] matrix[i];
    }
    if (row != 0) delete[] matrix;
};

template<Arithmetic T>
inline Matrix<T>& Matrix<T>::operator=(const Matrix<T>& other)
{
    if (this == &other) return *this;
    Matrix<T> temp(other);
    std::swap(row, temp.row);
    std::swap(col, temp.col);
    std::swap(matrix, temp.matrix);
    return *this;
}

template<Arithmetic T>
inline Matrix<T>& Matrix<T>::operator=(Matrix<T>&& other) noexcept
{
    if (this == &other) return *this;
    Matrix<T> temp(std::move(other));
    std::swap(row, temp.row);
    std::swap(col, temp.col);
    std::swap(matrix, temp.matrix);
    return *this;
}

template<Arithmetic T>
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

template<Arithmetic V>
inline const Matrix<V> operator+(const Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.col_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    Matrix<V> result(A);
    return result += B;
}

template<Arithmetic V>
inline const Matrix<V> operator-(const Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.col_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for subtract.");
    }
    Matrix<V> result(A);
    return result -= B;
}

template<Arithmetic V>
inline const Matrix<V> operator*(const Matrix<V>& A, const Matrix<V>& B)
{
    Matrix<V> result(A);
    return result *= B;
}

template<Arithmetic V>
inline const Matrix<V> operator+(const Matrix<V>& A, const V& scalar)
{
    Matrix<V> res(A);
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < A.col_size(); j++) {
            res(i, j) += scalar;
        }
    }
    return res;
}

template<Arithmetic V>
inline const Matrix<V> operator-(const Matrix<V>& A, const V& scalar)
{
    Matrix<V> res(A);
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < A.col_size(); j++) {
            res(i, j) -= scalar;
        }
    }
    return res;
}

template<Arithmetic V>
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

template<Arithmetic V>
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

template<Arithmetic V>
inline Matrix<V>& operator+=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.cow_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < B.col_size(); j++) {
            A(i, j) += B(i, j);
        }
    }
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator-=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.row_size() != B.row_size() || A.cow_size() != B.col_size()) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < B.col_size(); j++) {
            A(i, j) -= B(i, j);
        }
    }
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator*=(Matrix<V>& A, const Matrix<V>& B)
{
    if (A.col_size() != B.row_size()) {
        throw std::invalid_argument("Matrices must have compatible dimensions for multiplication");
    }

    Matrix<V> C(A.row_size(), B.col_size());
    for (typename Matrix<V>::iterator_type i = 0; i < A.row_size(); i++) {
        for (typename Matrix<V>::iterator_type j = 0; j < B.col_size(); j++) {
            for (typename Matrix<V>::iterator_type k = 0; k < A.col_size(); k++) {
                C(i, j) += A(i, k) * B(k, j);
            }
        }
    }
    A = C;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator+=(Matrix<V>& A, const V& scalar)
{
    A = A + scalar;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator-=(Matrix<V>& A, const V& scalar)
{
    A = A - scalar;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator*=(Matrix<V>& A, const V& scalar)
{
    A = A * scalar;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator/=(Matrix<V>& A, const V& scalar)
{
    A = A / scalar;
    return A;
}

template<Arithmetic V>
inline const Matrix<V> operator+(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L)
{
    typename Matrix<V>::size_type r = L.size();
    typename Matrix<V>::size_type c = L.begin()->size();
    if (A.row_size() != r || A.col_size() != c) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    Matrix<V> result(A);
    auto row_it = L.begin();
    for (typename Matrix<V>::iterator_type i = 0; i < r; ++i, ++row_it) {
        auto col_it = row_it->begin();
        for (typename Matrix<V>::iterator_type j = 0; j < c; ++j, ++col_it) {
            result(i, j) += *col_it;
        }
    }
    return result;
}

template<Arithmetic V>
inline const Matrix<V> operator-(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L)
{
    typename Matrix<V>::size_type r = L.size();
    typename Matrix<V>::size_type c = L.begin()->size();
    if (A.row_size() != r || A.col_size() != c) {
        throw std::invalid_argument("Matrices must have the same size for addition.");
    }
    Matrix<V> result(A);
    auto row_it = L.begin();
    for (typename Matrix<V>::iterator_type i = 0; i < r; ++i, ++row_it) {
        auto col_it = row_it->begin();
        for (typename Matrix<V>::iterator_type j = 0; j < c; ++j, ++col_it) {
            result(i, j) -= *col_it;
        }
    }
    return result;
}

template <Arithmetic V>
inline const Matrix<V> operator*(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L) {
    typename Matrix<V>::size_type L_rows = L.size();
    if (L_rows == 0) {
        throw std::invalid_argument("Second matrice not be empty.");
    }
    typename Matrix<V>::size_type L_cols = L.begin()->size();
    for (const auto& row : L) {
        if (row.size() != L_cols) {
            throw std::invalid_argument("All string int second initializer_list should be one count of cols.");
        }
    }
    if (A.col_size() != L_rows) {
        throw std::invalid_argument("A rows != B cols.");
    }
    Matrix<V> C(A.row_size(), L_cols);
    std::vector<std::vector<V>> L_matrix;
    L_matrix.reserve(L_rows);
    for (const auto& row : L) {
        L_matrix.emplace_back(row);
    }
    for (typename Matrix<V>::size_type i = 0; i < A.row_size(); ++i) {
        for (typename Matrix<V>::size_type j = 0; j < L_cols; ++j) {
            V sum = V{};
            for (typename Matrix<V>::size_type k = 0; k < A.col_size(); ++k) {
                sum += A(i, k) * L_matrix[k][j];
            }
            C(i, j) = sum;
        }
    }

    return C;
}

template<Arithmetic V>
inline Matrix<V>& operator+=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L)
{
    A = A + L;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator-=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L)
{
    A = A - L;
    return A;
}

template<Arithmetic V>
inline Matrix<V>& operator*=(Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> L)
{
    A = A * L;
    return A;
}

template<Arithmetic V>
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

template<Arithmetic V>
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

template<Arithmetic V>
inline bool operator!=(const Matrix<V>& A, const Matrix<V>& B)
{
    return !(A == B);
}

template<Arithmetic V>
inline bool operator!=(const Matrix<V>& A, const std::initializer_list<std::initializer_list<V>> B)
{
    return !(A == B);
}

template <Arithmetic T>
inline T& Matrix<T>::operator()(iterator_type row, iterator_type col) {
    if (row >= this->row || col >= this->col) {
        throw std::out_of_range("Index out of range");
    }
    return matrix[row][col];
}

template <Arithmetic T>
inline T Matrix<T>::operator()(iterator_type row, iterator_type col) const {
    if (row >= this->row || col >= this->col) {
        throw std::out_of_range("Index out of range");
    }
    return matrix[row][col];
}

template<Arithmetic T>
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

template<Arithmetic T>
inline T Matrix<T>::determinant() const
{
    if (row != col) {
        throw std::runtime_error("row != col: matrix is cannot be to find determinant");
    }
    size_type s = row;
    T** tempMatrix = new T*[s];
    for (size_type i = 0; i < s; ++i) {
        tempMatrix[i] = new T[s];
        for (size_type j = 0; j < s; ++j) {
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

template<Arithmetic T>
inline T Matrix<T>::determinant(const Matrix<T>& mat)
{
    return mat.determinant();
}

template<Arithmetic T>
inline void Matrix<T>::setTranspose()
{
    (*this) = (*this).transpose();
}

template<Arithmetic T>
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
    for (size_type i = 0; i < row; ++i) {
        invMat[i] = new T[col];
    }

    T** tempMatrix = new T * [row];
    for (size_type i = 0; i < row; ++i) {
        tempMatrix[i] = new T[row];
        for (size_type j = 0; j < row; ++j) {
            tempMatrix[i][j] = matrix[i][j];
        }
    }

    for (size_type i = 0; i < row; ++i) {
        for (size_type j = 0; j < row; ++j) {
            invMat[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (size_type i = 0; i < row; ++i) {
        if (tempMatrix[i][i] == 0.0) {
            for (size_type j = i + 1; j < row; ++j) {
                if (tempMatrix[j][i] != 0.0) {
                    for (size_type k = 0; k < row; ++k) {
                        std::swap(tempMatrix[i][k], tempMatrix[j][k]);
                        std::swap(invMat[i][k], invMat[j][k]);
                    }
                    break;
                }
            }
        }

        T factor = tempMatrix[i][i];
        for (size_type j = 0; j < row; ++j) {
            tempMatrix[i][j] /= factor;
            invMat[i][j] /= factor;
        }

        for (size_type j = 0; j < row; ++j) {
            if (j != i) {
                factor = tempMatrix[j][i];
                for (size_type k = 0; k < row; ++k) {
                    tempMatrix[j][k] -= factor * tempMatrix[i][k];
                    invMat[j][k] -= factor * invMat[i][k];
                }
            }
        }
    }

    Matrix<T> result(row, col);
    for (size_type i = 0; i < row; i++) {
        for (size_type j = 0; j < row; j++) {
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

template<Arithmetic T>
inline void Matrix<T>::setInverse()
{
    (*this) = (*this).inverse();
}

template<Arithmetic T>
inline T Matrix<T>::minor(const size_type& ix, const size_type& jx) const
{
    if (row != col) {
        throw std::invalid_argument("Matrix should be square!");
    }
    std::vector<std::vector<T>> submatrix;
    for (size_type i = 0; i < row; ++i) 
    {
        if (i == ix) continue;
        std::vector<T> subrow;
        for (size_type j = 0; j < row; ++j) 
        {
            if (j == jx) continue;
            subrow.push_back(matrix[i][j]);
        }
        submatrix.push_back(subrow);
    }
    Matrix<T> res(row - 1, col - 1);
    for (size_type i = 0; i < row - 1; i++)
    {
        for (size_type j = 0; j < col - 1; j++)
        {
            res(i, j) = submatrix[i][j];
        }
    }
    return determinant(res);
}

template<Arithmetic T>
inline T Matrix<T>::minor(const size_type& ix, const size_type& jx, const Matrix<T>& mat)
{
    if (mat.row != mat.col) {
        throw std::invalid_argument("Matrix should be square!");
    }
    std::vector<std::vector<T>> submatrix;
    for (size_type i = 0; i < mat.row; ++i)
    {
        if (i == ix) continue;
        std::vector<T> subrow;
        for (size_type j = 0; j < mat.row; ++j)
        {
            if (j == jx) continue;
            subrow.push_back(mat(i, j));
        }
        submatrix.push_back(subrow);
    }
    Matrix<T> res(mat.row - 1, mat.col - 1);
    for (size_type i = 0; i < mat.row - 1; i++)
    {
        for (size_type j = 0; j < mat.col - 1; j++)
        {
            res(i, j) = submatrix[i][j];
        }
    }
    return determinant(res);
}

template<Arithmetic T>
inline Matrix<T> Matrix<T>::ones(const size_type& size)
{
    Matrix<T> result(size, size);
    for (size_type i = 0; i < size; i++)
    {
        for (size_type j = 0; j < size; j++)
        {
            result(i, j) = T(1);
        }
    }
    return result;
}

template<Arithmetic T>
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

template<Arithmetic T>
inline T Matrix<T>::trace(const Matrix<T>& mat)
{
    T result = 0.0;
    size_type min_dim = std::min(mat.row, mat.col);
    for (size_type i = 0; i < min_dim; i++)
    {
        result += mat(i, i);
    }
    return result;
}

template<Arithmetic T>
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

template<Arithmetic T>
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

template<Arithmetic T>
inline Matrix<T> Matrix<T>::ones(const size_type& row, const size_type& col)
{
    Matrix<T> result(row, col);
    for (size_type i = 0; i < row; i++)
    {
        for (size_type j = 0; j < col; j++)
        {
            result(i, j) = T(1);
        }
    }
    return result;
}

template<Arithmetic T>
inline void Matrix<T>::setOnes()
{
    for (size_type i = 0; i < row; i++)
    {
        for (size_type j = 0; j < col; j++)
        {
            matrix[i][j] = T(1);
        }
    }
}

template<Arithmetic T>
inline Matrix<T> Matrix<T>::zeroes(const size_type& size)
{
    Matrix<T> result(size, size);
    for (size_type i = 0; i < size; i++)
    {
        for (size_type j = 0; j < size; j++)
        {
            result(i, j) = T(0);
        }
    }
    return result;
}


template<Arithmetic T>
inline Matrix<T> Matrix<T>::zeroes(const size_type& row, const size_type& col)
{
    Matrix<T> result(row, col);
    for (size_type i = 0; i < row; i++)
    {
        for (size_type j = 0; j < col; j++)
        {
            result(i, j) = T(0);
        }
    }
    return result;
}

template<Arithmetic T>
inline void Matrix<T>::setZeroes()
{
    for (size_type i = 0; i < row; i++)
    {
        for (size_type j = 0; j < col; j++)
        {
            matrix[i][j] = T(0);
        }
    }
}

template<Arithmetic T>
inline Matrix<T> Matrix<T>::random(const size_type& size, const T& min, const T& max) {
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

template<Arithmetic T>
inline Matrix<T> Matrix<T>::random(const size_type& row, const size_type& col, const T& min, const T& max) {
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

template<Arithmetic T>
inline void Matrix<T>::setRandom(const T& min, const T& max)
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