#ifndef FLUIDFINALVER_SPECIALARR_H
#define FLUIDFINALVER_SPECIALARR_H

#include <vector>
#include <cstring>

template<typename T, int FixedRows, int FixedCols>
struct Array {
    T data_[FixedRows][FixedCols]{};  // Хранение данных в фиксированном массиве
    int rows_ = FixedRows;             // Количество строк
    int cols_ = FixedCols;             // Количество столбцов

    // Метод инициализации для фиксированного массива оставлен пустым,
    // так как размеры фиксированы и не требуют дополнительной инициализации
    void initialize(int /*rows*/, int /*cols*/) {}

    // Оператор доступа к элементам массива остался без изменений
    T* operator[](int n) {
        return data_[n];
    }

    // Оператор присваивания
    Array& operator=(const Array& other) {
        if (this != &other) {
            std::memcpy(data_, other.data_, sizeof(data_));
            // rows_ и cols_ фиксированы и не требуют копирования
        }
        return *this;
    }

    // Геттеры для размеров массива
    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
};

// Специализация для динамического размера массива
template<typename T>
struct Array<T, -1, -1> {
    std::vector<std::vector<T>> data_{};  // Хранение данных в динамическом двумерном векторе
    int rows_ = 0;                        // Текущее количество строк
    int cols_ = 0;                        // Текущее количество столбцов

    // Метод инициализации для динамического массива
    void initialize(int rows, int cols) {
        rows_ = rows;
        cols_ = cols;
        data_.assign(rows, std::vector<T>(cols, T{}));
    }

    // Оператор доступа к элементам массива остался без изменений
    std::vector<T>& operator[](int n) {
        return data_[n];
    }

    // Оператор присваивания
    Array& operator=(const Array& other) {
        if (this != &other) {
            data_ = other.data_;
            rows_ = other.rows_;
            cols_ = other.cols_;
        }
        return *this;
    }

    // Геттеры для размеров массива
    int getRows() const { return rows_; }
    int getCols() const { return cols_; }
};

#endif //FLUIDFINALVER_SPECIALARR_H
