#ifndef MYGEOMFUNCTIONS_H
#define MYGEOMFUNCTIONS_H

#include <cmath>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <array>
#include <vector>
#include <initializer_list>

using std::array;
using std::vector;
using std::initializer_list;
using std::numeric_limits;
using std::size_t;

//by the way,
//why the hell std::array::operator[] () is not constexpr
//problaly an oversight
//it's probably already fixed in c++17


//4th spatial dimentiom is called "w" in order to avoid confusion with time (t)
namespace WK4dG
{
//stands for WhaleKit's 4d geometry
//though it doesn't matter since i'm gonna shamelessly
//use "using namespace WK4dG" all around the "project"
constexpr bool fuzzyEqual (float f1, float f2)
{
    auto epsi = std::numeric_limits<float>::epsilon()*8;//juust in case u now
    return fabs(f2-f1) < epsi;
}


const constexpr double mathPi = acos(-1);

struct vec4
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 0;
    constexpr vec4() = default;
    constexpr vec4(vec4 const&) = default;
    constexpr vec4(float xa, float ya, float za, float wa) :
        x(xa), y(ya), z(za), w(wa)
    {};
    float& at(int i)
    {
        switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default:
            throw std::out_of_range("vec4 has only indexes in range [0, 4)");
        }
    }
    float const& at(int i) const
    {
        switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default:
            throw std::out_of_range("vec4 has only indexes in range [0, 4)");
        }
    }
    constexpr
    float& operator[](int i)
    {
        switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        default: return *((float*)(nullptr));
        }
    }
    constexpr
    float operator[](int i) const
    {
        switch(i){
        case 0: return x;
        case 1: return y;
        case 2: return z;
        case 3: return w;
        }
        if (numeric_limits<float>::has_signaling_NaN){
            return numeric_limits<float>::signaling_NaN();
        }else{
            return std::nanf("BadIdx");
        }
        //too bad assert(false); throw isn't allowed in constexpr function.
    }
    constexpr
    bool isZeroVec()
    {
        for(int i=0; i<4; ++i){
            if ( !fuzzyEqual((*this)[i], 0) ){
                return false;
            }
        }
        return true;
    }
    constexpr
    float mod() const
    {
        return std::sqrt(x*x+y*y+z*z+w*w);
    }
    constexpr
    vec4 normazized() const&
    {
        return (*this)/mod();
    }
    constexpr
    vec4&& normazized() &&
    {
        this->normazize();
        return std::move(*this);
    }
    constexpr
    void normazize()
    {
        (*this)/=mod();
    }
    constexpr
    vec4& operator+=(vec4 const& oth)
    {
        x+=oth.x;
        y+=oth.y;
        z+=oth.z;
        w+=oth.w;
        return *this;
    }
    constexpr
    vec4& operator-=(vec4 const& oth)
    {
        x-=oth.x;
        y-=oth.y;
        z-=oth.z;
        w-=oth.w;
        return *this;
    }
    constexpr vec4 operator+(vec4 const& oth)  const
    {
        return vec4{x + oth.x, y + oth.y,
                    z + oth.z, w + oth.w};
    }

    constexpr vec4 operator-(vec4 const& oth)  const
    {
        return vec4{x - oth.x, y - oth.y,
                    z - oth.z, w - oth.w};
    }
    constexpr vec4 operator*(vec4 const& oth)  const
    {
        return vec4{x*oth.x, y*oth.y, z*oth.z, w*oth.w};
    }
    constexpr
    vec4& operator*=(vec4 const& oth)
    {
        x*=oth.x;
        y*=oth.y;
        z*=oth.z;
        w*=oth.w;
        return *this;
    }
    constexpr vec4 operator*(float f)  const
    {
        return vec4{x*f, y*f, z*f, w*f};
    }
    constexpr
    vec4& operator/=(float f)
    {
        (*this)*= (1/f);
        return *this;
    }
    constexpr vec4 operator/(float f)  const
    {
        return (*this)*(1/f);
    }
    constexpr
    vec4& operator*=(float f)
    {
        x*=f; y*=f;
        z*=f; w*=f;
        return *this;
    }
    constexpr vec4 operator-() const
    {
        return vec4{-x,-y,-z,-w};
    }
};

