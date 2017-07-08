#include "wk4dcore.h"

bool WK4d::plainIntersectsLineSegment(const WK4d::vec4 &p1, const WK4d::vec4 &p2, const WK4d::hyperPlane4 &pl)
{
    //this is the 4d-space hyperplane equation with dots 1 and 2 coordinates
    //substituted in it
    bool p1Side = pl.A*p1.x + pl.B*p1.y + pl.C*p1.z + pl.D*p1.w + pl.E >= 0;
    bool p2Side = pl.A*p2.x + pl.B*p2.y + pl.C*p2.z + pl.D*p2.w + pl.E >= 0;
    return p1Side != p2Side; //if dots at the different sides of plane, then
    //line segment connecting these points intersects plane
}

WK4d::vec4 WK4d::findIntersectionPoint(const WK4d::vec4 &p1, const WK4d::vec4 &p2, const WK4d::hyperPlane4 &pl)
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

vector<WK4d::vec4> WK4d::cubeCrossSectionByHyperPlane(const WK4d::AACube &cube, const WK4d::hyperPlane4 &plane)
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

vector<vector<WK4d::vec4> > WK4d::tesseractCrossSectionByHyperPlane(const WK4d::AATesseract &ts, const WK4d::hyperPlane4 &plane)
{
    vector<vector<vec4>> result;
    result.reserve(6);
    for (int i = 0; i<8; ++i){
        directions d = directions(i);
        auto a = cubeCrossSectionByHyperPlane( ts.getCubeCell(d), plane );
        if (!a.empty()){
            result.emplace_back(std::move(a) );
        }
    }
    return result;
}
