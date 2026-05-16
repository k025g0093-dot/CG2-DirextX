#include "Sphere.h"

constexpr auto pi = 3.14159265355f;//πの宣言



void UpdateSphere(VertexData* vertexData) {
	uint32_t kSubdivision = 16; // 分割数
	const float kLonEvery = (pi * 2.0f) / float(kSubdivision); // 経度(横)の角度
	const float kLatEvery = pi / float(kSubdivision);          // 緯度(縦)の角度

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		// 現在の緯度と、次のマスの緯度
		float lat = -pi / 2.0f + (kLatEvery * latIndex);
		float nextLat = lat + kLatEvery;
		float v = float(latIndex) / float(kSubdivision);
		float nextV = float(latIndex + 1) / float(kSubdivision);

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;

			// 現在の経度と、次のマスの経度
			float lon = lonIndex * kLonEvery;
			float nextLon = lon + kLonEvery;
			float u = float(lonIndex) / float(kSubdivision);
			float nextU = float(lonIndex + 1) / float(kSubdivision);

			// 1枚目の三角形 (左下 -> 左上 -> 右下)
// --- 1枚目の三角形 ---
			vertexData[start + 0].position = { cosf(lat) * cosf(lon), sinf(lat), cosf(lat) * sinf(lon), 1.0f };
			vertexData[start + 0].texcoord = { u, 1.0f - v };
			vertexData[start + 0].normal = { vertexData[start + 0].position.x, vertexData[start + 0].position.y, vertexData[start + 0].position.z }; // 追加

			vertexData[start + 1].position = { cosf(nextLat) * cosf(lon), sinf(nextLat), cosf(nextLat) * sinf(lon), 1.0f };
			vertexData[start + 1].texcoord = { u, 1.0f - nextV };
			vertexData[start + 1].normal = { vertexData[start + 1].position.x, vertexData[start + 1].position.y, vertexData[start + 1].position.z }; // 追加

			vertexData[start + 2].position = { cosf(lat) * cosf(nextLon), sinf(lat), cosf(lat) * sinf(nextLon), 1.0f };
			vertexData[start + 2].texcoord = { nextU, 1.0f - v };
			vertexData[start + 2].normal = { vertexData[start + 2].position.x, vertexData[start + 2].position.y, vertexData[start + 2].position.z }; // 追加

			// --- 2枚目の三角形 ---
			vertexData[start + 3].position = { cosf(nextLat) * cosf(lon), sinf(nextLat), cosf(nextLat) * sinf(lon), 1.0f };
			vertexData[start + 3].texcoord = { u, 1.0f - nextV };
			vertexData[start + 3].normal = { vertexData[start + 3].position.x, vertexData[start + 3].position.y, vertexData[start + 3].position.z }; // 追加

			vertexData[start + 4].position = { cosf(nextLat) * cosf(nextLon), sinf(nextLat), cosf(nextLat) * sinf(nextLon), 1.0f };
			vertexData[start + 4].texcoord = { nextU, 1.0f - nextV };
			vertexData[start + 4].normal = { vertexData[start + 4].position.x, vertexData[start + 4].position.y, vertexData[start + 4].position.z }; // 追加

			vertexData[start + 5].position = { cosf(lat) * cosf(nextLon), sinf(lat), cosf(lat) * sinf(nextLon), 1.0f };
			vertexData[start + 5].texcoord = { nextU, 1.0f - v };
			vertexData[start + 5].normal = { vertexData[start + 5].position.x, vertexData[start + 5].position.y, vertexData[start + 5].position.z }; // 追加

		}
	}
}