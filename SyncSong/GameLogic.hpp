#pragma once
#include <Siv3D.hpp>
#include "GameTypes.hpp"

namespace GameLogic
{
	// 球をクリックしたかチェック（3Dレイキャストを使用）
	Optional<int32> CheckSphereClick(const Vec2& mousePos, const Array<SphereState>& spheres,
		const DebugCamera3D& camera, const Mat4x4& transform);

	// スナップできる球を検索
	Optional<int32> FindSnapTarget(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform);

	// ドラッグ&ドロップ処理
	void ProcessDragAndDrop(Array<SphereState>& spheres, DragState& dragState, 
		const DebugCamera3D& camera, const Mat4x4& transform, const Array<Vec3>& gridPositions);

	// 回転処理
	void ProcessRotation(double& rotationAngle, bool isAutoRotationEnabled, bool isDragging);

	// デバッグ用：スナップ候補を取得
	Array<int32> GetSnapCandidates(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform);
}