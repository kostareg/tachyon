#pragma once

#include "tachyon/common/assert.h"

#include <vector>

// TODO: support for data other than double?
// TODO: support for 3d data? generalise to tensors!
// TODO: look at std::mdspan for this
/**
 * @brief 1-indexed matrix
 */
class Matrix {
    size_t current;
    size_t height;
    size_t width;
    size_t capacity;
    double *data;

  public:
    Matrix() : current(0), height(0), width(0), capacity(0), data(nullptr) {}

    explicit Matrix(size_t capacity)
        : height(0), width(0), capacity(capacity), data(new double[capacity]) {}

    Matrix(size_t height, size_t width)
        : current(0), height(height), width(width), capacity(height * width * 2),
          data(new double[capacity]) {}

    Matrix(size_t height, size_t width, size_t capacity)
        : current(0), height(height), width(width), capacity(capacity), data(new double[capacity]) {
    }

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(const std::vector<double> &list)
        : current(list.size()), height(1), width(list.size()), capacity(height * width * 2),
          data(new double[capacity]) {
        std::uninitialized_copy(list.begin(), list.end(), data);
    }

    /**
     * @brief creates a row matrix (1 by values.size())
     * @param list linear value data
     */
    explicit Matrix(std::vector<double> &&list)
        : current(list.size()), height(1), width(list.size()), capacity(height * width * 2),
          data(new double[capacity]) {
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    /**
     * @brief create a 2-dimensional matrix
     * @param list linear value data
     * @param width width
     */
    Matrix(const std::vector<double> &list, size_t width)
        : current(list.size()), height(list.size() / width), width(width),
          capacity(height * width * 2), data(new double[capacity]) {
        TY_ASSERT("matrix is square" && list.size() % width == 0);
        std::uninitialized_copy(list.begin(), list.end(), data);
    }

    /**
     * @brief create a 2-dimensional matrix
     * @param list linear value data
     * @param width width
     */
    Matrix(std::vector<double> &&list, size_t width)
        : current(list.size()), height(list.size() / width), width(width),
          capacity(height * width * 2), data(new double[capacity]) {
        TY_ASSERT("matrix is square" && list.size() % width == 0);
        std::uninitialized_move(list.begin(), list.end(), data);
    }

    Matrix(const Matrix &other)
        : current(other.current), height(other.height), width(other.width),
          capacity(other.capacity) {
        data = new double[capacity];
        std::uninitialized_copy_n(other.data, height * width, data);
    }

    size_t size() const { return width * height; }

    size_t getCurrent() const { return current; }

    size_t getCapacity() const { return capacity; }

    size_t getWidth() const { return width; }

    size_t getHeight() const { return height; }

    void pushBack(double elem) {
        if (height <= 1) {
            // if row matrix (i.e. array), always increase current width
            ++current;
            ++width;
        } else if (current / height >= width) {
            // if current is on the edge, increase current and height
            ++current;
            ++height;
        } else // else, increase current
            ++current;
        if (size() > capacity) {
            capacity *= 2;
            double *tmp = data;
            data = new double[capacity];
            std::uninitialized_copy_n(tmp, current - 1, data);
            delete[] tmp;
        }
        data[current - 1] = elem;
    }

    double popBack() {
        TY_ALWAYS_ASSERT("for now, pop back only works on row matrices" && height <= 1);
        TY_ALWAYS_ASSERT("not empty" && size() != 0);
        return data[--width];
    }

    Matrix &operator=(const Matrix &other) {
        if (this == &other) return *this;

        current = other.current;
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
    double &operator()(size_t index) const {
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
    double &operator()(size_t row, size_t col) const {
        TY_ASSERT("row and col are over or equal to 1" && row >= 1 && col >= 1);
        TY_ASSERT("row and col are within height and width" && row <= height && col <= width);
        return data[(row - 1) * width + (col - 1)];
    }

    bool operator==(const Matrix &other) const {
        if (data == nullptr && other.data == nullptr) return true;

        if (data == nullptr || other.data == nullptr) return false;

        if (current != other.current || width != other.width || height != other.height)
            return false;

        for (size_t i = 0; i < size(); ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    ~Matrix() { delete[] data; }
};