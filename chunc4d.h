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
        size_t result= std::hash<T>()(key[0]);
        for (int i=1; i<N; ++i){
            result <<= 1;
            result ^=  std::hash<T>()(key[i]);
        }
        return result;
    }
};
template<>
struct hash< glm::ivec4  >
{
    size_t operator () (glm::ivec4 const& key) const
    {
        size_t result= std::hash<glm::ivec4::value_type>()(key[0]);
        for (int i=1; i<4; ++i){
            result <<= 1;
            result ^=  std::hash<glm::ivec4::value_type>()(key[i]);
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

    size_t aSize = S;
    array<blocks, S*S*S*S> m_content; //[w,z,y,x]
    //
    constexpr inline
    blocks& at(size_t wi, size_t zi, size_t yi, size_t xi)
    {
        assert(wi<S);assert(zi<S);assert(yi<S);assert(xi<S);
        return m_content.begin()[( S*S*S*wi + S*S*zi +S*yi + xi) ];
    }
    constexpr inline
    blocks& at(glm::ivec4 coord)
    {
        return at(coord.w, coord.z, coord.y, coord.x);
    }
    constexpr inline
    blocks const& at(size_t wi, size_t zi, size_t yi, size_t xi) const
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
    glm::vec4 blockOriginAt(glm::ivec4 coordIdx) const
    {
        return startCoord()+vec4(coordIdx)*blockSize;
    }
    constexpr inline
    glm::vec4 startCoord() const
    {
        return glm::vec4(offsetPos.x, offsetPos.y, offsetPos.z, offsetPos.w)*blockSize;
    }
    constexpr inline
    glm::ivec4 coordToIndex(glm::vec4 & coord)
    {
         glm::vec4 res = glm::vec4((coord-startCoord())/blockSize);
         return glm::floor(res);
    }
    constexpr inline
    int coordToIndex(float coord, axes ax)
    {
         return std::floor( (coord-startCoord()[static_cast<size_t>(ax)])
                           /blockSize );
    }

    //{left=0, rigth=1, back=2, front=3, down=4, up=5, kata=6, ana=7}
    vector<std::pair<vector<glm::vec4> , glm::vec4>> getSlice (hyperPlane4 const& plane)
    {
        //returns vector of pairs, each pair contains
        //face as vector of points and center of tesseract, slice of which resulted in face
        vector<std::pair<vector<glm::vec4> , glm::vec4>> result;
        std::unordered_set<glm::ivec4> processedBlocks{};
        //possiblySlicedByPlane contains array{x,y,z,w}
        auto processBlock = [&](glm::ivec4 coordIdx)->void
        {
            if(    coordIdx.w >= S ||coordIdx.w < 0
                || coordIdx.z >= S ||coordIdx.z < 0
                || coordIdx.y >= S ||coordIdx.y < 0
                || coordIdx.x >= S ||coordIdx.x < 0 ){
                return;
            }

            bool alreadyProcessed = !(processedBlocks.emplace(
                                        coordIdx).second);
            if(alreadyProcessed || this->at(coordIdx) == blocks::air){
                return;
            }
            array<glm::ivec4,8> allDirectionVecs = {{
                {-1,0,0,0}, {1,0,0,0}, {0,-1,0,0}, {0,1,0,0},
                {0,0,-1,0}, {0,0,1,0}, {0,0,0,-1}, {0,0,0,1}
            }};
            AATesseract block{blockSize};
            vec4 pos = blockOriginAt(coordIdx);
            block.position.x = pos.x; block.position.y = pos.y;
            block.position.z = pos.z; block.position.w = pos.w;
            vec4 blockCenter = block.position + block.size/2.f;
            for (int i=0; i<8; ++i){
                directions iDir = static_cast<directions>(i);
                glm::ivec4 neighbourIdx = coordIdx+allDirectionVecs[i];
                bool idxInsideChunc = neighbourIdx[ int(axeOfDir(iDir))] < S
                                && neighbourIdx[ int(axeOfDir(iDir))]>=0;
                if(!idxInsideChunc || this->at(neighbourIdx) == blocks::air ){
                    AACube cubeCell = block.getCubeCell(iDir);
                    vector<vec4> face = cubeCrossSectionByHyperPlane(cubeCell, plane);
                    if (!face.empty()){
                        result.emplace_back(vector<vec4>{}, blockCenter);
                        for(vec4& vert :face){
                            result.back().first.emplace_back(vert);
                        }
                    }
                }
            }
        };
        //найдем все блоки, которые пересекают плоскость
        auto plNorm = plane.getNormal();
        array<size_t, 4> axe;
        {
            array<std::pair<char, float const*>,4> normalDirs = {{
                {0, &(plNorm.x)}, {1, &(plNorm.y)},
                {2, &(plNorm.z)}, {3, &(plNorm.w)}
            }};
            std::sort(normalDirs.begin(), normalDirs.end(),
                      [](std::pair<char, float const*>const& a,
                      std::pair<char, float const*>const& b){
                return (std::fabs(*(a.second))
                         < std::fabs(*(b.second)));
            });
            for (int i=0; i<4; ++i){
                axe[i] = (normalDirs[i].first);
            }
        }

        //находим n-ю координату блока через который проходит плоскость
        //по 3м другим используя уравнение плоскости
        auto getCubeNthCoord=[&](glm::vec4 const& worldCoord,
                                  int whichDim)->float{
            //(plane.A*dimentions[0]
            //+ plane.B*dimentions[1]
            //+ plane.C*dimentions[2]
            //+ plane.D*dimentions[3]
            //+ plane.E) = 0
            float result = (plane.A*worldCoord[0]
                    + plane.B*worldCoord[1]
                    + plane.C*worldCoord[2]
                    + plane.D*worldCoord[3]
                    + plane.E);
            result = -(result - plane.at(whichDim)*worldCoord[whichDim]
                       )/plane.at(whichDim);
            return result;
        };
        glm::vec4 worldCoords;
        glm::ivec4 coordIdx;
        for (coordIdx[axe[0]]=0; coordIdx[axe[0]]<S; ++coordIdx[axe[0]] )
            for (coordIdx[axe[1]]=0; coordIdx[axe[1]]<S; ++coordIdx[axe[1]] )
                for (coordIdx[axe[2]]=0; coordIdx[axe[2]]<S; ++coordIdx[axe[2]] ){

            glm::vec4 u0 = unitVec(static_cast<axes>(axe[0]))*blockSize;
            glm::vec4 u1 = unitVec(static_cast<axes>(axe[1]))*blockSize;
            glm::vec4 u2 = unitVec(static_cast<axes>(axe[2]))*blockSize;
            worldCoords = startCoord()+vec4(coordIdx)*blockSize;
            //worldCoords[axe[3]] = getCubeNthCoord(worldCoords, axe[3]);
            axes axe3 = static_cast<axes>(axe[3]);
            float minCoord = startCoord()[axe[3]]
                , maxCoord = startCoord()[axe[3]] + S*blockSize;

            auto a = {getCubeNthCoord(worldCoords, axe[3]),
                      getCubeNthCoord(worldCoords+u1, axe[3]),
                      getCubeNthCoord(worldCoords+u2, axe[3]),
                      getCubeNthCoord(worldCoords+u2+u1, axe[3]),
                      getCubeNthCoord(worldCoords+u0, axe[3]),
                      getCubeNthCoord(worldCoords+u0+u1, axe[3]),
                      getCubeNthCoord(worldCoords+u0+u2, axe[3]),
                      getCubeNthCoord(worldCoords+u0+u2+u1, axe[3])};
            maxCoord = std::max(a);
            minCoord = std::min(a);

            int minIdx = std::max(coordToIndex(minCoord, axe3), 0);
            int maxIdx = std::min(coordToIndex(maxCoord, axe3), int(S));
            for (coordIdx[axe[3]] = minIdx;
                coordIdx[axe[3]] <= maxIdx;
                ++coordIdx[axe[3]] ){
                processBlock(coordIdx);
            }

//            coordIdx[axe[3]] = (worldCoords[axe[3]]-startCoord()[axe[3]]
//                                )/blockSize;
//            processBlock(coordIdx);
//            coordIdx[axe[3]] += 1;
//            processBlock(coordIdx);
//            coordIdx[axe[3]] -= 2;
//            processBlock(coordIdx);
        }
        return result;
    }
};

}
#endif // CHUNC4D_H
