#ifndef CHUNC4D_H
#define CHUNC4D_H

#include <cmath>
#include <vector>
#include <unordered_set>
#include <utility>
#include <algorithm>

#include "glm/vec4.hpp"
#include "glm/glm.hpp"

#include "wk4dcore.h"
#include "wk4dgraphics.h"


namespace std{

template<class T, std::size_t N>
struct hash< array<T, N>  >
{
    size_t operator () (array<T,N> const& key) const
    {
        size_t result= std::hash<T>(key[0]);
        for (int i=1; i<N; ++i){
            result <<= 1;
            result ^=  std::hash<T>(key[0]);
        }
        return result;
    }
};
}

namespace WK4d{

using std::vector;

using blocktype = unsigned char;
template <size_t S=8>
struct chunc4d{
    enum class blocks: blocktype {air = 0, solid = 1};

    glm::ivec4 offsetPos;
    float blockSize;

    array<blocktype, S*S*S*S> m_content; //[w,z,y,x]
    //
    constexpr inline
    blocktype& at(size_t wi, size_t zi, size_t yi, size_t xi)
    {
        assert(wi<S);assert(zi<S);assert(yi<S);assert(xi<S);
        return m_content.begin()[( S*S*S*wi + S*S*zi +S*yi + xi) ];
    }
    constexpr inline
    blocktype& at(glm::ivec4 coord)
    {
        return at(coord.w, coord.z, coord.y, coord.x);
    }
    constexpr inline
    blocktype const& at(size_t wi, size_t zi, size_t yi, size_t xi) const
    {
        assert(wi<S);assert(zi<S);assert(yi<S);assert(xi<S);
        return m_content.begin()[( S*S*S*wi + S*S*zi +S*yi + xi) ];
        //"whats that^ weird construction?" one might ask
        //well, thats bevause T*[] is constexpr,
        //array<T>::begin()  is constexpr,
        //whereas array<T>::operator[]() is not
        //so, that shennanigan is solely here for the sake
        //of constexpr-ness
    }
    constexpr inline
    glm::vec4 blockOriginAt(size_t wi, size_t zi, size_t yi, size_t xi) const
    {
        return startCoord()+vec4(xi, yi, zi, wi)*blockSize;
    }
    constexpr inline
    glm::vec4 startCoord() const
    {
        return glm::vec4(offsetPos.x, offsetPos.y, offsetPos.z, offsetPos.w)*blockSize;
    }
    //{left=0, rigth=1, back=2, front=3, down=4, up=5, kata=6, ana=7}
    vector<std::pair<vector<glm::vec4> , glm::vec4>> getSlice (hyperPlane4 const& plane)
    {
        //returns vector of pairs, each pair contains
        //face as vector of points and center of tesseract, slice of which resulted in face
        vector<std::pair<vector<glm::vec4> , glm::vec4>> result;
        std::unordered_set<array<blocktype, 4>> processedBlocks{};
        //possiblySlicedByPlane contains array{x,y,z,w}
        auto processBlock = [&](size_t wi, size_t zi, size_t yi, size_t xi)->void
        {
            bool alreadyProcessed = (processedBlocks.emplace(
                                         array<blocktype, 4>{wi,zi,yi,xi}).second);
            if(alreadyProcessed || wi >= S || zi >= S|| yi>=S||xi>=S){
                return;
            }
            glm::ivec4 currentBlockIdx(xi, yi, zi, wi);
            if(this->at(currentBlockIdx) == blocks::air){
                return;
            }
            array<glm::ivec4,8> allDirectionVecs = {
                {-1,0,0,0}, {1,0,0,0}, {0,-1,0,0}, {0,1,0,0},
                {0,0,-1,0}, {0,0,1,0}, {0,0,0,-1}, {0,0,0,1}
            };
            AATesseract block{blockSize};
            vec4 pos = blockOriginAt(wi, zi, yi, xi);
            block.position.x = pos.x; block.position.y = pos.y;
            block.position.z = pos.z; block.position.w = pos.w;
            vec4 blockCenter = block.position + block.size/2;
            for (int i=0; i<8; ++i){
                directions iDir{i};
                glm::ivec4 neighbourIdx = currentBlockIdx+allDirectionVecs[i];
                bool idxInsideChunc = neighbourIdx[ int(axeOfDir(iDir))] < S
                                && neighbourIdx[ int(axeOfDir(iDir))]>0;
                if(!idxInsideChunc || this->at(neighbourIdx) == blocks::air ){
                    AACube cubeCell = block.getCubeCell(iDir);
                    vector<vec4> face = cubeCrossSectionByHyperPlane(cubeCell, plane);
                    result.emplace_back({}, blockCenter);
                    for(vec4& vert :face){
                        result.back().first.emplace_back(vert.x, vert.y,
                                                         vert.z, vert.w);
                    }

                }
            }
        };
        //найдем все блоки, которые пересекают плоскость
        auto plNorm = plane.getNormal();
        array<size_t, 4> axe;
        {
            array<std::pair<char, float const*>,4> normalDirs = {
                {0, &(plNorm.x)}, {1, &(plNorm.y)},
                {2, &(plNorm.z)}, {3, &(plNorm.w)}
            };
            std::sort(normalDirs.begin(), normalDirs.end(),
                      [](std::pair<char, float const*>const& a,
                      std::pair<char, float const*>const& b){
                return (*(a.second) < *(b.second));
            });
            for (int i=0; i<4; ++i){
                axe[i] = (normalDirs[i].first);
            }
        }

        auto getCubeNthCoord=[&](array<int,4> &dimentions,
                                  int whichDim)->float{
            //(plane.A*dimentions[0]
            //+ plane.B*dimentions[1]
            //+ plane.C*dimentions[2]
            //+ plane.D*dimentions[3]
            //+ plane.E) = 0
            float result = (plane.A*dimentions[0]
                    + plane.B*dimentions[1]
                    + plane.C*dimentions[2]
                    + plane.D*dimentions[3]
                    + plane.E);
            result = -(result - plane.at(whichDim)*dimentions[whichDim]
                       )/plane.at(whichDim);
            return result;
        };
        array<int,4> dimIdx;//x y z w
        array<int,4> coordDims;//x y z w
        for (dimIdx[axe[0]]=0; dimIdx[axe[0]]<S; ++dimIdx[axe[0]] ){
            for (dimIdx[axe[1]]=0; dimIdx[axe[1]]<S; ++dimIdx[axe[1]] ){
                for (dimIdx[axe[2]]=0; dimIdx[axe[2]]<S; ++dimIdx[axe[2]] ){
                    coordDims[axe[0]] = startCoord()[axe[0]]
                                            + dimIdx[axe[0]]*blockSize;
                    coordDims[axe[1]] = startCoord()[axe[1]]
                                            + dimIdx[axe[1]]*blockSize;
                    coordDims[axe[2]] = startCoord()[axe[2]]
                                            + dimIdx[axe[2]]*blockSize;
                    coordDims[axe[3]] = getCubeNthCoord(coordDims, axe[3]);
                    //находим 4-ю координату блока через который проходит плоскость
                    //по 3м другим используя уравнение плоскости
                    dimIdx[axe[3]] = (coordDims[axe[3]]-startCoord()[axe[3]])/blockSize;
                    processBlock(dimIdx[0], dimIdx[1], dimIdx[2], dimIdx[3]);
                    dimIdx[axe[3]] += 1;
                    processBlock(dimIdx[0], dimIdx[1], dimIdx[2], dimIdx[3]);
                    dimIdx[axe[3]] -= 2;
                    processBlock(dimIdx[0], dimIdx[1], dimIdx[2], dimIdx[3]);
                }
            }
        }
        return result;
    }
};

}
#endif // CHUNC4D_H
