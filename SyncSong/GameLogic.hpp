#pragma once
#include <Siv3D.hpp>
#include "GameTypes.hpp"

namespace GameLogic
{
	// �����N���b�N�������`�F�b�N�i3D���C�L���X�g���g�p�j
	Optional<int32> CheckSphereClick(const Vec2& mousePos, const Array<SphereState>& spheres,
		const DebugCamera3D& camera, const Mat4x4& transform);

	// �X�i�b�v�ł��鋅������
	Optional<int32> FindSnapTarget(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform);

	// �h���b�O&�h���b�v����
	void ProcessDragAndDrop(Array<SphereState>& spheres, DragState& dragState, 
		const DebugCamera3D& camera, const Mat4x4& transform, const Array<Vec3>& gridPositions);

	// ��]����
	void ProcessRotation(double& rotationAngle, bool isAutoRotationEnabled, bool isDragging);

	// �f�o�b�O�p�F�X�i�b�v�����擾
	Array<int32> GetSnapCandidates(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform);
}