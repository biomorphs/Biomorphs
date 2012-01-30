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
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float2 Pos : POSITION;
	float4 Colour : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float4 Colour : COLOR;
};

float4 PositionScale;

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	output.Pos = float4((input.Pos* PositionScale.zw) + PositionScale.xy,0.0,1.0);
	output.Colour = input.Colour;

    return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	//return float4(1.0f,1.0f,1.0f,1.0f);
	return input.Colour;
}


//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
		//SetRasterizerState(rs);	// wireframe!
		SetDepthStencilState(ds, 0);
		//SetBlendState(SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}


