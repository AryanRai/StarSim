#pragma once

#include <vector>
#include <array>
#include <initializer_list>

namespace mathcore {

namespace LinearAlgebra {
    // Forward declarations
    template<size_t N> class Vector;
    template<size_t M, size_t N> class Matrix;

    // Dynamic Vector class
    class DynamicVector {
    public:
        std::vector<double> data;

        DynamicVector() = default;
        DynamicVector(size_t size, double initial_value = 0.0);
        DynamicVector(std::initializer_list<double> values);
        
        size_t size() const { return data.size(); }
        double& operator[](size_t index) { return data[index]; }
        const double& operator[](size_t index) const { return data[index]; }
        
        // Vector operations
        DynamicVector operator+(const DynamicVector& other) const;
        DynamicVector operator-(const DynamicVector& other) const;
        DynamicVector operator*(double scalar) const;
        DynamicVector operator/(double scalar) const;
        
        double dot(const DynamicVector& other) const;
        double magnitude() const;
        double magnitudeSquared() const;
        DynamicVector normalized() const;
        
        void normalize();
        void resize(size_t new_size, double value = 0.0);
    };

    // Fixed-size Vector template
    template<size_t N>
    class Vector {
    public:
        std::array<double, N> data;

        Vector() { data.fill(0.0); }
        Vector(std::initializer_list<double> values);
        
        constexpr size_t size() const { return N; }
        double& operator[](size_t index) { return data[index]; }
        const double& operator[](size_t index) const { return data[index]; }
        
        // Vector operations
        Vector<N> operator+(const Vector<N>& other) const;
        Vector<N> operator-(const Vector<N>& other) const;
        Vector<N> operator*(double scalar) const;
        Vector<N> operator/(double scalar) const;
        
        double dot(const Vector<N>& other) const;
        double magnitude() const;
        double magnitudeSquared() const;
        Vector<N> normalized() const;
        
        void normalize();
    };

    // Common vector types
    using Vector2D = Vector<2>;
    using Vector3D = Vector<3>;
    using Vector4D = Vector<4>;

    // Cross product for 3D vectors
    Vector3D cross(const Vector3D& a, const Vector3D& b);

    // Dynamic Matrix class
    class DynamicMatrix {
    public:
        std::vector<std::vector<double>> data;
        size_t rows_, cols_;

        DynamicMatrix() : rows_(0), cols_(0) {}
        DynamicMatrix(size_t rows, size_t cols, double initial_value = 0.0);
        DynamicMatrix(std::initializer_list<std::initializer_list<double>> values);
        
        size_t rows() const { return rows_; }
        size_t cols() const { return cols_; }
        
        std::vector<double>& operator[](size_t row) { return data[row]; }
        const std::vector<double>& operator[](size_t row) const { return data[row]; }
        
        double& operator()(size_t row, size_t col) { return data[row][col]; }
        const double& operator()(size_t row, size_t col) const { return data[row][col]; }
        
        // Matrix operations
        DynamicMatrix operator+(const DynamicMatrix& other) const;
        DynamicMatrix operator-(const DynamicMatrix& other) const;
        DynamicMatrix operator*(const DynamicMatrix& other) const;
        DynamicMatrix operator*(double scalar) const;
        DynamicVector operator*(const DynamicVector& vector) const;
        
        DynamicMatrix transpose() const;
        double determinant() const;
        DynamicMatrix inverse() const;
        
        void resize(size_t new_rows, size_t new_cols, double value = 0.0);
        
        // Static constructors
        static DynamicMatrix identity(size_t size);
        static DynamicMatrix zeros(size_t rows, size_t cols);
        static DynamicMatrix ones(size_t rows, size_t cols);
    };

    // Fixed-size Matrix template
    template<size_t M, size_t N>
    class Matrix {
    public:
        std::array<std::array<double, N>, M> data;

        Matrix() {
            for (auto& row : data) {
                row.fill(0.0);
            }
        }
        
        Matrix(std::initializer_list<std::initializer_list<double>> values);
        
        constexpr size_t rows() const { return M; }
        constexpr size_t cols() const { return N; }
        
        std::array<double, N>& operator[](size_t row) { return data[row]; }
        const std::array<double, N>& operator[](size_t row) const { return data[row]; }
        
        double& operator()(size_t row, size_t col) { return data[row][col]; }
        const double& operator()(size_t row, size_t col) const { return data[row][col]; }
        
        // Matrix operations
        Matrix<M, N> operator+(const Matrix<M, N>& other) const;
        Matrix<M, N> operator-(const Matrix<M, N>& other) const;
        template<size_t P> Matrix<M, P> operator*(const Matrix<N, P>& other) const;
        Matrix<M, N> operator*(double scalar) const;
        Vector<M> operator*(const Vector<N>& vector) const;
        
        Matrix<N, M> transpose() const;
        
        // Static constructors
        static Matrix<M, N> identity() requires (M == N);
        static Matrix<M, N> zeros();
        static Matrix<M, N> ones();
    };

    // Common matrix types
    using Matrix2x2 = Matrix<2, 2>;
    using Matrix3x3 = Matrix<3, 3>;
    using Matrix4x4 = Matrix<4, 4>;

    // Linear algebra algorithms
    namespace Algorithms {
        // Gaussian elimination
        DynamicMatrix gaussianElimination(DynamicMatrix matrix);
        DynamicVector solveLinearSystem(const DynamicMatrix& A, const DynamicVector& b);
        
        // LU Decomposition
        struct LUDecomposition {
            DynamicMatrix L, U;
            std::vector<size_t> permutation;
        };
        
        LUDecomposition luDecompose(const DynamicMatrix& matrix);
        DynamicVector solveLU(const LUDecomposition& lu, const DynamicVector& b);
        
        // QR Decomposition
        struct QRDecomposition {
            DynamicMatrix Q, R;
        };
        
        QRDecomposition qrDecompose(const DynamicMatrix& matrix);
        
        // Eigenvalue/Eigenvector computation
        struct EigenResult {
            std::vector<double> eigenvalues;
            DynamicMatrix eigenvectors;
        };
        
        EigenResult eigenDecomposition(const DynamicMatrix& matrix, double tolerance = 1e-10, int max_iterations = 1000);
        
        // Singular Value Decomposition
        struct SVDResult {
            DynamicMatrix U, V;
            std::vector<double> singular_values;
        };
        
        SVDResult svdDecompose(const DynamicMatrix& matrix);
        
        // Matrix norms
        double frobeniusNorm(const DynamicMatrix& matrix);
        double spectralNorm(const DynamicMatrix& matrix);
        double maxNorm(const DynamicMatrix& matrix);
        
        // Vector norms
        double norm1(const DynamicVector& vector);
        double norm2(const DynamicVector& vector);
        double normInf(const DynamicVector& vector);
        double normP(const DynamicVector& vector, double p);
        
        // Matrix conditioning
        double conditionNumber(const DynamicMatrix& matrix);
        int rank(const DynamicMatrix& matrix, double tolerance = 1e-10);
        
        // Least squares
        DynamicVector leastSquares(const DynamicMatrix& A, const DynamicVector& b);
    }
    
    // Utility functions
    double angle(const DynamicVector& a, const DynamicVector& b);
    DynamicVector project(const DynamicVector& a, const DynamicVector& b);
    DynamicMatrix outerProduct(const DynamicVector& a, const DynamicVector& b);
    
    // Matrix/Vector I/O
    void printMatrix(const DynamicMatrix& matrix);
    void printVector(const DynamicVector& vector);
}

} // namespace mathcore 