#ifndef MORPH_RENDER_INCLUDED
#define MORPH_RENDER_INCLUDED

#include "framework\graphics\device_types.h"
#include "morph_dna.h"
#include "core/minmax.h"

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
	void CalculateBounds( MorphDNA& dna, D3DXVECTOR2& min, D3DXVECTOR2& max );
	void DrawBiomorph( MorphDNA& dna, D3DXVECTOR2 offset = D3DXVECTOR2(0.0f,0.0f), float size = 1.0f );
	void EndRendering();	// call this to push all data to D3D

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

	inline void CalculateBounds( const D3DXVECTOR2& origin, D3DXVECTOR2& direction, D3DXVECTOR2& min, D3DXVECTOR2& max );
	inline void GetGeometryVectors( float angle, float length, D3DXVECTOR2& direction, D3DXVECTOR2& perpendicular );
	__forceinline int WriteVertices( MorphVertex*& vertices, 
								float width,
								const D3DXVECTOR4& colour,
								const D3DXVECTOR2& origin,
								const D3DXVECTOR2& direction, 
								const D3DXVECTOR2& perpendicular,
								const D3DXVECTOR2& offset,
								float drawScale );
	__forceinline int WriteQuadIndices( unsigned int*& indices, int vertexOffset );

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

		bool Draw;				// flag to switch between draw and bounds calculation
		D3DXVECTOR2 BoundsMin;
		D3DXVECTOR2 BoundsMax;

		float DrawScale;
	};

	// returns indices written
	int _drawRecursive( MorphDNA& dna, RecursionParams& params, MorphVertex*& vertices, unsigned int*& indices );
	inline void _buildRenderParameters( MorphDNA& dna, RecursionParams& p );

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

inline void MorphRender::_buildRenderParameters( MorphDNA& dna, RecursionParams& p )
{
	p.vertexOffset = m_verticesWritten;
	p.branchDepth = BASEDEPTH(dna);
	p.Angle = 0;
	p.Length = BASELENGTH(dna);
	p.Origin = D3DXVECTOR2(0.0f,0.0f);
	p.Colour = BASECOLOUR(dna);
	p.BoundsMin = D3DXVECTOR2(1.0f,1.0f);
	p.BoundsMax = D3DXVECTOR2(0.0f,0.0f);
	p.Draw = false;
	p.DrawScale = 1.0f;
}

__forceinline int MorphRender::WriteQuadIndices( unsigned int*& indices, int vertexOffset )
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
__forceinline int MorphRender::WriteVertices( MorphVertex*& vertices, 
										float width,
										const D3DXVECTOR4& colour,
										const D3DXVECTOR2& origin,
										const D3DXVECTOR2& direction, 
										const D3DXVECTOR2& perpendicular,
										const D3DXVECTOR2& offset,
										float drawScale)
{
	const D3DXVECTOR2 perp = perpendicular * width * drawScale;
	const D3DXVECTOR2 o(origin + offset);
	const D3DXVECTOR2 d = direction * drawScale;

	vertices[0].mPosition = o - perp;
	vertices[0].mColour = colour;

	vertices[1].mPosition = o + perp;
	vertices[1].mColour = colour;

	vertices[2].mPosition = o + d + perp;
	vertices[2].mColour = colour;

	vertices[3].mPosition = o + d - perp;
	vertices[3].mColour = colour;
	vertices += 4;

	return 4;
}

inline void MorphRender::CalculateBounds( const D3DXVECTOR2& origin, D3DXVECTOR2& direction, D3DXVECTOR2& min, D3DXVECTOR2& max )
{
	// calculate min and max
	min.x = Bounds::Min(min.x, origin.x);
	min.x = Bounds::Min(min.x, origin.x + direction.x);

	min.y = Bounds::Min(min.y, origin.y);
	min.y = Bounds::Min(min.y, origin.y + direction.y);

	max.x = Bounds::Max(max.x, origin.x);
	max.x = Bounds::Max(max.x, origin.x + direction.x);

	max.y = Bounds::Max(max.y, origin.y);
	max.y = Bounds::Max(max.y, origin.y + direction.y);
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