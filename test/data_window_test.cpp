#define TEST_MATRIX_SIZE 5
#define TEST_ARRAY_SIZE TEST_MATRIX_SIZE * TEST_MATRIX_SIZE

#include <gtest/gtest.h>

#include <Eigen/Dense>

#include "geometry/dataWindow.h"

// Initialize the values of the four corners of the DataWindow
//   3 --- 4
//   |     |
//   1 --- 2
// and ensure the same corners have the same value under matrix indices
TEST(DataWindow, data_window_coordinate_mapping_should_be_correct) {
    double *window_array = (double *) malloc(sizeof(double) * TEST_ARRAY_SIZE);
    memset(window_array, 0, sizeof(double) * TEST_ARRAY_SIZE);

    DataWindow<double> window(window_array, TEST_MATRIX_SIZE, TEST_MATRIX_SIZE);
    // The window bottom-left has index (0, 0)
    window(0, 0) = 1;
    // The window bottom-right has index (N, 0)
    window(TEST_MATRIX_SIZE - 1, 0) = 2;
    // The window top-left has index (0, N)
    window(0, TEST_MATRIX_SIZE - 1) = 3;
    // The window top-right has index (N, N)
    window(TEST_MATRIX_SIZE - 1, TEST_MATRIX_SIZE - 1) = 4;

    double *matrix_array = (double *) malloc(sizeof(double) * TEST_ARRAY_SIZE);
    memset(matrix_array, 0, sizeof(double) * TEST_ARRAY_SIZE);

    Eigen::Map<Eigen::MatrixXd> matrix(matrix_array, TEST_MATRIX_SIZE, TEST_MATRIX_SIZE);
    // The matrix bottom-left has index (N, 0)
    matrix(TEST_MATRIX_SIZE - 1, 0) = 1;
    // The matrix bottom-right has index (N, N)
    matrix(TEST_MATRIX_SIZE - 1, TEST_MATRIX_SIZE - 1) = 2;
    // The matrix top-left has index (0, 0)
    matrix(0, 0) = 3;
    // The matrix top-right has index (0, N)
    matrix(0, TEST_MATRIX_SIZE - 1) = 4;


    ASSERT_EQ(std::vector<double>(window_array, window_array + TEST_ARRAY_SIZE),
            std::vector<double>(matrix_array, matrix_array + TEST_ARRAY_SIZE));
}
