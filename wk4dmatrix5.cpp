#include "wk4dmatrix5.h"


WK4d::matrix5x5 WK4d::rotationMatrix(WK4d::axes a1, WK4d::axes a2, float angle)
{
    matrix5x5 result(1);
    if (a1 == a2){
        return result;
    }
    int a1i = static_cast<int>(a1);
    int a2i = static_cast<int>(a2);
    //столбцы и строки матрицы связаны с координатами
    result[a1i][a1i] = std::cos(angle);
    result[a1i][a2i] = std::sin(angle);
    result[a2i][a1i] = -std::sin(angle);
    result[a2i][a2i] = std::cos(angle);
    //не могу сказать, что полностью интуитивно понимаю почему
    //но аналогия с 3д матрицами видна отчетливо
    return result;
}

WK4d::matrix5x5 WK4d::moveMatrix(WK4d::vec4 vec){ //aka translation matrix
    matrix5x5 result(1);
    for (int i=0; i<4; ++i){
        result[i][4] = vec[i];
    }
    return result;
}
WK4d::vec4 WK4d::operator *(const WK4d::matrix5x5 &m, const WK4d::vec4 &v)
{
    array<float, 5> res{};
    for (int row = 0; row<5; ++row){
        for (int i=0; i<4; ++i){
            res.begin()[row] += m[row][i]*v[i];
        }
        res.begin()[row] += m[row][4];
    }
    return vec4{res.begin()[0],res.begin()[1],res.begin()[2],res.begin()[3]};
}

