#ifndef WK4DCORE_H
#define WK4DCORE_H

#include <cmath>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <array>
#include <vector>
#include <initializer_list>

#include "glm/vec4.hpp"
#include "glm/glm.hpp"

using std::array;
using std::vector;
using std::initializer_list;
using std::numeric_limits;
using std::size_t;

//by the way,
//why the hell std::array::operator[] () is not constexpr?
//problaly an oversight
//it's probably already fixed in c++17

namespace WK4d
{
//the most basic stuff
constexpr bool fuzzyEqual (float f1, float f2)
{
    auto epsi = std::numeric_limits<float>::epsilon()*8;//juust in case u now
    return fabs(f2-f1) < epsi;
}
const constexpr double mathPi = acos(-1);

enum class axes {x=0,y=1,z=2,w=3};
enum class directions {left=0, rigth=1, back=2, front=3, down=4, up=5, kata=6, ana=7};
//ana +w, kata -w
constexpr inline
axes axeOfDir(directions dir){
    return axes(static_cast<int>(dir)/2);
}
constexpr inline
bool dirCodirectedWAxis(directions dir){
    return static_cast<int>(dir)%2 == 1;
}
constexpr inline
directions dirFromAxe(axes axe, bool codirected= true){
    return directions(static_cast<int>(axe)*2 + codirected);
}

inline
glm::vec4 unitVec(axes axe)
{
    switch (axe){
    case axes::x: return glm::vec4{1,0,0,0};
    case axes::y: return glm::vec4{0,1,0,0};
    case axes::z: return glm::vec4{0,0,1,0};
    case axes::w: return glm::vec4{0,0,0,1};
    }
    if (numeric_limits<float>::has_signaling_NaN){
        return glm::vec4{
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN()
        };
    }else{
        return glm::vec4{std::nanf("BadIdx"), std::nanf("BadIdx"),
                    std::nanf("BadIdx"), std::nanf("BadIdx")};
    }
}

inline
glm::ivec4 unitIvec(axes axe)
{
    switch (axe){
    case axes::x: return glm::ivec4{1,0,0,0};
    case axes::y: return glm::ivec4{0,1,0,0};
    case axes::z: return glm::ivec4{0,0,1,0};
    case axes::w: return glm::ivec4{0,0,0,1};
    }
    throw std::invalid_argument("there's only 4 axes, idiot!");
}


//vectors

inline
bool isZeroVec(glm::vec4 const& v)
{
    for(int i=0; i<4; ++i){
        if ( !fuzzyEqual(v[i], 0) ){
            return false;
        }
    }
    return true;
}
inline glm::vec4 vecRotatedTowardAnother(glm::vec4 const& toRot, glm::vec4 const& tow, float angle)
{
    return toRot*std::cos(angle) +
            tow*std::sin(angle);
}


//why in the name of god aren't you constexpr, glm::vec4??????????????
//why
//WHYYYYYYYYYYYYYYYYY
//why do you have to ruin everything???
using vec4 = glm::vec4;


inline vec4 orthogonalToThree(vec4 const& v1, vec4 const& v2, vec4 const& v3)
{
    //4d vector orthogonal to 3 another 3dvecs equals determinant of
    // | e1 e2 e3 e4 |
    // | x1 y1 z1 w1 |
    // | x2 y2 z2 w2 |
    // | x3 y3 z3 w3 |
    // | x4 y4 z4 w4 |
    //где e1-4 - orthogonal unit vectors, x1-4 x-coords of vecs
    //y1-4 y-coords of vecs ect.
    //solution found with sympy sympy with following program:
    //    from sympy.matrices import *
    //    dims = ['x', 'y', 'z', 'w']
    //    vecs = ['v1.', 'v2.', 'v3.']
    //    rows = []
    //    rows.append([symbols(i) for i in dims])
    //    for j in vecs:
    //        rows.append([symbols(j+i) for i in dims])
    //    m = Matrix(rows)
    //    x, y, z, w = symbols('x y z w')
    //    collect(m.det(), x).coeff(x) # outputs x-coords of result vector
    //    collect(m.det(), y).coeff(y) # outputs y-coords of result vector
    //    collect(m.det(), z).coeff(z) # outputs z-coords of result vector
    //    collect(m.det(), w).coeff(w) # you know the drill
    return vec4{
        v1.w*v2.y*v3.z - v1.w*v2.z*v3.y - v1.y*v2.w*v3.z +
                v1.y*v2.z*v3.w + v1.z*v2.w*v3.y - v1.z*v2.y*v3.w
        ,-v1.w*v2.x*v3.z + v1.w*v2.z*v3.x + v1.x*v2.w*v3.z -
                v1.x*v2.z*v3.w - v1.z*v2.w*v3.x + v1.z*v2.x*v3.w
        ,v1.w*v2.x*v3.y - v1.w*v2.y*v3.x - v1.x*v2.w*v3.y +
                v1.x*v2.y*v3.w + v1.y*v2.w*v3.x - v1.y*v2.x*v3.w
        ,-v1.x*v2.y*v3.z + v1.x*v2.z*v3.y + v1.y*v2.x*v3.z -
                v1.y*v2.z*v3.x - v1.z*v2.x*v3.y + v1.z*v2.y*v3.x
    };
}

struct hyperPlane4
{
    //A*x + B*y + C*z + D*w + E
    constexpr hyperPlane4(hyperPlane4 const&) = default;
    hyperPlane4(vec4 p0, vec4 p1, vec4 p2, vec4 p3 )
        : hyperPlane4(p0, orthogonalToThree(p1-p0, p2-p0, p3-p0))
    {
        //makes plane containing all the dots
    }
    hyperPlane4(vec4 point, vec4 normalVec) :
        A(normalVec.x), B(normalVec.y),
        C(normalVec.z), D(normalVec.w),
        E(0)
    {
        moveToContainPoint(point);
        //make plane containing point p perpendicular to normalVec
    }
    inline
    void moveToContainPoint(vec4 point)
    {
        //constexpr-функция, которая меняет объект
        //(чтобы моэно было использовать в не меняюших
        //внешнее состояние constexp-функциях)
        //убедитесь, что собираете это компилятором, поддерживающим c++14
        E = -(A*point.x + B*point.y + C*point.z + D*point.w);
    }
    inline
    vec4 getNormal() const
    {
        return vec4{A,B,C,D};
    }
    inline
    void setNormal(vec4 newNormal)
    {
        A = newNormal.x; B = newNormal.y;
        C = newNormal.z; D = newNormal.w;
    }
    inline
    float& at(int i)
    {
        assert(i>=0 && i<5);
        switch(i){
        case 0: return A;
        case 1: return B;
        case 2: return C;
        case 3: return D;
        case 4: return E;
        }
    }
    inline
    float const& at(int i) const
    {
        assert(i>=0 && i<5);
        switch(i){
        case 0: return A;
        case 1: return B;
        case 2: return C;
        case 3: return D;
        case 4: return E;
        }
    }

