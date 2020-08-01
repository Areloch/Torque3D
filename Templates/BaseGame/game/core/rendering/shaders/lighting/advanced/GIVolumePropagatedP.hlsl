//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "../../shaderModelAutoGen.hlsl"
#include "../../torque.hlsl"
#include "../../lighting.hlsl"

struct Conn
{
   float4 position : TORQUE_POSITION;
   float2 uv0      : TEXCOORD0;
   float3 wsEyeRay : TEXCOORD1;
};

TORQUE_UNIFORM_SAMPLER3D(lpvData, 0);
TORQUE_UNIFORM_SAMPLER2D(deferredBuffer, 1);

#ifdef USE_SSAO_MASK
TORQUE_UNIFORM_SAMPLER2D(ssaoMask, 2);
#endif

uniform float3 eyePosWorld;
uniform float3 volumeStart;
uniform float3 volumeSize;

float4 main( Conn IN ) : TORQUE_TARGET0
{ 
   float4 prepassSample = TORQUE_DEFERRED_UNCONDITION( deferredBuffer, IN.uv0 );
   float3 normal = prepassSample.rgb;
   float depth = prepassSample.a;

   // Use eye ray to get ws pos
   float4 worldPos = float4(eyePosWorld + IN.wsEyeRay * depth, 1.0f);

   float3 volume_position = (worldPos.xyz - volumeStart) / volumeSize;
   if ( volume_position.x < 0 || volume_position.x > 1 || 
        volume_position.y < 0 || volume_position.y > 1 || 
        volume_position.z < 0 || volume_position.z > 1 )
   {
        return float4(0.0, 0.0, 0.0, 0.0); 
   }

   float4 color = TORQUE_TEX3D(lpvData, volume_position);

#ifdef USE_SSAO_MASK
   float ao = 1.0 - TORQUE_TEX2D( ssaoMask, IN.uv0 ).r;
   color = color * ao;
#endif

   //return float4(color.rgb, 0.0);
   return float4(1, 1, 1, 1);
}
