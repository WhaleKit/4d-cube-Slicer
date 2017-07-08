#ifndef WK4DMATRIX5_H
#define WK4DMATRIX5_H

#include"wk4dcore.h"

namespace WK4d
{
struct matrix5x5
{
    array<array<float, 5>, 5> elems; //elems[row][column]
    //matrix5x5() = default;
    matrix5x5()
        :elems{{
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0}
        }}
    {}
    matrix5x5(float f)
        :elems{{
        {f,0,0,0,0},
        {0,f,0,0,0},
        {0,0,f,0,0},
        {0,0,0,f,0},
        {0,0,0,0,f}
        }}
    {}
    matrix5x5(initializer_list<float> f0,
              initializer_list<float> f1,
              initializer_list<float> f2,
              initializer_list<float> f3,
              initializer_list<float> f4)
        :elems{{
    {f0.begin()[0], f0.begin()[1], f0.begin()[2],
          f0.begin()[3], f0.begin()[4]},
    {f1.begin()[0], f1.begin()[1], f1.begin()[2],
                f1.begin()[3], f1.begin()[4]},
    {f2.begin()[0], f2.begin()[1], f2.begin()[2],
          f2.begin()[3], f2.begin()[4]},
    {f3.begin()[0], f3.begin()[1], f3.begin()[2],
          f3.begin()[3], f3.begin()[4]},
    {f4.begin()[0], f4.begin()[1], f4.begin()[2],
          f4.begin()[3], f4.begin()[4]}
    }}
    {}
    matrix5x5(initializer_list<float> f)
        :elems{{
        {f.begin()[0], f.begin()[1], f.begin()[2],
              f.begin()[3], f.begin()[4]},
        {f.begin()[5], f.begin()[6], f.begin()[7],
              f.begin()[8], f.begin()[9]},
        {f.begin()[10], f.begin()[11], f.begin()[12],
              f.begin()[13], f.begin()[14]},
        {f.begin()[15], f.begin()[16], f.begin()[17],
              f.begin()[18], f.begin()[19]},
        {f.begin()[20], f.begin()[21], f.begin()[22],
              f.begin()[23], f.begin()[24]}
    }}
    {}
    //ways ti init:
    //    matrix5x5 m1 ={
    //        1,1,1,1,1,
    //        1,1,1,1,1,
    //        1,1,1,1,1,
    //        1,1,1,1,1,
    //        1,1,1,1,1
    //    };
    //    matrix5x5 m2 ={{{
    //        {1,1,1,1,1},
    //        {1,1,1,1,1},
    //        {1,1,1,1,1},
    //        {1,1,1,1,1},
    //        {1,1,1,1,1}
    //    }}};

    inline
    array<float, 5>& operator[] (int row)
    {
        return elems[row];
    }
    inline
    array<float, 5>const& operator[] (int row) const
    {
        return elems[row];
    }
    matrix5x5 operator * (matrix5x5 const& b) const
    {
        matrix5x5 result;
        //может потом сделаю поэлементную, если так будет быстрее
        for (int row = 0; row<5; ++row){
            for (int col =0; col<5; ++col){
                //processing result[row][col]
                result[row][col]=0;
                for (int i = 0; i<5; ++i){
                    result[row][col] += elems[row][i]*b[i][col];
                }
            }
        }
        return result;
    }
    matrix5x5& operator *= (matrix5x5 const& b)
    {
        *this = (*this)*b;
        return *this;
    }

    matrix5x5 operator +(float f)  const
    {
        matrix5x5 result = *this;
        result *= f;
        return result;
    }
    matrix5x5& operator *= (float f)
    {
        for (int row = 0; row<5; ++row){
            for (int col =0; col<5; ++col){
                elems[row][col] *= f;
            }
        }
        return *this;
    }

    matrix5x5 operator +(matrix5x5 const& b) const
    {
        matrix5x5 result = *this;
        result += b;
        return result;
    }
    matrix5x5& operator += (matrix5x5 const& b)
    {
        for (int row = 0; row<5; ++row){
            for (int col =0; col<5; ++col){
                elems[row][col] += b[row][col];
            }
        }
        return *this;
    }
};

matrix5x5 rotationMatrix(axes a1, axes a2, float angle);
matrix5x5 moveMatrix (vec4 vec);


vec4 operator *(matrix5x5 const& m, vec4 const& v);


glm::vec4 operator *(matrix5x5 const& m, glm::vec4 const& v);


//создает матрицу поворота, такую, что повернет базис i j k l
//в тот, что ей передан
//переданные вектора должны быть ортогональными, иметь длину 1
inline
matrix5x5 toNewBasisRotationMatrix(vec4 va0, vec4 va1, vec4 va2, vec4 va3)
{
    return matrix5x5{
        {va0[0], va1[0], va2[0], va3[0], 0},
        {va0[1], va1[1], va2[1], va3[1], 0},
        {va0[2], va1[2], va2[2], va3[2], 0},
        {va0[3], va1[3], va2[3], va3[3], 0},
        {     0,      0,      0,      0, 1}
    };
}
//создает матрицу поворота, такую, что повернет переданный функции
//базис в i j k l
//переданные вектора должны быть ортогональными, иметь длину 1

matrix5x5 fromBasisToStandartRotationMatrix(glm::vec4 va0, glm::vec4 va1, glm::vec4 va2, glm::vec4 va3);


}


#endif // WK4DMATRIX5_H