    float A, B, C, D, E;
};

bool plainIntersectsLineSegment(vec4 const&p1, vec4 const&p2, hyperPlane4 const& pl);

vec4 findIntersectionPoint (vec4 const& p1, vec4 const& p2, hyperPlane4 const& pl);

//it's just a axis-aligned cube
//but in the 4d space
struct AACube
{
    axes perpendicularTo = axes::w;
    array<float, 4> dimentions;
    vec4 position; //..of left back low kata corner

    //if it perpendicular to z, dimentions is {x y w}
    //if it perpendicular to w, dimentions is {x y z}
    constexpr
    AACube(axes ax = axes::w, float a=1, float b=1, float c=1, vec4 pos = vec4{0,0,0,0})
        :perpendicularTo(ax), dimentions({a,b,c}), position(pos)
    {}
    AACube(vec4 dims, vec4 pos)
        :dimentions({dims[0],dims[1],dims[2],dims[3]}),
          position(pos)
    {
        int i=0;
        for (; i<4; ++i){
            if ( fuzzyEqual(dims[i], 0) ){
                perpendicularTo = axes(i);
                dimentions[i] = 0;
                break;
            }
        }
    }
    inline
    vec4 getDimentionVec(axes ax) const
    {
        return unitVec(ax)*dimentions[static_cast<int>(ax)];
    }

    array<vec4, 4> getSquarePoints(directions dir) const
    {
        assert(axeOfDir(dir) == perpendicularTo);
        axes ax = axeOfDir(dir); //нас интерсует квадрат
        array<axes, 2> dims; //направления, перпендикулярные dir и perpendicularTo
        for (int i = 0, j=0; i<4; ++i){
            if (axes(i) != ax && axes(i) != perpendicularTo){
                dims[j] = axes(i);
                ++j;
            }
        }

        array<vec4, 4> result;
        result[0] = position;
        result[1] = position + getDimentionVec(dims[0]);
        result[2] = position + getDimentionVec(dims[0])
                             + getDimentionVec(dims[1]);
        result[3] = position + getDimentionVec(dims[1]);
        if (dirCodirectedWAxis(dir)){
            for (int i =0; i<4; ++i){
                result[i] += getDimentionVec(ax);
            }
        }
        return result;
    }

};

//Axis-aligned tesseract
struct AATesseract
{
    vec4 size;
    vec4 position; //of left back low kata corner


    AATesseract (float xa, float ya, float za, float wa)
        :size(xa, ya, za, wa), position(0,0,0,0)
    {}
    explicit
    AATesseract (float a)
        :size(a,a,a,a)
    {}
    inline
    vec4 getDimentionVec(axes ax) const
    {
        return unitVec(ax)*size[static_cast<int>(ax)];
    }
    AACube getCubeCell (directions dir) const
    {
        AACube result(size*(vec4{1,1,1,1} - unitVec(axeOfDir(dir))), position);
        if (dirCodirectedWAxis(dir)){
            result.position += getDimentionVec(axeOfDir(dir));
        }
        return result;
    }
};


vector<vec4> cubeCrossSectionByHyperPlane (AACube const& cube, hyperPlane4 const& plane);

vector<vector<vec4>> tesseractCrossSectionByHyperPlane (AATesseract const& ts,
                                                        hyperPlane4 const& plane);

}

#endif // WK4DCORE_H
