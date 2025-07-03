#pragma once

#include "tachyon/common/assert.h"

#include <vector>

namespace tachyon::runtime
{
// TODO: support for data other than double?
// TODO: support for 3d data?
// TODO: look at std::mdspan for this
/**
 * @brief 1-indexed matrix
 */
class Matrix
{
    size_t height;
    size_t width;
    double *data;

  public:
    Matrix() = delete;

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(const std::vector<double> &list)
        : height(1), width(list.size()), data(new double[list.size()])
    {
        std::uninitialized_copy(list.begin(), list.end(), data);
    }

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(std::vector<double> &&list)
        : height(1), width(list.size()), data(new double[list.size()])
    {
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    /**
     * @brief create a 2-dimensional matrix
     * @param list linear value data
     * @param width width
     */
    Matrix(const std::vector<double> &list, size_t width)
        : height(list.size() / width), width(width), data(new double[list.size()])
    {
        TY_ASSERT("matrix is square" && list.size() % width == 0);
        std::uninitialized_copy(list.begin(), list.end(), data);
    }

    /**
     * @brief create a 2-dimensional matrix
     * @param list linear value data
     * @param width width
     */
    Matrix(std::vector<double> &&list, size_t width)
        : height(list.size() / width), width(width), data(new double[list.size()])
    {
        TY_ASSERT("matrix is square" && list.size() % width == 0);
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    Matrix &operator=(const Matrix &other)
    {
        if (this == &other)
            return *this;

        size_t size = other.width * other.height;

        delete[] data;
        data = new double[size];

        std::uninitialized_copy_n(other.data, size, data);

        return *this;
    }

    /**
     * @brief get reference to value at position
     * @param row value row
     * @param col value column
     * @return value reference
     */
    double &operator()(size_t row, size_t col)
    {
        TY_ASSERT("row and col are over or equal to 1" && row >= 1 && col >= 1);
        TY_ASSERT("row and col are within height and width" && row <= height && col <= width);
        return data[(row - 1) * width + (col - 1)];
    }

    ~Matrix() { delete[] data; }
};
} // namespace tachyon::runtime