#include "mygeomfunctions.h"


vector<WK4dG::vec4> WK4dG::cubeCrossSectionByHyperPlane(const WK4dG::AACube &cube, const WK4dG::hyperPlane4 &plane)
{
    axes ax = ( cube.perpendicularTo != axes::x )? (axes::x) : (axes::y);
    array<vec4, 4> square = cube.getSquarePoints(dirFromAxe(ax, false));

    vector<vec4> result;
    result.reserve(4);

    //ищем пересечения граней куба с гиперплоскостью
    for (int i = 0; i<4; ++i){
        vec4& v1 = square[i];
        vec4  v2 = square[i] + cube.getDimentionVec(ax);
        if (plainIntersectsLineSegment(v1, v2, plane)){
            result.push_back(findIntersectionPoint(v1, v2, plane));
        }
    }
    for (int i = 0; i<4; ++i){
        vec4& v1 = square[i];
        vec4  v2 = square[ (i+1)%4 ];
        if (plainIntersectsLineSegment(v1, v2, plane)){
            result.push_back(findIntersectionPoint(v1, v2, plane));
        }
    }
    for (vec4& v : square){
        v += cube.getDimentionVec(ax);
    }
    for (int i = 0; i<4; ++i){
        vec4& v1 = square[i];
        vec4  v2 = square[ (i+1)%4 ];
        if (plainIntersectsLineSegment(v1, v2, plane)){
            result.push_back(findIntersectionPoint(v1, v2, plane));
        }
    }
    return result;
}

vector<vector<WK4dG::vec4> > WK4dG::tesseractCrossSectionByHyperPlane(const WK4dG::AATesseract &ts, const WK4dG::hyperPlane4 &plane)
{
    vector<vector<vec4>> result;
    result.reserve(6);
    for (int i = 0; i<=8; ++i){
        directions d = directions(i);
        auto a = cubeCrossSectionByHyperPlane( ts.getCubeCell(d), plane );
        if (!a.empty()){
            result.emplace_back(std::move(a) );
        }
    }
    return result;
}

WK4dG::vec4 WK4dG::operator *(const WK4dG::matrix5x5 &m, const WK4dG::vec4 &v)
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



bool WK4dG::plainIntersectsLineSegment(const WK4dG::vec4 &p1, const WK4dG::vec4 &p2, const WK4dG::hyperPlane4 &pl)
{
    //this is the 4d-space hyperplane equation with dots 1 and 2 coordinates
    //substituted in it
    bool p1Side = pl.A*p1.x + pl.B*p1.y + pl.C*p1.z + pl.D*p1.w + pl.E >= 0;
    bool p2Side = pl.A*p2.x + pl.B*p2.y + pl.C*p2.z + pl.D*p2.w + pl.E > 0;
    return p1Side != p2Side; //if dots at the different sides of plane, then
    //line segment connecting these points intersects plane
}

WK4dG::vec4 WK4dG::findIntersectionPoint(const WK4dG::vec4 &p1, const WK4dG::vec4 &p2, const WK4dG::hyperPlane4 &pl)
{
    const vec4 a = (p2-p1);
    const vec4 k = p1;
    vec4 result;

    float upperSum=0;
    float lowerSum=0;
    for (int i=0; i<4 ;++i){
        upperSum += pl.at(i)*k[i];
        lowerSum += pl.at(i)*a[i];
    }
    float t = -(upperSum + pl.E)/lowerSum;
    for (int i=0; i<4 ;++i){
        result[i] = k[i] + a[i]*t;
    }
    return result;
}

//WK4dG::vec4 WK4dG::findIntersectionPoint(const WK4dG::vec4 &p1, const WK4dG::vec4 &p2, const WK4dG::hyperPlane4 &pl)
//{
//    const vec4 a = p2-p1;
//    const vec4 k = p1;

//    int j = 0;
//    for(; ; ++j){
//        if (j==4){
//            throw std::invalid_argument("points shouldn't have the same coord");
//            //хотя, может оставить это на милость реализации, чтобы было как с
//            //z-fight - ами?
//        }
//        if (!fuzzyEqual(a.at(j), 0)){
//            break;
//        }
//    }
//    vec4 result;
//    //решение вывод уравнения долгий, но если кратко, то
//    //пусть плоскость задана ур-ем sum(P_i*x_i по i от 1 до n) + E = 0
//    //(здесь и далее "=" - используется как утверждение равенства)
//    //, где n - число измерений, x_i - размерность пространства (x,y,z,w)
//    //а прямая - параметрически, системой
//    //{(x_i = k_i * t + a_i) для каждой i от 1 до n}, где t - параметр
//    //где a (a_1, a_2 итд) - точка на прямой, k - направляющий вектор
//    //решением системы получаем, что тогда
//    //x_j = -( E + P_j + sum(P_i * (a_i - k_i*a_j/k_j по i от 1 до n исключая j) ))
//    //                          / ( в смысле делить, а не закомментировать)
//    //       (P_j + sum( P_i*k_i по i от 1 до n исключая j ) * k_j)
//    //где x_j - j-тая координата точки пересечения, такая, что a_j не равно 0
//    //подставив ее в уравнения прямой найдем все остальные
//    // !!! \\ !!решение выше мб неверным!! \\ !!! //


//    auto upperSumOfAllExceptOne = 0.f;
//    auto lowerSumOfAllExceptOne = 0.f;
// //    for (int i =0; i != j && i < 4; ++i){
// //        upperSumOfAllExceptOne += pl.at(i) * ( a.at(i) - k.at(i)*a.at(j) / k.at(j) );
// //        lowerSumOfAllExceptOne += pl.at(i) * k.at(i);
// //    }
//    for (int i =0;  i < 4; ++i){
//        if ( i == j){
//            continue;
//        }
//        upperSumOfAllExceptOne += pl.at(i)*( a[i]*k[j]/a[j] - k[i] );
//        lowerSumOfAllExceptOne += pl.at(i)*a[i];
//    }

// //    result.at(j) = - (pl.E + pl.at(j) + upperSumOfAllExceptOne) /
// //            (pl.at(j) + lowerSumOfAllExceptOne * k.at(j));
//    if (upperSumOfAllExceptOne!=0){
//        result.at(j) = upperSumOfAllExceptOne /
//                (  (pl.at(j) + 1/a[j]) * lowerSumOfAllExceptOne);
//    }else{
//        result.at(j)=0;
//    }
//    //мы нашли j-тую координату точки пересечения, осталось теепрь найти
//    //остальные координаты из уравнения прямой
//    auto t = ( result.at(j) - k.at(j) ) / a.at(j) ;
//    for (int i = 0; i != j && i < 4; ++i){
//        result.at(i) = a.at(i) * t + k.at(i);
//    }
//    return result;
//}


WK4dG::matrix5x5 WK4dG::fromBasisToStandartRotationMatrix(WK4dG::vec4 va0, WK4dG::vec4 va1, WK4dG::vec4 va2, WK4dG::vec4 va3)
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

WK4dG::matrix5x5 WK4dG::rotationMatrix(WK4dG::axes a1, WK4dG::axes a2, float angle)
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

WK4dG::matrix5x5 WK4dG::moveMatrix(WK4dG::vec4 vec){ //aka translation matrix
    matrix5x5 result(1);
    for (int i=0; i<4; ++i){
        result[i][4] = vec[i];
    }
    return result;
}

constexpr WK4dG::vec4 WK4dG::FPSPointOfView::myUp;
