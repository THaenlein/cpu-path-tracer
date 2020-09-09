/*
---------------------------------------------------------------------------
Open Asset Import Library (assimp)
---------------------------------------------------------------------------

Copyright (c) 2006-2019, assimp team

All rights reserved.

Redistribution and use of this software in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met:

* Redistributions of source code must retain the above
  copyright notice, this list of conditions and the
  following disclaimer.

* Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the
  following disclaimer in the documentation and/or other
  materials provided with the distribution.

* Neither the name of the assimp team, nor the names of its
  contributors may be used to endorse or promote products
  derived from this software without specific prior
  written permission of the assimp team.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------
*/

/** @file camera.h
 *  @brief Defines the aiCamera data structure
 */

#pragma once
#ifndef AI_CAMERA_H_INC
#define AI_CAMERA_H_INC

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
/** Helper structure to describe a virtual camera.
 *
 * Cameras have a representation in the node graph and can be animated.
 * An important aspect is that the camera itself is also part of the
 * scene-graph. This means, any values such as the look-at vector are not
 * *absolute*, they're <b>relative</b> to the coordinate system defined
 * by the node which corresponds to the camera. This allows for camera
 * animations. For static cameras parameters like the 'look-at' or 'up' vectors
 * are usually specified directly in aiCamera, but beware, they could also
 * be encoded in the node transformation. The following (pseudo)code sample
 * shows how to do it: <br><br>
 * @code
 * // Get the camera matrix for a camera at a specific time
 * // if the node hierarchy for the camera does not contain
 * // at least one animated node this is a static computation
 * get-camera-matrix (node sceneRoot, camera cam) : matrix
 * {
 *    node   cnd = find-node-for-camera(cam)
 *    matrix cmt = identity()
 *
 *    // as usual - get the absolute camera transformation for this frame
 *    for each node nd in hierarchy from sceneRoot to cnd
 *      matrix cur
 *      if (is-animated(nd))
 *         cur = eval-animation(nd)
 *      else cur = nd->mTransformation;
 *      cmt = mult-matrices( cmt, cur )
 *    end for
 *
 *    // now multiply with the camera's own local transform
 *    cam = mult-matrices (cam, get-camera-matrix(cmt) )
 * }
 * @endcode
 *
 * @note some file formats (such as 3DS, ASE) export a "target point" -
 * the point the camera is looking at (it can even be animated). Assimp
 * writes the target point as a subnode of the camera's main node,
 * called "<camName>.Target". However this is just additional information
 * then the transformation tracks of the camera main node make the
 * camera already look in the right direction.
 *
*/
struct aiCamera
{
    /** The name of the camera.
     *
     *  There must be a node in the scenegraph with the same name.
     *  This node specifies the position of the camera in the scene
     *  hierarchy and can be animated.
     */
    C_STRUCT aiString mName;

    /** Position of the camera relative to the coordinate space
     *  defined by the corresponding node.
     *
     *  The default value is 0|0|0.
     */
    C_STRUCT aiVector3D mPosition;

    /** 'Up' - vector of the camera coordinate system relative to
     *  the coordinate space defined by the corresponding node.
     *
     *  The 'right' vector of the camera coordinate system is
     *  the cross product of  the up and lookAt vectors.
     *  The default value is 0|1|0. The vector
     *  may be normalized, but it needn't.
     */
    C_STRUCT aiVector3D mUp;


    /** 'LookAt' - vector of the camera coordinate system relative to
     *  the coordinate space defined by the corresponding node.
     *
     *  This is the viewing direction of the user.
     *  The default value is 0|0|1. The vector
     *  may be normalized, but it needn't.
     */
    C_STRUCT aiVector3D mLookAt;


    /** 'Right' - vector of the camera relative to the coordinate space
     *  defined by the corresponding node.
     *
     *  This is the viewing direction of the user.
     *  The default value is 0|0|0. The vector
     *  may be normalized, but it needn't.
     */
    C_STRUCT aiVector3D mRight;

