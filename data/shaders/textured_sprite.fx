RasterizerState rs
{
FillMode = WIREFRAME;
};

DepthStencilState ds
{
DepthEnable = false;
};

BlendState SrcAlphaBlendingAdd
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ONE;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float2 Pos : POSITION;
	float2 UV : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 UV : TEXCOORD;
};

Texture2D BlitTexture;
SamplerState sampleTile
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

float4 PositionScale;

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	output.Pos = float4((input.Pos* PositionScale.zw) + PositionScale.xy,0.0,1.0);
	output.UV = input.UV;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 sprite = BlitTexture.Sample( sampleTile, input.UV );
	return sprite;
}


//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
		//SetRasterizerState(rs);	// wireframe!
		SetBlendState(SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState(ds, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}