constexpr vec4 orthogonalToThree(vec4 const& v1, vec4 const& v2, vec4 const& v3)
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
    constexpr hyperPlane4(vec4 p0, vec4 p1, vec4 p2, vec4 p3 )
        : hyperPlane4(p0, orthogonalToThree(p1-p0, p2-p0, p3-p0))
    {
        //makes plane containing all the dots
    }
    constexpr hyperPlane4(vec4 point, vec4 normalVec) :
        A(normalVec.x), B(normalVec.y),
        C(normalVec.z), D(normalVec.w),
        E(0)
    {
        moveToContainPoint(point);
        //make plane containing point p perpendicular to normalVec
    }
    constexpr void moveToContainPoint(vec4 point)
    {
        //constexpr-функция, которая меняет объект
        //(чтобы моэно было использовать в не меняюших
        //внешнее состояние constexp-функциях)
        //убедитесь, что собираете это компилятором, поддерживающим c++14
        E = -(A*point.x + B*point.y + C*point.z + D*point.w);
    }
    vec4 getNormal() const
    {
        return vec4{A,B,C,D};
    }
    void setNormal(vec4 newNormal)
    {
        A = newNormal.x; B = newNormal.y;
        C = newNormal.z; D = newNormal.w;
    }

    float& at(int i)
    {
        switch(i){
        case 0: return A;
        case 1: return B;
        case 2: return C;
        case 3: return D;
        case 4: return E;
        default:
            throw std::out_of_range("hyperPlane4 has only indexes in range [0, 5)");
        }
    }
    float const& at(int i) const
    {
        switch(i){
        case 0: return A;
        case 1: return B;
        case 2: return C;
        case 3: return D;
        case 4: return E;
        default:
            throw std::out_of_range("hyperPlane4 has only indexes in range [0, 5)");
        }
    }

    float A, B, C, D, E;
};

//template <typename range_T>
//decltype( *std::declval<range_T>() ) sumOfAllExceptOne (range_T r, int exceptNum)
//{
//    auto iter = std::begin(r);
//    auto endIt = std::end(r);
//    decltype(*iter) accumulate = 0;
//    for (int i =0; (i != exceptNum) && (iter != endIt); ++i, ++iter ){
//        accumulate += *iter;
//    }
//    return accumulate;
//}

bool plainIntersectsLineSegment(vec4 const&p1, vec4 const&p2, hyperPlane4 const& pl);

vec4 findIntersectionPoint (vec4 const& p1, vec4 const& p2, hyperPlane4 const& pl);


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
constexpr
vec4 unitVec(axes axe)
{
    switch (axe){
    case axes::x: return vec4{1,0,0,0};
    case axes::y: return vec4{0,1,0,0};
    case axes::z: return vec4{0,0,1,0};
    case axes::w: return vec4{0,0,0,1};
    }
    if (numeric_limits<float>::has_signaling_NaN){
        return vec4{
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN(),
            numeric_limits<float>::signaling_NaN()
        };
    }else{
        return vec4{std::nanf("BadIdx"), std::nanf("BadIdx"),
                    std::nanf("BadIdx"), std::nanf("BadIdx")};
    }
}

//its just a axis-aligned cube
//but it is in the 4d space
//
struct AACube
{
    axes perpendicularTo = axes::w;
    array<float, 4> dimentions;
    vec4 position; //of left back low kata corner
    //if it perpendicular to z, dimentions is {x y w}
    //if it perpendicular to w, dimentions is {x y z}
    constexpr
    AACube(axes ax = axes::w, float a=1, float b=1, float c=1, vec4 pos = vec4{0,0,0,0})
        :perpendicularTo(ax), dimentions({a,b,c}), position(pos)
    {}
    constexpr
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
    inline constexpr
    vec4 getDimentionVec(axes ax) const
    {
        return unitVec(ax)*dimentions[static_cast<int>(ax)];
    }

