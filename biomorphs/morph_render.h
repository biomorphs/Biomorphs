#ifndef MORPH_RENDER_INCLUDED
#define MORPH_RENDER_INCLUDED

#include "framework\graphics\device_types.h"
#include "morph_dna.h"

class MorphRender
{
public:
	struct Parameters
	{
		int mTextureWidth;
		int mTextureHeight;
	};
	MorphRender();
	~MorphRender();

	bool Initialise( Device* d, const Parameters& p );
	bool Release();

	void StartRendering();	// call this at the start of the frame
	void DrawBiomorph( MorphDNA& dna );
	void EndRendering(D3DXVECTOR4 posScale);	// call this to push all data to D3D

	inline int GetVertexCount()
	{
		return m_verticesWritten;
	}

	Texture2D CopyOutputTexture(Texture2D& texture);

private:
	// vertex structure
	struct MorphVertex
	{
		D3DXVECTOR2 mPosition;
		D3DXVECTOR4 mColour;	
	};

	// calculate direction and perpendicular (normalised)
	inline void GetGeometryVectors( float angle, float length, D3DXVECTOR2& direction, D3DXVECTOR2& perpendicular );
	inline int WriteVertices( MorphVertex*& vertices, 
								float width,
								const D3DXVECTOR4& colour,
								const D3DXVECTOR2& origin,
								const D3DXVECTOR2& direction, 
								const D3DXVECTOR2& perpendicular );
	inline int WriteQuadIndices( unsigned int*& indices, int vertexOffset );

	// recursion structure (saves passing lots of arguments)
	// base structure based on the MorphDNA
	struct RecursionParams
	{
		int vertexOffset;		// index buffer vertex offset
		int branchDepth;		// branch depth

		D3DXVECTOR2 Origin;		// branch origin
		float Angle;			// branch angle (0 = straight up)
		float Length;			// length of a branch
		D3DXVECTOR4 Colour;		// branch colour
	};

	// returns indices written
	int _drawRecursive( MorphDNA& dna, RecursionParams& params, MorphVertex*& vertices, unsigned int*& indices );

	static const int kMaxVertices = 4 * 1024 * 1024;
	static const int kMaxIndices = kMaxVertices * 6;

	Parameters m_params;

	// temporary pointers to locked data
	MorphVertex* m_lockedVBData;
	unsigned int* m_lockedIBData;
	int m_verticesWritten;
	int m_indicesWritten;

	Device* m_device;
	Effect m_shader;
	VertexDescriptor m_vd;
	VertexBuffer m_vb;
	IndexBuffer m_ib;
	ShaderInputLayout m_inputLayout;

	// render to texture
	Rendertarget m_rt;
	DepthStencilBuffer m_depthStencil;
	Texture2D m_texture;
};

inline int MorphRender::WriteQuadIndices( unsigned int*& indices, int vertexOffset )
{
	// add some indices for the 2 triangles
	indices[0] = vertexOffset + 0;	
	indices[1] = vertexOffset + 2;		
	indices[2] = vertexOffset + 1;

	indices[3] = vertexOffset + 0;	
	indices[4] = vertexOffset + 3;		
	indices[5] = vertexOffset + 2;
	indices += 6;

	return 6;
}

// write verts for a single branch
inline int MorphRender::WriteVertices( MorphVertex*& vertices, 
										float width,
										const D3DXVECTOR4& colour,
										const D3DXVECTOR2& origin,
										const D3DXVECTOR2& direction, 
										const D3DXVECTOR2& perpendicular )
{
	const D3DXVECTOR2 perp = perpendicular * width;

	vertices[0].mPosition = origin - perp;
	vertices[0].mColour = colour;

	vertices[1].mPosition = origin + perp;
	vertices[1].mColour = colour;

	vertices[2].mPosition = origin + direction + perp;
	vertices[2].mColour = colour;

	vertices[3].mPosition = origin + direction - perp;
	vertices[3].mColour = colour;
	vertices += 4;

	return 4;
}

// calculate direction and perpendicular (normalised)
inline void MorphRender::GetGeometryVectors( float angle, float length, D3DXVECTOR2& direction, D3DXVECTOR2& perpendicular )
{
	const D3DXVECTOR2 baseDirection(0.0f, 1.0f);

	// rotate the direction, based on the angle
	D3DXMATRIX branchRotationMat;
	D3DXMatrixRotationZ( &branchRotationMat, angle );

	// rotate the vector
	D3DXVECTOR4 branchDirection4;
	D3DXVec2Transform( &branchDirection4, &baseDirection, &branchRotationMat );
	
	// resolve to normalised 2d vector
	D3DXVECTOR2 branchDirection2( branchDirection4.x, branchDirection4.y );
	D3DXVec2Normalize( &direction, &branchDirection2 );
	direction *= length;

	// now get the normalised perpendicular
	D3DXVECTOR3 branchPerp3;
	const D3DXVECTOR3 v0( 0.0f, 0.0f, -1.0f);
	D3DXVECTOR3 v1( branchDirection2.x, branchDirection2.y, 0.0f );
	D3DXVec3Cross( &branchPerp3, &v0, &v1 );
	D3DXVECTOR2 branchPerpendicular( branchPerp3.x, branchPerp3.y );
	D3DXVec2Normalize( &branchPerpendicular, &branchPerpendicular );
	perpendicular = branchPerpendicular * 0.5f;
}

#endif