#pragma once

#include "tachyon/common/assert.h"

#include <vector>

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
    size_t capacity;
    double *data;

  public:
    Matrix() : height(0), width(0), capacity(0), data(nullptr) {}

    explicit Matrix(size_t capacity)
        : height(0), width(0), capacity(capacity), data(new double[capacity])
    {
    }

    explicit Matrix(size_t height, size_t width)
        : height(height), width(width), capacity(height * width * 2), data(new double[capacity])
    {
    }

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(const std::vector<double> &list)
        : height(1), width(list.size()), capacity(height * width * 2), data(new double[capacity])
    {
        std::uninitialized_copy(list.begin(), list.end(), data);
    }

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(std::vector<double> &&list)
        : height(1), width(list.size()), capacity(height * width * 2), data(new double[capacity])
    {
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    /**
     * @brief create a 2-dimensional matrix
     * @param list linear value data
     * @param width width
     */
    Matrix(const std::vector<double> &list, size_t width)
        : height(list.size() / width), width(width), capacity(height * width * 2),
          data(new double[capacity])
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
        : height(list.size() / width), width(width), capacity(height * width * 2),
          data(new double[capacity])
    {
        TY_ASSERT("matrix is square" && list.size() % width == 0);
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    size_t size() const { return width * height; }

    size_t getCapacity() const { return capacity; }

    size_t getWidth() const { return width; }

    size_t getHeight() const { return height; }

    void pushBack(double elem)
    {
        TY_ALWAYS_ASSERT("for now, push back only works on row matrices" && height <= 1);
        ++width;
        if (size() > capacity)
        {
            capacity *= 2;
            double *tmp = data;
            data = new double[capacity];
            std::uninitialized_copy_n(tmp, size() - 1, data);
            delete[] tmp;
        }
        data[size()] = elem;
    }

    double popBack()
    {
        TY_ALWAYS_ASSERT("for now, pop back only works on row matrices" && height <= 1);
        TY_ALWAYS_ASSERT("not empty" && size() != 0);
        return data[--width];
    }

    Matrix &operator=(const Matrix &other)
    {
        if (this == &other)
            return *this;

        height = other.height;
        width = other.width;
        capacity = other.capacity;

        delete[] data;
        data = new double[capacity];

        std::uninitialized_copy_n(other.data, height * width, data);

        return *this;
    }

    /**
     * @brief get reference to value at position
     * @param index index
     * @return value reference
     */
    double &operator()(size_t index) const
    {
        TY_ASSERT("index is over or equal to 1" && index >= 1);
        TY_ASSERT("index is within size" && index <= size());
        return data[index - 1];
    }

    /**
     * @brief get reference to value at position
     * @param row value row
     * @param col value column
     * @return value reference
     */
    double &operator()(size_t row, size_t col) const
    {
        TY_ASSERT("row and col are over or equal to 1" && row >= 1 && col >= 1);
        TY_ASSERT("row and col are within height and width" && row <= height && col <= width);
        return data[(row - 1) * width + (col - 1)];
    }

    bool operator==(const Matrix &other) const
    {
        if (data == nullptr && other.data == nullptr)
            return true;

        if (data == nullptr || other.data == nullptr)
            return false;

        if (width != other.width || height != other.height)
            return false;

        for (size_t i = 0; i < size(); ++i)
        {
            if (data[i] != other.data[i])
                return false;
        }
        return true;
    }

    ~Matrix() { delete[] data; }
};