    /** Half horizontal field of view angle, in radians.
     *
     *  The field of view angle is the angle between the center
     *  line of the screen and the left or right border.
     *  The default value is 1/4PI.
     */
    float mHorizontalFOV;

    /** Distance of the near clipping plane from the camera.
     *
     * The value may not be 0.f (for arithmetic reasons to prevent
     * a division through zero). The default value is 0.1f.
     */
    float mClipPlaneNear;

    /** Distance of the far clipping plane from the camera.
     *
     * The far clipping plane must, of course, be further away than the
     * near clipping plane. The default value is 1000.f. The ratio
     * between the near and the far plane should not be too
     * large (between 1000-10000 should be ok) to avoid floating-point
     * inaccuracies which could lead to z-fighting.
     */
    float mClipPlaneFar;

    /** Screen aspect ratio.
     *
     * This is the ration between the width and the height of the
     * screen. Typical values are 4/3, 1/2 or 1/1. This value is
     * 0 if the aspect ratio is not defined in the source file.
     * 0 is also the default value.
     */
    float mAspect;

#ifdef __cplusplus

    aiCamera() AI_NO_EXCEPT
        : mUp               (0.f,1.f,0.f)
        , mLookAt           (0.f,0.f,1.f)
        , mHorizontalFOV    (0.25f * (float)AI_MATH_PI)
        , mClipPlaneNear    (0.1f)
        , mClipPlaneFar     (1000.f)
        , mAspect           (0.f)
    {}

    /** @brief Get a *right-handed* camera matrix from me
     *  @param out Camera matrix to be filled
     */
    void GetCameraMatrix (aiMatrix4x4& out) const
    {
        /** todo: test ... should work, but i'm not absolutely sure */

        /** We don't know whether these vectors are already normalized ...*/
        aiVector3D lookAt = mLookAt;    lookAt.Normalize();
        aiVector3D up = mUp;            up.Normalize();
        aiVector3D right = mRight;      right.Normalize();

        out.a4 = mPosition.x;
        out.b4 = mPosition.y;
        out.c4 = mPosition.z;

        out.a1 = right.x;
        out.b1 = right.y;
        out.c1 = right.z;

        out.a2 = up.x;
        out.b2 = up.y;
        out.c2 = up.z;

        out.a3 = lookAt.x;
        out.b3 = lookAt.y;
        out.c3 = lookAt.z;

        out.d1 = out.d2 = out.d3 = 0.f;
        out.d4 = 1.f;
    }

    /* Transformation matrix layout
    *
    * [Right.x] [Up.x] [Back.x] [Position.x]
    * [Right.y] [Up.y] [Back.y] [Position.y]
    * [Right.z] [Up.z] [Back.z] [Position.Z]
    * [       ] [    ] [      ] [Unit Scale]
    */
    void Transform(aiMatrix4x4& transMatrix)
    {
        mRight = { transMatrix.a1, transMatrix.b1, transMatrix.c1 };
        mUp = { transMatrix.a2, transMatrix.b2, transMatrix.c2 };
        mLookAt = { -transMatrix.a3, -transMatrix.b3, -transMatrix.c3 };
        mPosition = { transMatrix.a4, transMatrix.b4, transMatrix.c4 };

        mRight.Normalize();
        mUp.Normalize();
        mLookAt.Normalize();
    }

    void print(std::ostream& printStream)
    {
        printStream << "Camera name: " << mName.C_Str() << '\n';
        printStream << '\t' << "Position: " << mPosition.x << ", " << mPosition.y << ", " << mPosition.z << '\n';
        printStream << '\t' << "Up: " << mUp.x << ", " << mUp.y << ", " << mUp.z << '\n';
        printStream << '\t' << "Right: " << mRight.x << ", " << mRight.y << ", " << mRight.z << '\n';
        printStream << '\t' << "Look at: " << mLookAt.x << ", " << mLookAt.y << ", " << mLookAt.z << "\n\n";
    }

#endif
};


#ifdef __cplusplus
}
#endif

#endif // AI_CAMERA_H_INC
