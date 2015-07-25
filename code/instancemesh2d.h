/************************************************************************
*    FILE NAME:     instancemesh2d.h
*
*    DESCRIPTION:   Class to render multiple 2D sprites using a single
*					draw call
************************************************************************/

#ifndef __instance_mesh_2d_h__
#define __instance_mesh_2d_h__

// Windows lib dependencies
#include <atlbase.h>

// Standard lib dependencies
#include <string>
#include <map>
#include <functional>

// DirectX lib dependencies
#include <d3dx9.h>

// Boost lib dependencies
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>

// Game lib dependencies
#include <common/defs.h>
#include <common/uv.h>
#include <common/color.h>
#include <common/point.h>
#include <common/matrix.h>
#include <common/worldpoint.h>

// Forward declaration(s)
class CMegaTexture;
class CMegaTextureComponent;
class CSpriteGroup2D;
class CActorSprite2D;

class CInstanceMesh2D : public boost::noncopyable
{
public:

    // Constructor
    CInstanceMesh2D();

    // Initialize the passed in sprite to the instance mesh
    void InitInstanceSprite( CActorSprite2D * pSprite );
    void InitInstanceSprite( CSpriteGroup2D * pSprite );

    // Add a sprite to the instance mesh
    void AddSprite( CSpriteGroup2D * pSprite );

    // Initialize the mesh
    void Init( const std::string & megatextureName );

    // Reset the instance buffer to the size of the group vector
    void ResetInstanceBuffer();

    // Render the instance mesh
    void Render();

    // Clear the render vector
    void Clear();

private:

    //////////////////////////////////////////////////////////////
    //	INSTANCE MESH SPECIFIC VERTEX DATA
    //////////////////////////////////////////////////////////////

    // Special instance mesh vertex object
    class CVertexData
    {
    public:

        CVertexData() {}
        CPoint vert;	// Verts
        uint uvIndex;	// Index used to determine UVs
    };


    //////////////////////////////////////////////////////////////
    //	INSTANCE MESH SPECIFIC INSTANCE DATA
    //////////////////////////////////////////////////////////////

    // Special instance mesh vertex object
    class CInstanceData
    {
    public:

        // We fit the matrix data into 12 floats instead of 16
        void SetMatrix( const D3DXMATRIX & matrix )
        {
            mat11 = matrix._11; mat12 = matrix._12; mat13 = matrix._13; mat14 = matrix._14;
            mat21 = matrix._21; mat22 = matrix._22; mat23 = matrix._23; mat24 = matrix._24;
            mat31 = matrix._31; mat32 = matrix._32; mat33 = matrix._33; mat34 = matrix._34;
            mat41 = matrix._41; mat42 = matrix._42; mat43 = matrix._43; mat44 = matrix._44;
        }

        // Set the color
        void SetColor( const CColor & color )
        {
            r = color.r;
            g = color.g;
            b = color.b;
            a = color.a;
        }
        
        // Set the UVs
        void SetUVs( const CUV & uv1, const CUV & uv2 )
        {
            u1 = uv1.u;
            v1 = uv1.v;
            u2 = uv2.u;
            v2 = uv2.v;
        }
        
        // Set the UVs
        void SetUVs( float uv[4] )
        {
            u1 = uv[0];
            v1 = uv[1];
            u2 = uv[2];
            v2 = uv[3];
        }
        
        // Instance matrix. I wrote out all floats individually so that I know
        // the exact order they're in
        float mat11, mat12, mat13, mat14;
        float mat21, mat22, mat23, mat24;
        float mat31, mat32, mat33, mat34;
        float mat41, mat42, mat43, mat44;

        // Color modifier
        float r,g,b,a;

        // We only have two Us and two Vs, so currently the 2D instancing doesn't support
        // UV mapping diagnally
        float u1,v1,u2,v2;
    };

    //////////////////////////////////////////////////////////////
    //	Class to keep track of frame index
    //////////////////////////////////////////////////////////////
    class SpriteGrp
    {
    public:
        SpriteGrp( CSpriteGroup2D * pSpriteGroupp, int frmIndex )
            : pSpriteGrp(pSpriteGroupp), frameIndex(frmIndex)
        {}

        CSpriteGroup2D * GetSpriteGrp()
        { return pSpriteGrp; }

        int GetFrameIndex()
        { return frameIndex; }

    private:
        CSpriteGroup2D * pSpriteGrp;
        int frameIndex;
    };
    

private:

    // Update the mesh information
    void Update();

    // Display error information
    void DisplayError( HRESULT hr );

private:

    // MultiMap of sprite groups. These objects own none of these sprites
    std::multimap<CWorldValue, SpriteGrp, std::greater<CWorldValue>> pRenderMultiMap;
    std::multimap<CWorldValue, SpriteGrp, std::greater<CWorldValue>>::iterator renderIter;

    // Vertex buffer
    CComPtr<IDirect3DVertexBuffer9> spVertexBuffer;
    CComPtr<IDirect3DIndexBuffer9> spIndexBuffer;
    CComPtr<IDirect3DVertexDeclaration9> spVertexDeclaration;
    CComPtr<IDirect3DVertexBuffer9> spInstanceBuffer;

    // Texture information that the instance mesh is using
    CMegaTexture * pMegaTexture;

    // The total number of instances the instance buffer can hold
    size_t instanceCount;

    // Constants
    const int VERTEX_COUNT;
    const int FACE_COUNT;
    const int INDEX_COUNT;

};

#endif  // __instance_mesh_2d_h__
