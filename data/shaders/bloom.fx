///////////////////////////////////////////////////////////////////////////////////////
// Render States
RasterizerState rs
{
FillMode = WIREFRAME;
};

DepthStencilState ds
{
DepthEnable = false;
};

BlendState SrcAlphaBlendingOff
{
    BlendEnable[0] = false;
};

///////////////////////////////////////////////////////////////////////////////////////
// Input / Output
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

///////////////////////////////////////////////////////////////////////////////////////
// Textures and samplers
Texture2D BlitTexture;				// main texture
Texture2D Tinyblur;					// tiny blur texture

SamplerState sampleLinear
{
    Filter = MIN_MAG_MIP_LINEAR;	// play around with this
    AddressU = Wrap;
    AddressV = Wrap;
};

///////////////////////////////////////////////////////////////////////////////////////
// Constants

float4 PositionScale;
float2 PixelSize;
float2 BloomConsts;	// x = threshold, y = mul

///////////////////////////////////////////////////////////////////////////////////////
// Vertex shader
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;

	output.Pos = float4((input.Pos* PositionScale.zw) + PositionScale.xy,0.0,1.0);
	output.UV = input.UV;

    return output;
}


///////////////////////////////////////////////////////////////////////////////////////
// Combine pass
float4 PS_COMBINE( PS_INPUT input) : SV_Target
{
	float4 full = BlitTexture.Sample( sampleLinear, input.UV );
	float4 tblr = Tinyblur.Sample( sampleLinear, input.UV );

	// now calculate the luminance from the blurred colour
	float lum = (0.4f * tblr.x) + (0.55f * tblr.y) + (0.16f * tblr.z);

	float br = max(lum - BloomConsts.x,0.0f) * BloomConsts.y;

	return full + (tblr * br);
}

technique10 Combine
{
    pass P0
    {
		SetBlendState(SrcAlphaBlendingOff, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState(ds, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_COMBINE() ) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// Downsample pass (needs some work)
float4 PS_DS( PS_INPUT input) : SV_Target
{
	// do a tiny 4 tap blur to help with aliasing
	float4 s0 = BlitTexture.Sample( sampleLinear, input.UV + float2(-PixelSize.x,PixelSize.y) );
	float4 s1 = BlitTexture.Sample( sampleLinear, input.UV + float2( PixelSize.x,PixelSize.y) );
	float4 s2 = BlitTexture.Sample( sampleLinear, input.UV + float2(-PixelSize.x,-PixelSize.y) );
	float4 s3 = BlitTexture.Sample( sampleLinear, input.UV + float2( PixelSize.x,-PixelSize.y) );

	return ( (s0 + s1 + s2 + s3) / 4.0f );
}

technique10 Downsample
{
    pass P0
    {
		SetBlendState(SrcAlphaBlendingOff, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState(ds, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_DS() ) );
    }
}


///////////////////////////////////////////////////////////////////////////////////////
// Horizontal Gaussian blur
float4 PS_BH( PS_INPUT input) : SV_Target
{
	const float blurSize = max(PixelSize.x, PixelSize.y);
	float4 sum = float4(0.0,0.0,0.0,0.0);
	float2 vTexCoord = input.UV;

   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x - 4.0*blurSize, vTexCoord.y)) * 0.05;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x - 3.0*blurSize, vTexCoord.y)) * 0.09;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x - 2.0*blurSize, vTexCoord.y)) * 0.12;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x - blurSize, vTexCoord.y)) * 0.15;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y)) * 0.16;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x + blurSize, vTexCoord.y)) * 0.15;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x + 2.0*blurSize, vTexCoord.y)) * 0.12;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x + 3.0*blurSize, vTexCoord.y)) * 0.09;
   sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x + 4.0*blurSize, vTexCoord.y)) * 0.05;
 
   return sum;
}

technique10 BlurH
{
    pass P0
    {
		//SetRasterizerState(rs);	// wireframe!
		SetBlendState(SrcAlphaBlendingOff, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState(ds, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BH() ) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////
// Vertical Gaussian blur
float4 PS_BV( PS_INPUT input) : SV_Target
{
	const float blurSize = max(PixelSize.x, PixelSize.y);
	float4 sum = float4(0.0,0.0,0.0,0.0);
	float2 vTexCoord = input.UV;

	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y - 4.0*blurSize)) * 0.05;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y - 3.0*blurSize)) * 0.09;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y - 2.0*blurSize)) * 0.12;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y - blurSize)) * 0.15;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y)) * 0.16;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y + blurSize)) * 0.15;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y + 2.0*blurSize)) * 0.12;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y + 3.0*blurSize)) * 0.09;
	sum += BlitTexture.Sample(sampleLinear, float2(vTexCoord.x, vTexCoord.y + 4.0*blurSize)) * 0.05;
 
	return sum;
}

technique10 BlurV
{
    pass P0
    {
		//SetRasterizerState(rs);	// wireframe!
		SetBlendState(SrcAlphaBlendingOff, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState(ds, 0);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BV() ) );
    }
}