blending 
{
    src = src_alpha 
    dst = inv_src_alpha 
}

#include "common.slh"

fragment_in
{
    float4 projectedPosition : COLOR0;

    float4 invWorldMatrix0 : TEXCOORD0;
    float4 invWorldMatrix1 : TEXCOORD1;
    float4 invWorldMatrix2 : TEXCOORD2;
    float4 instanceOpacity : TEXCOORD3;
    #if VERTEX_FOG
        [lowp] half4 varFog : TEXCOORD4;
    #endif
};

fragment_out
{
    float4 color : SV_TARGET0;
};

uniform sampler2D albedo;
uniform sampler2D dynamicTextureSrc0;

[auto][a] property float4x4 invProjMatrix;
[auto][a] property float4x4 invViewMatrix;

[auto][a] property float2 viewportSize;
[auto][a] property float2 viewportOffset;
[auto][a] property float2 renderTargetSize;

fragment_out fp_main(fragment_in input)
{
    fragment_out output;

    float opacity = input.instanceOpacity.x;

    float2 xyNDC = input.projectedPosition.xy / input.projectedPosition.w;

    #if DEPTH_TARGET_IS_FRAMEBUFFER
        float2 texCoord = xyNDC * ndcToUvMapping.xy + ndcToUvMapping.zw;
    #else
        float2 texCoord = xyNDC * float2(0.5, -0.5) + float2(0.5, 0.5);
    #endif

    // viewport correction
    texCoord = (viewportOffset + texCoord * viewportSize) / renderTargetSize;

    float depthSample = tex2D(dynamicTextureSrc0, texCoord).r;
    float depth = (depthSample - ndcToZMappingOffset) / ndcToZMappingScale;
    float4 intermediatePosition = mul(float4(xyNDC, depth, 1.0), invProjMatrix);
    float3 positionVS = intermediatePosition.xyz / intermediatePosition.w;
    float3 positionWS = mul(float4(positionVS.xyz, 1.0), invViewMatrix).xyz;

    float4x4 inverseWorldMatrix = float4x4(
        float4(input.invWorldMatrix0.x,  input.invWorldMatrix1.x,  input.invWorldMatrix2.x, 0.0),
        float4(input.invWorldMatrix0.y,  input.invWorldMatrix1.y,  input.invWorldMatrix2.y, 0.0),
        float4(input.invWorldMatrix0.z,  input.invWorldMatrix1.z,  input.invWorldMatrix2.z, 0.0),
        float4(input.invWorldMatrix0.w,  input.invWorldMatrix1.w,  input.invWorldMatrix2.w, 1.0)
    );

    float3 positionMS = mul(float4(positionWS, 1.0), inverseWorldMatrix).xyz;

    opacity *= step(abs(positionMS.x), 0.5) * step(abs(positionMS.y), 0.5) * step(abs(positionMS.z), 0.5);


    float2 decalTexCoord = positionMS.xy + 0.5;
    float4 albedoSample = tex2D(albedo, decalTexCoord);
    albedoSample.a *= opacity;
    output.color = albedoSample;

    // Fog for decal box. it is ok for small boxes.
    #if VERTEX_FOG
        float varFogAmoung = float(input.varFog.a);
        float3 varFogColor  = float3(input.varFog.rgb);

        output.color.rgb = lerp(output.color.rgb, varFogColor, varFogAmoung);
    #endif

    return output;
}