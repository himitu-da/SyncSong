#pragma once
#include <Siv3D.hpp>

namespace Config
{
	// ウィンドウ設定
	constexpr Size WindowSize{ 800, 600 };
	const ColorF BackgroundColor{ 0.8, 0.9, 1.0 };

	// 円筒設定
	constexpr double CylinderRadius = 2.0;
	constexpr double CylinderHeight = 6.0;

	// グラデーション設定
	constexpr int32 GradientHeight = 256;
	const ColorF TopColor{ 0.4, 0.2, 0.8 };
	const ColorF BottomColor{ 0.8, 0.4, 0.6 };

	// 回転設定
	constexpr double RotationSpeedDeg = 15.0;
	constexpr double MouseRotationFactor = -0.3;

	// グリッド設定
	constexpr int32 GridUDiv = 20;
	constexpr int32 GridVDiv = 8;
	constexpr double GridMargin = 1.0;

	// 球の設定
	constexpr double SphereRadius = 0.1;
	constexpr double SnapDistance = 0.2;

	// ドラッグ設定
	constexpr double DragPlaneX = 3.0;
}