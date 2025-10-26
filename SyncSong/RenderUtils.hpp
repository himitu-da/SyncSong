#pragma once
#include <Siv3D.hpp>
#include "Config.hpp"
#include "GameTypes.hpp"

namespace RenderUtils
{
	// グラデーションテクスチャ生成
	Texture CreateGradientTexture(const ColorF& topColor, const ColorF& bottomColor, int32 height);

	// 円筒メッシュ生成
	Mesh CreateCylinderMesh(double radius, double height, int32 quality = 24);

	// 3Dシーン初期設定
	void Setup3DScene();

	// 3Dシーン描画
	void Render3DScene(
		const MSRenderTexture& renderTexture,
		DebugCamera3D& camera,
		const Mesh& cylinderMesh,
		const Texture& gradientTexture,
		const Array<SphereState>& spheres,
		double rotationAngle,
		const DragState& dragState);

	// 画面への描画
	void RenderToScreen(const MSRenderTexture& renderTexture);
}