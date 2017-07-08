#ifndef WK4DPOINTSOFVIEW_H
#define WK4DPOINTSOFVIEW_H

#include "wk4dcore.h"
#include "wk4dmatrix5.h"
#include "glm/gtx/projection.hpp"

namespace WK4d
{

struct FPSPointOfView
{
    hyperPlane4 planeImOn=hyperPlane4(vec4(0,0,0,0), vec4(0,0,0,1));
    vec4 myCoord;
    static vec4 myUp;
    vec4 myFront=vec4{1,0,0,0};
    float myPitch; //agnle in radians
    char times=0;
    //inline
    vec4 getMyRigth() const
    {
        vec4 result = orthogonalToThree(planeImOn.getNormal(),
                                        myUp, myFront);
        for (int i=2; isZeroVec(result); i*=64){
            if(i<=0){
                assert (false);
            }
            result = orthogonalToThree(planeImOn.getNormal()*float{i},
                                           myUp*float{i}, myFront*float{i});
        }
        return glm::normalize(result);

    }
    inline
    vec4 myViewDirection() const
    {
        return std::cos(myPitch)*myFront + std::sin(myPitch)*myUp;
    }

    void rotateForwardUp(float f)
    {
        //differs from othera due to the fact, that it's not a spasesim:
        //this one rotation is gimgle-lockable
        myPitch += f;
        myPitch = std::min(myPitch, static_cast<float>( mathPi/2) );
        myPitch = std::max(myPitch, static_cast<float>(-mathPi/2) );
    }
    void rotateForwardRight(float ang)
    {
        vec4 newF = vecRotatedTowardAnother(myFront, getMyRigth(), ang);
        myFront = newF;
    }
    void rotateRightAna (float ang)
    {
        vec4 newA = vecRotatedTowardAnother(planeImOn.getNormal(), getMyRigth(), -ang);
        planeImOn = hyperPlane4(myCoord, newA);
    }
    void rotateForwardAna(float ang)
    {
        vec4 newF = vecRotatedTowardAnother(myFront, planeImOn.getNormal(), ang);
        vec4 newA = vecRotatedTowardAnother(planeImOn.getNormal(), myFront, -ang);
        myFront = newF;
        planeImOn = hyperPlane4(myCoord, newA);
    }
    matrix5x5 getWorldToHyperplaneLocalTransformMatrix() const
    {
        //матрица. смещаем. поворачиваем для нового базиса.
        //поворачиваем для pitch
        matrix5x5 transform = moveMatrix(-myCoord);
//        transform = toNewBasisRotationMatrix(myFront, getMyRigth(), myUp
//                        ,planeImOn.getNormal().normazized())*transform;
        transform = fromBasisToStandartRotationMatrix(myFront, getMyRigth(), myUp
                        ,glm::normalize(planeImOn.getNormal()))*transform;
        return transform;
    }
    matrix5x5 getHyperplaneLocalToWorldTransformMatrix() const
    {
        matrix5x5 transform = fromBasisToStandartRotationMatrix(myFront, getMyRigth(), myUp
                                                ,glm::normalize(planeImOn.getNormal()));
        transform = moveMatrix(myCoord)*transform;
        return transform;
    }
    void normalize()
    {
        myFront -= glm::proj(myFront, myUp);
        vec4 newA = planeImOn.getNormal();
        newA -= glm::proj(newA, myUp);
        newA -= glm::proj(newA, myFront);
        myFront = glm::normalize(myFront);
        planeImOn = hyperPlane4(myCoord, glm::normalize(newA));
    }
};

struct SpaceSimPointOfView
{
    hyperPlane4 planeImOn =  hyperPlane4(vec4{0,0,0,0},vec4{0,0,0,1});
    vec4 myCoord = {0,0,0,0};
    vec4 myUp = {0,0,1,0};
    vec4 myFront=vec4{1,0,0,0};

    inline
    vec4 getMyRigth() const
    {
        return glm::normalize(orthogonalToThree(planeImOn.getNormal(),myUp, myFront));
    }
    inline
    vec4 myViewDirection() const
    {
        return myFront;
    }
    void rotateForwardUp(float ang)
    {
        //differs from othera due to the fact, that it's not a spasesim:
        //this one rotation is gimgle-lockable
        vec4 newF = vecRotatedTowardAnother(myFront, myUp, ang);
        vec4 newU = vecRotatedTowardAnother(myUp, myFront, -ang);
        myUp   = newU;
        myFront= newF;
    }
    void rotateForwardRight(float ang)
    {
        vec4 newF = vecRotatedTowardAnother(myFront, getMyRigth(), ang);
        myFront = newF;
    }
    void rotateForwardAna(float ang)
    {
        vec4 newF = vecRotatedTowardAnother(myFront, planeImOn.getNormal(), ang);
        vec4 newA = vecRotatedTowardAnother(planeImOn.getNormal(),myFront, -ang);
        myFront = newF;
        planeImOn = hyperPlane4(myCoord, newA);
    }
    //2 following functions return new plane normal vector
    //il concider changing hyperplane's normal directly instead
    void rotateRightAna (float ang)
    {
        vec4 newA;
        newA = vecRotatedTowardAnother(planeImOn.getNormal(), getMyRigth(), -ang);
        planeImOn = hyperPlane4(myCoord, newA);
    }
    void rotateRightUp(float ang)
    {
        vec4 newU = vecRotatedTowardAnother(myUp, getMyRigth(), -ang);
        myUp = newU;
    }
    void rotateUpAna(float ang)
    {
        vec4 newUp = vecRotatedTowardAnother(myUp, planeImOn.getNormal(), ang);
        vec4 newA = vecRotatedTowardAnother(planeImOn.getNormal(),myUp, -ang);
        myUp = newUp;
        planeImOn = hyperPlane4(myCoord, newA);
    }

    matrix5x5 getWorldToHyperplaneLocalTransformMatrix() const
    {
        //матрица. смещаем. поворачиваем для нового базиса.
        //поворачиваем для pitch
        matrix5x5 transform = fromBasisToStandartRotationMatrix(myFront, getMyRigth(), myUp
                                                ,glm::normalize(planeImOn.getNormal()));
        transform = moveMatrix(myCoord)*transform;
        return transform;
    }
    matrix5x5 getHyperplaneLocalToWorldTransformMatrix() const
    {
        matrix5x5 transform = fromBasisToStandartRotationMatrix(myFront, getMyRigth()
                                            , myUp
                                            ,glm::normalize(planeImOn.getNormal()))
                            *moveMatrix(myCoord);
        //transform = *transform;
        return transform;
    }
    void normalize()
    {
        myUp -= glm::proj(myUp, myFront);
        vec4 newA = planeImOn.getNormal();
        newA -= glm::proj(newA, myUp);
        newA -= glm::proj(newA, myFront);
        glm::normalize(myUp);
        glm::normalize(myFront);
        planeImOn = hyperPlane4(myCoord, glm::normalize(newA));
    }
};

}


#endif // WK4DPOINTSOFVIEW_H
