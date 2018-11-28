//--------------------------------------------------------------------------------------
// File: DX11 Framework.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

// Texture Variables
Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;

	float4 SpecularMtrl;
	float4 SpecularLight;
	float SpecularPower;
	float3 EyePosW;

	float4 AmbientMaterial;
	float4 AmbientLight;
	float4 DiffuseMtrl;
	float4 DiffuseLight;
	float3 LightVecW;
}

//--------------------------------------------------------------------------------------

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
	float3 Norm : NORMAL;
	float3 PosW : POSITION;
    float4 Color : COLOR0;
	float2 Tex : TEXCOORD0;
};

//------------------------------------------------------------------------------------
// Vertex Shader - Implements Gouraud Shading using Diffuse lighting and Specular
//------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 NormalL : NORMAL, float2 Tex : TEXCOORD0)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.PosW = Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);	

	output.Tex = Tex;

	float3 normalW = mul(float4(NormalL, 0.0f), World).xyz;
	output.Norm = normalize(normalW);

	return output;
}



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	// Convert from local space to world space 
	// W component of vector is 0 as vectors cannot be translated
	float3 normalW = normalize(input.Norm);
	float3 lightVec = normalize(LightVecW);

	// Compute ambient
	float3 ambient = AmbientMaterial.xyz * AmbientLight.xyz;

	//compute specular
	float3 toEye = normalize(EyePosW - input.PosW.xyz);
	float3 r = reflect(-lightVec, normalW);
	float specularAmount = pow(max(dot(r, toEye), 0.0f), SpecularPower);
	float3 specular = specularAmount * (SpecularMtrl * SpecularLight).rgb;

	// Compute Diffuse
	float diffuseAmount = max(dot(lightVec, normalW), 0.0f);
	float3 diffuse = (diffuseAmount * (DiffuseMtrl * DiffuseLight).rgb);

	// Compute Texture Colour
	float4 textureColour = txDiffuse.Sample(samLinear, input.Tex);

	input.Color.rgb = (textureColour.rgb * (diffuse + ambient)) + specular;
	input.Color.a = DiffuseMtrl.a;
	
    return input.Color;
}
