
/************************************************************************
*    FILE NAME:       megatexture.h
*
*    DESCRIPTION:     Class to create a single texture out of several
*                     textures. This is used in conjunction with the
*                     instance mesh.
************************************************************************/

#ifndef __mega_texture_h__
#define __mega_texture_h__

// Windows lib dependencies
#include <atlbase.h>

// DirectX lib dependencies
#include <d3dx9.h>

// Standard lib dependencies
#include <string>

// Boost lib dependencies
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

// Game lib dependencies
#include <common/pointint.h>
#include <common/size.h>
#include <common/uv.h>
#include <common/defs.h>

// Forward declaration(s)
class CMegaTextureComponent;
class CTexturePartition;

namespace NText
{
    class CTextureFor2D;
}

// Typedefs for boost containers and objects
typedef boost::ptr_map< NText::CTextureFor2D *, CMegaTextureComponent > SPComponentMap;
typedef SPComponentMap::iterator SPComponentMapIter;
typedef boost::ptr_vector< boost::ptr_vector<CTexturePartition> > SPPartitionVecVec;

class CMegaTexture
{
public:

    // Constructor
    CMegaTexture();

    // Destructor
    virtual ~CMegaTexture();

    // Create a mega texture using the group name passed in
    void CreateMegaTexture( const std::string & group, uint wLimit );

    // Get the mega texture's texture
    NText::CTextureFor2D * GetTexture();

    // Get the UVs of a texture
    float * GetUVs( NText::CTextureFor2D * pTex );

    // Render the mega texture
    void Render();

private:

    // Initialize the mega texture's buffers
    void InitBuffers();

    // Try to fit a texture into partitions
    bool FitTextureToPartition( int row, int column, 
                                CMegaTextureComponent * pComponent, 
                                SPPartitionVecVec & spPartVecVec ); 

    // If any textures are overlapping, assert
    void CheckTextureOverlap();

    // Calculate the UVs of a mega texture
    void CalculateGroupUVs();

    // Render the textures to a single texture surface
    void CopyToMegaTexture( const std::string & group );

    // Display error information
    void DisplayError( HRESULT hr, const std::string & functionStr, int lineValue );

private:

    // Mega texture
    boost::scoped_ptr< NText::CTextureFor2D > spMegaTexture;

    // Map to hold the texture components
    SPComponentMap spComponentMap;
    SPComponentMapIter spComponentMapIter;

    // The mega texture's buffers
    CComPtr< IDirect3DVertexBuffer9 > spVertexBuffer;
    CComPtr< IDirect3DIndexBuffer9 > spIndexBuffer;
    CComPtr< IDirect3DVertexDeclaration9 > spVertexDeclaration;

    // Constants
    const int VERTEX_COUNT;
    const int FACE_COUNT;
    const int INDEX_COUNT;

};

#endif  // __mega_texture_h__