WK4d::matrix5x5 WK4d::fromBasisToStandartRotationMatrix(WK4d::vec4 va0, WK4d::vec4 va1, WK4d::vec4 va2, WK4d::vec4 va3)
{
    matrix5x5 result;
    result[5][5]=1;
    float det =
            va0[0]*va1[1]*va2[2]*va3[3] - va0[0]*va1[1]*va2[3]*va3[2] - va0[0]*va1[2]*va2[1]*va3[3] +
            va0[0]*va1[2]*va2[3]*va3[1] + va0[0]*va1[3]*va2[1]*va3[2] - va0[0]*va1[3]*va2[2]*va3[1] -
            va0[1]*va1[0]*va2[2]*va3[3] + va0[1]*va1[0]*va2[3]*va3[2] + va0[1]*va1[2]*va2[0]*va3[3] -
            va0[1]*va1[2]*va2[3]*va3[0] - va0[1]*va1[3]*va2[0]*va3[2] + va0[1]*va1[3]*va2[2]*va3[0] +
            va0[2]*va1[0]*va2[1]*va3[3] - va0[2]*va1[0]*va2[3]*va3[1] - va0[2]*va1[1]*va2[0]*va3[3] +
            va0[2]*va1[1]*va2[3]*va3[0] + va0[2]*va1[3]*va2[0]*va3[1] - va0[2]*va1[3]*va2[1]*va3[0] -
            va0[3]*va1[0]*va2[1]*va3[2] + va0[3]*va1[0]*va2[2]*va3[1] + va0[3]*va1[1]*va2[0]*va3[2] -
            va0[3]*va1[1]*va2[2]*va3[0] - va0[3]*va1[2]*va2[0]*va3[1] + va0[3]*va1[2]*va2[1]*va3[0];
    result[0][0] = ( va1[1]*va2[2]*va3[3] - va1[1]*va2[3]*va3[2] - va1[2]*va2[1]*va3[3] +
       va1[2]*va2[3]*va3[1] + va1[3]*va2[1]*va3[2] - va1[3]*va2[2]*va3[1] )/det;
    result[0][1] = ( -va1[0]*va2[2]*va3[3] + va1[0]*va2[3]*va3[2] + va1[2]*va2[0]*va3[3] -
       va1[2]*va2[3]*va3[0] - va1[3]*va2[0]*va3[2] + va1[3]*va2[2]*va3[0] )/det;
    result[0][2] = ( va1[0]*va2[1]*va3[3] - va1[0]*va2[3]*va3[1] - va1[1]*va2[0]*va3[3] +
       va1[1]*va2[3]*va3[0] + va1[3]*va2[0]*va3[1] - va1[3]*va2[1]*va3[0] )/det;
    result[0][3] = ( -va1[0]*va2[1]*va3[2] + va1[0]*va2[2]*va3[1] + va1[1]*va2[0]*va3[2] -
       va1[1]*va2[2]*va3[0] - va1[2]*va2[0]*va3[1] + va1[2]*va2[1]*va3[0] )/det;
    result[1][0] = ( -va0[1]*va2[2]*va3[3] + va0[1]*va2[3]*va3[2] + va0[2]*va2[1]*va3[3] -
       va0[2]*va2[3]*va3[1] - va0[3]*va2[1]*va3[2] + va0[3]*va2[2]*va3[1] )/det;
    result[1][1] = ( va0[0]*va2[2]*va3[3] - va0[0]*va2[3]*va3[2] - va0[2]*va2[0]*va3[3] +
       va0[2]*va2[3]*va3[0] + va0[3]*va2[0]*va3[2] - va0[3]*va2[2]*va3[0] )/det;
    result[1][2] = ( -va0[0]*va2[1]*va3[3] + va0[0]*va2[3]*va3[1] + va0[1]*va2[0]*va3[3] -
       va0[1]*va2[3]*va3[0] - va0[3]*va2[0]*va3[1] + va0[3]*va2[1]*va3[0] )/det;
    result[1][3] = ( va0[0]*va2[1]*va3[2] - va0[0]*va2[2]*va3[1] - va0[1]*va2[0]*va3[2] +
       va0[1]*va2[2]*va3[0] + va0[2]*va2[0]*va3[1] - va0[2]*va2[1]*va3[0] )/det;
    result[2][0] = ( va0[1]*va1[2]*va3[3] - va0[1]*va1[3]*va3[2] - va0[2]*va1[1]*va3[3] +
       va0[2]*va1[3]*va3[1] + va0[3]*va1[1]*va3[2] - va0[3]*va1[2]*va3[1] )/det;
    result[2][1] = ( -va0[0]*va1[2]*va3[3] + va0[0]*va1[3]*va3[2] + va0[2]*va1[0]*va3[3] -
       va0[2]*va1[3]*va3[0] - va0[3]*va1[0]*va3[2] + va0[3]*va1[2]*va3[0] )/det;
    result[2][2] = ( va0[0]*va1[1]*va3[3] - va0[0]*va1[3]*va3[1] - va0[1]*va1[0]*va3[3] +
       va0[1]*va1[3]*va3[0] + va0[3]*va1[0]*va3[1] - va0[3]*va1[1]*va3[0] )/det;
    result[2][3] = ( -va0[0]*va1[1]*va3[2] + va0[0]*va1[2]*va3[1] + va0[1]*va1[0]*va3[2] -
       va0[1]*va1[2]*va3[0] - va0[2]*va1[0]*va3[1] + va0[2]*va1[1]*va3[0] )/det;
    result[3][0] = ( -va0[1]*va1[2]*va2[3] + va0[1]*va1[3]*va2[2] + va0[2]*va1[1]*va2[3] -
       va0[2]*va1[3]*va2[1] - va0[3]*va1[1]*va2[2] + va0[3]*va1[2]*va2[1] )/det;
    result[3][1] = ( va0[0]*va1[2]*va2[3] - va0[0]*va1[3]*va2[2] - va0[2]*va1[0]*va2[3] +
       va0[2]*va1[3]*va2[0] + va0[3]*va1[0]*va2[2] - va0[3]*va1[2]*va2[0] )/det;
    result[3][2] = ( -va0[0]*va1[1]*va2[3] + va0[0]*va1[3]*va2[1] + va0[1]*va1[0]*va2[3] -
       va0[1]*va1[3]*va2[0] - va0[3]*va1[0]*va2[1] + va0[3]*va1[1]*va2[0] )/det;
    result[3][3] = ( va0[0]*va1[1]*va2[2] - va0[0]*va1[2]*va2[1] - va0[1]*va1[0]*va2[2] +
       va0[1]*va1[2]*va2[0] + va0[2]*va1[0]*va2[1] - va0[2]*va1[1]*va2[0] )/det;
    return result;
    //я не китаец! это все кодогенерация, честно!
}
