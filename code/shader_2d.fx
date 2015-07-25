//-----------------------------------------------------------------------------
// GLOBAL VARIABLES
//-----------------------------------------------------------------------------

// Camera view matrix
float4x4 cameraViewProjMatrix;

// Object's color
float4 materialColor;

// texture filter
//int textureFilter;

// Texture
texture diffuseTexture;


//-----------------------------------------------------------------------------
// STRUCT DEFINITIONS
//-----------------------------------------------------------------------------

struct PS_INPUT
{
	float2 uv0    : TEXCOORD0;
    float4 color  : COLOR0;
};

struct VS_INPUT_COLOR_ONLY
{
	float4 pos	  : POSITION;
	float2 uv0    : TEXCOORD0;
};

struct VS_INPUT_INSTANCE
{
	float4 vPos	     : POSITION;
	uint vUVIndex    : BLENDINDICES;
	float4 iMatrix1  : TEXCOORD1;
	float4 iMatrix2  : TEXCOORD2;
	float4 iMatrix3  : TEXCOORD3;
	float4 iMatrix4  : TEXCOORD4;
	float4 iColor	 : COLOR0;
	float4 iUV		 : TEXCOORD5;
};

struct VS_OUTPUT_COLOR_ONLY
{
	float4 pos	  : POSITION;
	float2 uv0    : TEXCOORD0;
	float4 color  : COLOR0;
};


//-----------------------------------------------------------------------------
// TEXTURE SAMPLERS
//-----------------------------------------------------------------------------
sampler textureSamplerPoint = 
sampler_state
{
    Texture = <diffuseTexture>;
	MinFilter = Linear;
    MagFilter = Point;
	//AddressU = Clamp;
    //AddressV = Clamp;
};

sampler textureSamplerLinear = 
sampler_state
{
    Texture = <diffuseTexture>;
	MinFilter = Linear;
    MagFilter = Linear;
	//AddressU = Clamp;
    //AddressV = Clamp;
};



//-----------------------------------------------------------------------------
// VERTEX SHADERS
//-----------------------------------------------------------------------------

// Basic vertex shader
VS_OUTPUT_COLOR_ONLY v_shader( VS_INPUT_COLOR_ONLY IN )
{
	VS_OUTPUT_COLOR_ONLY OUT;

	OUT.pos = mul( IN.pos, cameraViewProjMatrix );
	OUT.uv0 = IN.uv0;

	OUT.color = materialColor;

	return OUT;
}

// Instancing vertex shader
VS_OUTPUT_COLOR_ONLY v_instance_shader( VS_INPUT_INSTANCE IN )
{
	VS_OUTPUT_COLOR_ONLY OUT;

	float4x4 mInstanceMatrix = float4x4(IN.iMatrix1,IN.iMatrix2,IN.iMatrix3,IN.iMatrix4);

	float2 uv;

	if( IN.vUVIndex == 0 )
		uv = IN.iUV.xy;

	else if( IN.vUVIndex == 1 )
		uv = IN.iUV.zy;

	else if( IN.vUVIndex == 2 )
		uv = IN.iUV.xw;
	
	else
		uv = IN.iUV.zw;

	OUT.pos = mul( IN.vPos, mInstanceMatrix );
	OUT.uv0 = uv;

	OUT.color = IN.iColor;

	return OUT;
}

//-----------------------------------------------------------------------------
// PIXEL SHADERS
//-----------------------------------------------------------------------------

// Point texture filter pixel shader
float4 p_shader_point( PS_INPUT IN ) : COLOR
{
	return tex2D( textureSamplerPoint, IN.uv0 ) * IN.color;
}

// Linear texture filter pixel shader
float4 p_shader_linear( PS_INPUT IN ) : COLOR
{
	return tex2D( textureSamplerLinear, IN.uv0 ) * IN.color;
}

// Rect texture filter pixel shader
float4 p_shader_rect( PS_INPUT IN ) : COLOR
{
	return IN.color;
}



//-----------------------------------------------------------------------------
// Effects
//-----------------------------------------------------------------------------

technique pointFilter
{
	pass Pass0
	{
		VertexShader = compile vs_3_0 v_shader();
		PixelShader  = compile ps_3_0 p_shader_point();
	}
}

technique linearFilter
{
	pass Pass0
	{
		VertexShader = compile vs_3_0 v_shader();
		PixelShader  = compile ps_3_0 p_shader_linear();
	}
}

technique solidRect
{
	pass Pass0
	{
		VertexShader = compile vs_3_0 v_shader();
		PixelShader  = compile ps_3_0 p_shader_rect();
	}
}

technique instance
{
	pass Pass0
	{
		VertexShader = compile vs_3_0 v_instance_shader();
		PixelShader  = compile ps_3_0 p_shader_linear();
	}
}


