#include "device_types.h"

EffectTechnique Effect::GetTechniqueByName(const char* name)
{
	EffectTechnique result;
	if( IsValid() )
	{
		result.m_technique = m_effect->GetTechniqueByName(name);

		// get the pass count
		D3D10_TECHNIQUE_DESC techDesc;
		result.m_technique->GetDesc( &techDesc );
		result.m_passCount = techDesc.Passes;
		result.m_parent = this;
	}

	return result;
}

TextureSampler EffectTechnique::GetSamplerByName(const char* name)
{
	TextureSampler result;

	ID3D10EffectVariable* var = m_parent->m_effect->GetVariableByName(name);
	if( var && var->IsValid() )
	{
		result.m_sampler = var->AsShaderResource();
		result.m_effect = this;
	}
	else
	{
		printf("Unknown shader sampler '%s\n", name);
	}

	return result;
}

VectorConstant EffectTechnique::GetVectorConstant(const char* name)
{
	VectorConstant result;

	if( IsValid() )
	{
		ID3D10EffectVariable* var =  m_parent->m_effect->GetVariableByName(name);
		if( var )
		{
			result.m_variable = var->AsVector();
		}
	}

	return result;
}

MatrixConstant EffectTechnique::GetMatrixConstant(const char* name)
{
	MatrixConstant result;

	if( IsValid() )
	{
		result.m_variable = m_parent->m_effect->GetVariableByName(name)->AsMatrix();
	}

	return result;
}

void MatrixConstant::Apply()
{
	if( m_variable )
	{
		m_variable->SetMatrix((float*)m_value);
	}
}

void MatrixConstant::Set(D3DXMATRIX mat)
{
	m_value = mat;
}

void VectorConstant::Set(D3DXVECTOR4 vec)
{
	m_value = vec;
}

void VectorConstant::Apply()
{
	if( m_variable )
	{
		m_variable->SetFloatVector((float*)m_value);
	}
}

void TextureSampler::Set(Texture2D& t)
{
	if( m_sampler && t.IsValid() )
	{
		HRESULT hr = (m_sampler->SetResource( t.m_shaderResource ));
		if( FAILED(hr) )
		{
			printf("Bah");
		}
	}
}