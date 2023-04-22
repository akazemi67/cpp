#include <iostream>
#include <iterator>

/*
 * This challenge is from the book: "The Modern C++ Challenge (2018)"
 * Here is the problem statement:
 *
 * Write a class template that represents a two-dimensional array container with methods for
 * element access (at() and data()), capacity querying, iterators, filling, and swapping. It
 * should be possible to move objects of this type.
 */

template<typename T, int cols>
class Row{
private:
    T data[cols]{};
public:
    const size_t Cols = cols;

    T& operator[](int colIdx){
        if(colIdx<0 || colIdx>=Cols)
            throw std::out_of_range("Invalid column index.");
        return data[colIdx];
    }

    const T& operator[](int colIdx) const{
        if(colIdx<0 || colIdx>=Cols)
            throw std::out_of_range("Invalid column index.");
        return data[colIdx];
    }
};

template<typename T, int rows, int cols>
class Matrix{
private:
    Row<T, cols> *data = new Row<T, cols>[rows];
public:
    const size_t Rows = rows;
    const size_t Cols = cols;

    Matrix(T filler){
        std::cout << "Filler Constructor Called!" << std::endl;
        for(int r=0; r<rows; r++) {
            for (int c = 0; c < cols; c++) {
                data[r][c] = filler;
            }
        }
    }
    Matrix(std::initializer_list<T> lst){
        if(lst.size() > (Rows*Cols))
            throw std::out_of_range("Number of items is more than matrix capacity.");
        int r=0;
        int c=0;
        for(auto item : lst){
            if(c>=Cols)
                c=0, r++;
            data[r][c++] = item;
        }
    }
    Matrix(const Matrix<T, rows, cols> &mat){
        std::cout << "Copy Constructor Called!" << std::endl;
        for(int r=0; r<rows; r++) {
            for (int c = 0; c < cols; c++) {
                data[r][c] = mat.data[r][c];
            }
        }
    }
    Matrix(Matrix<T, rows, cols> &&mat){
        std::cout << "Move Constructor Called!" << std::endl;
        data = mat.data;
        mat.data = nullptr;
    }

    Row<T, cols>& operator[](int rowIdx){
        if(rowIdx<0 || rowIdx>=Rows)
            throw std::out_of_range("Invalid row index.");
        return data[rowIdx];
    }
    const Row<T, cols>& operator[](int rowIdx) const{
        if(rowIdx<0 || rowIdx>=Rows)
            throw std::out_of_range("Invalid row index.");
        return data[rowIdx];
    }

    ~Matrix(){
        std::cout << "Destructor Called!" << std::endl;
        delete[] data;
    }

    template<typename F, int rowsF, int colsF>
    friend std::ostream& operator<<(std::ostream &out, const Matrix<F, rows, cols> &mat);

    class Iterator{
    private:
        Matrix<T, rows, cols> *mat;
        int row, col;

        void incrementIndex(){
            col++;
            if(col==cols)
                row++, col=0;
        }
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

        Iterator() : mat{}, row{0}, col{} {}
        Iterator(Matrix<T, rows, cols> *ptr, int r, int c): mat{ptr}, row{r}, col{c} {}

        reference operator*() const { return mat->data[row][col];}
        pointer operator->() {return &mat->data[row][col];}
        Iterator& operator++(){
            incrementIndex();
            return *this;
        }
        Iterator operator++(int){
            Iterator tmp = *this;
            incrementIndex();
            return tmp;
        }
        friend bool operator==(const Iterator &a, const Iterator &b){
            return (a.row==b.row) && (a.col==b.col);
        }
        friend bool operator!=(const Iterator &a, const Iterator &b){
            return !(a==b);
        }
    };

    Iterator begin(){
        return Iterator(this, 0, 0);
    }
    Iterator end(){
        return Iterator(this, Rows, 0);
    }
};

template<typename F, int rowsF, int colsF>
std::ostream& operator<<(std::ostream &out, const Matrix<F, rowsF, colsF> &mat){
    for(int r=0; r<rowsF; r++){
        for(int c=0; c<colsF; c++){
            out << mat[r][c] << " ";
        }
        out << std::endl;
    }
    return out;
}

template<typename T, int cols, int rows>
void print(Matrix<T, cols, rows> mat){
    std::cout << mat;
}

int main(){
    Matrix<int, 3, 4> mat({1,2,3,4,5,6,7,8,9,10,11,12});
    for(auto it=mat.begin(); it!=mat.end(); ++it){
        std::cout << *it << std::endl;
    }
    std::fill(mat.begin(), mat.end(), 55);
    for(auto x: mat){
        std::cout << x << " ";
    }
    return 0;
}

