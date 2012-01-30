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
	void DrawBiomorph( D3DXVECTOR2 origin, MorphDNA& dna );
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
	int _drawRecursive( MorphDNA& dna, RecursionParams& params, MorphVertex* vertices, unsigned int* indices );

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

#endif