    array<vec4, 4> getSquarePoints(directions dir) const
    {
        if (axeOfDir(dir) == perpendicularTo){
            throw std::invalid_argument("this cube dont have square on that side");
        }
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

    constexpr
    AATesseract (float xa, float ya, float za, float wa)
        :size(xa, ya, za, wa), position(0,0,0,0)
    {}
    explicit constexpr
    AATesseract (float a)
        :size(a,a,a,a)
    {}
    inline constexpr
    vec4 getDimentionVec(axes ax) const
    {
        return unitVec(ax)*size[static_cast<int>(ax)];
    }
    constexpr
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

//it's a shame there is no 5x5 matrices in glm
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



//создает матрицу поворота в плоскости осей a1 и a2
matrix5x5 rotationMatrix(axes a1, axes a2, float angle);
matrix5x5 moveMatrix (vec4 vec);


vec4 operator *(matrix5x5 const& m, vec4 const& v);


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

matrix5x5 fromBasisToStandartRotationMatrix(vec4 va0, vec4 va1, vec4 va2, vec4 va3);


struct FPSPointOfView
{
    hyperPlane4 *planeImOn;
    vec4 myCoord;
    static constexpr vec4 myUp = {0,0,1,0};
    vec4 myFront=vec4{1,0,0,0};
    float myPitch; //agnle in radians
    inline
    vec4 getMyRigth() const
    {
        return orthogonalToThree(planeImOn->getNormal(),myUp, myFront).normazized();
    }
    inline
    vec4 myViewDirection() const
    {
        return std::cos(myPitch)*myFront + std::sin(myPitch)*myUp;
    }

    void rotateForwardUp(float f)
    {
        //differs from othera due to the fact, that it's not a spasesim:
        //this one ratetion is gimgle-lockable
        myPitch += f;
        myPitch = std::min(myPitch, static_cast<float>( mathPi/2) );
        myPitch = std::max(myPitch, static_cast<float>(-mathPi/2) );
    }
    void rotateForwardRight(float ang)
    {
        vec4 newF;
        newF = std::cos(ang)*myFront +
                std::sin(ang)*getMyRigth();
        myFront = newF;
    }
    //2 following functions return new plane normal vector
    //il concider changing hyperplane's normal directly instead
    vec4 rotateRightAna (float ang)
    {
        vec4 newA;
        newA = std::sin(ang)*planeImOn->getNormal() +
                (-std::cos(ang)*getMyRigth());
        return newA;
    }
    vec4 rotateForwardAna(float ang)
    {
        vec4 newF;
        vec4 newA;
        newF = std::cos(ang)*myFront +
                std::sin(ang)*getMyRigth();
        newA = std::sin(ang)*planeImOn->getNormal() +
                (-std::cos(ang)*myFront);
        myFront = newF;
        return newA;
    }

    matrix5x5 getWorldToHyperplaneLocalTransformMatrix() const
    {
        //матрица. смещаем. поворачиваем для нового базиса.
        //поворачиваем для pitch
        matrix5x5 transform = moveMatrix(-myCoord);
        transform = toNewBasisRotationMatrix(myFront, getMyRigth(), myUp
                        ,planeImOn->getNormal().normazized())*transform;
        return transform;
    }
    matrix5x5 getWorldToViewTransformMatrix() const
    {
        matrix5x5 transform = moveMatrix(-myCoord);
        transform = toNewBasisRotationMatrix(myFront, getMyRigth(), myUp
                        ,planeImOn->getNormal().normazized())*transform;
        transform = rotationMatrix(axes::x, axes::z, myPitch) * transform;
        return transform;
    }
    matrix5x5 getHyperplaneLocalToWorldTransformMatrix() const
    {
        matrix5x5 transform = fromBasisToStandartRotationMatrix(myFront, getMyRigth(), myUp
                                                ,planeImOn->getNormal().normazized());
        transform = moveMatrix(myCoord)*transform;
        return transform;
    }


    void normalize()
    {
        myCoord.w = (planeImOn->A*myCoord.x+planeImOn->B*myCoord.y +
                     planeImOn->C*myCoord.z + planeImOn->E) / planeImOn->D;
        myFront.w = (planeImOn->A*(myCoord.x+myFront.x) +
                     planeImOn->B*(myCoord.y+myFront.y) +
                     planeImOn->C*(myCoord.z+myFront.z) +
                     + planeImOn->E) / planeImOn->D - myCoord.w;
        myFront.normazize();
    }

};





}

#endif // MYGEOMFUNCTIONS_H
