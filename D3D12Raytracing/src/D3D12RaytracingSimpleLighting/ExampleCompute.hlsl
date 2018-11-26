

struct Bone {
	float4x4 pose;
};

StructuredBuffer<Bone> boneBuffer;
ByteAddressBuffer vertexBuffer_POS;
ByteAddressBuffer vertexBuffer_NOR;
ByteAddressBuffer vertexBuffer_WEI;
ByteAddressBuffer vertexBuffer_BON;

RWByteAddressBuffer streamoutBuffer_POS;
RWByteAddressBuffer streamoutBuffer_NOR;
RWByteAddressBuffer streamoutBuffer_PRE; // previous frame skinned pos
// code from https://turanszkij.wordpress.com/2017/09/09/skinning-in-compute-shader/

inline void Skinning(inout float4 pos, inout float4 nor, in float4 inBon, in float4 inWei) {
	float4 p = 0, pp = 0;
	float3 n = 0;
	float4x4 m;
	float3x3 m3;
	float weightedSum = 0;
	[loop]
	for (uint i = 0; ((i < 4) && (weightedSum < 1.0f)); ++i) {
		m = boneBuffer[(uint)inBon[i]].pose;
		m3 = (float3x3)m;
		p += mul(float4(pos.xyz, 1.0f), m) * inWei[i];
		n += mul(nor.xyz, m3) * inWei[i];
		weightedSum += inWei[i];
	}

	bool w = any(inWei);
	pos.xyz = w ? p.xyz : pos.xyz;
	nor.xyz = w ? n : nor.xyz;
}

[numthreads(256, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	const uint fetchAddress = threadID.x * 16;
	uint4 pos_u = vertexBuffer_POS.Load4(fetchAddress);
	uint4 nor_u = vertexBuffer_NOR.Load4(fetchAddress);
	uint4 wei_u = vertexBuffer_WEI.Load4(fetchAddress);
	uint4 bon_u = vertexBuffer_BON.Load4(fetchAddress);
	float4 pos = asfloat(pos_u);
	float4 nor = asfloat(nor_u);
	float4 wei = asfloat(wei_u);
	float4 bon = asfloat(bon_u);
	Skinning(pos, nor, bon, wei);
	pos_u = asuint(pos);
	nor_u = asuint(nor);
	streamoutBuffer_PRE.Store4(fetchAddress, streamoutBuffer_POS.Load4(fetchAddress));
	// write out skinned props:
	streamoutBuffer_POS.Store4(fetchAddress, pos_u);
	streamoutBuffer_NOR.Store4(fetchAddress, nor_u);
}
