#ifndef SHADOWED_DEVICE_INCLUDED
#define SHADOWED_DEVICE_INCLUDED

#include "device_types.h"
#include "device.h"
#include "perf_grab.h"

// Performance grabs
namespace ShadowPerfGrab
{
	enum Stats
	{
		NumIBChanges = 0,
		NumVBChanges,
		NumEffectChanges,
		NumDrawCalls,

		MaxPerfGrabs
	};
}

DeclarePerfGrab(ShadowDevicePerfs,ShadowPerfGrab::MaxPerfGrabs,ShadowPerfGrab::Stats);
//////////////////

class ShadowedDevice
{
public:
	ShadowedDevice(Device* d)
		: m_device(d)
	{
		Invalidate();
	}

	static const unsigned int kuMaxVertexBuffers = 16;

	inline Device& GetDevice()
	{
		return *m_device;
	}

	inline void StartFrame()
	{
		DoPerfGrab(ShadowDevicePerfs::Reset());
		Invalidate();
	}

	inline void EndFrame()
	{
	}

	inline void SetRenderTargets( Rendertarget& colourTarget, DepthStencilBuffer& depthStencilTarget )
	{
		if( (colourTarget != m_colourTarget) || (depthStencilTarget != m_depthStencilBuffer) )
		{
			m_colourTarget = colourTarget;
			m_depthStencilBuffer = depthStencilTarget;

			m_device->SetRenderTargets(&colourTarget, &depthStencilTarget);
		}
	}

	inline void SetPrimitiveTopology(PrimitiveTopology t)
	{
		if( t != m_primitiveType )
		{
			m_primitiveType = t;
			m_device->SetPrimitiveTopology(t);
		}
	}

	inline void SetInputLayout(ShaderInputLayout& l)
	{
		if(l != m_inputLayout )
		{
			m_inputLayout = l;
			m_device->SetInputLayout(l);
		}
	}

	inline void SetVertexBuffer(int streamIndex, VertexBuffer& vb)
	{
		if( streamIndex < kuMaxVertexBuffers && vb != m_vertexBuffers[streamIndex] )
		{
			m_vertexBuffers[streamIndex] = vb;
			m_device->SetVertexBuffer(streamIndex, vb);
			DoPerfGrab(ShadowDevicePerfs::Increment(ShadowPerfGrab::NumVBChanges));
		}
	}

	inline void SetIndexBuffer(IndexBuffer& ib)
	{
		if( ib != m_indexBuffer )
		{
			m_indexBuffer = ib;
			m_device->SetIndexBuffer(ib);
			DoPerfGrab(ShadowDevicePerfs::Increment(ShadowPerfGrab::NumIBChanges));
		}
	}

	inline void Invalidate()
	{
		m_colourTarget.Invalidate();
		m_depthStencilBuffer.Invalidate();
		m_primitiveType = PRIMITIVE_NA;
		m_inputLayout.Invalidate();

		for(int i=0;i<kuMaxVertexBuffers;++i)
		{
			m_vertexBuffers[i].Invalidate();
		}

		m_indexBuffer.Invalidate();
		m_shaderTechnique.Invalidate();
		m_pass = -1;
	}

	inline void DrawIndexed(DrawIndexedParameters& params)
	{
		m_device->DrawIndexed(params);

		DoPerfGrab(ShadowDevicePerfs::Increment(ShadowPerfGrab::NumDrawCalls));
	}

	inline void SetTechnique(EffectTechnique& technique, int pass)
	{
		if( technique != m_shaderTechnique || pass != m_pass || technique.m_dirty )
		{
			m_shaderTechnique = technique;
			m_pass = pass;
			technique.m_dirty = false;

			m_device->SetTechnique(technique, pass);
			DoPerfGrab(ShadowDevicePerfs::Increment(ShadowPerfGrab::NumEffectChanges));
		}
	}

private:

	Rendertarget m_colourTarget;
	DepthStencilBuffer m_depthStencilBuffer;
	PrimitiveTopology m_primitiveType;
	ShaderInputLayout m_inputLayout;
	VertexBuffer m_vertexBuffers[kuMaxVertexBuffers];
	IndexBuffer m_indexBuffer;
	EffectTechnique m_shaderTechnique;
	int m_pass;

	Device* m_device;
};

#endif