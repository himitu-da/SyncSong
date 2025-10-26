#include "GameLogic.hpp"
#include "Config.hpp"
#include "GeometryUtils.hpp"

namespace GameLogic
{
	Optional<int32> CheckSphereClick(const Vec2& mousePos, const Array<SphereState>& spheres,
		const DebugCamera3D& camera, const Mat4x4& transform)
	{
		const Ray ray = camera.screenToRay(mousePos);

		for (int32 i = 0; i < spheres.size(); ++i)
		{
			Vec3 worldPos;

			// ���t�����Ă��鋅�͉�]�ϊ���K�p�A���O���ꂽ���͂��̂܂�
			if (spheres[i].isAttached)
			{
				worldPos = transform.transformPoint(spheres[i].position);
			}
			else
			{
				worldPos = spheres[i].position;
			}

			// x < 0 �̋��͑��ݍ�p���Ȃ�
			if (worldPos.x < 0)
			{
				continue;
			}

			const Sphere sphere{ worldPos, Config::SphereRadius };

			// ���C�Ƌ��̌�������
			const auto intersection = ray.intersects(sphere);
			if (intersection)
			{
				return i;
			}
		}
		return none;
	}

	Optional<int32> FindSnapTarget(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform)
	{
		for (int32 i = 0; i < spheres.size(); ++i)
		{
			if (i == excludeIndex || !spheres[i].isAttached || spheres[i].isYellow)
				continue;

			// ��]�ϊ����ꂽ�~����̋��̐��E���W���擾
			Vec3 sphereWorldPos = transform.transformPoint(spheres[i].position);

			// x < 0 �̋��̓X�i�b�v���Ȃ�
			if (sphereWorldPos.x < 0)
			{
				continue;
			}

			// �v���C���[-�h���b�O���̒�������~����̋��ւ̋������v�Z
			const double lineDistance = GeometryUtils::CalculatePointToLineDistance(sphereWorldPos, playerPos, draggedPos);
			
			if (lineDistance < Config::SnapDistance)
			{
				return i;
			}
		}
		return none;
	}

	// �X�i�b�v�\�ȋ��̃C���f�b�N�X���擾�i�f�o�b�O�\���p�j
	Array<int32> GetSnapCandidates(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
		const Vec3& playerPos, const Mat4x4& transform)
	{
		Array<int32> candidates;
		
		for (int32 i = 0; i < spheres.size(); ++i)
		{
			if (i == excludeIndex || !spheres[i].isAttached || spheres[i].isYellow)
				continue;

			Vec3 sphereWorldPos = transform.transformPoint(spheres[i].position);

			if (sphereWorldPos.x < 0)
			{
				continue;
			}

			const double lineDistance = GeometryUtils::CalculatePointToLineDistance(sphereWorldPos, playerPos, draggedPos);
			
			if (lineDistance < Config::SnapDistance)
			{
				candidates.push_back(i);
			}
		}
		return candidates;
	}

	void ProcessDragAndDrop(Array<SphereState>& spheres, DragState& dragState, 
		const DebugCamera3D& camera, const Mat4x4& transform, const Array<Vec3>& gridPositions)
	{
		const Vec2 mousePos = Cursor::Pos();
		const Vec3 playerPos = camera.getEyePosition();

		// ���̃h���b�O&�h���b�v����
		if (MouseL.down())
		{
			if (!dragState.isDragging)
			{
				// �����N���b�N�������`�F�b�N
				const auto clickedSphere = CheckSphereClick(mousePos, spheres, camera, transform);
				if (clickedSphere && spheres[*clickedSphere].isYellow)
				{
					dragState.isDragging = true;
					dragState.draggedSphereIndex = *clickedSphere;

					// ���̋��̈ʒu�i�ϊ���j���擾
					Vec3 originalSpherePos;
					if (spheres[*clickedSphere].isAttached)
					{
						originalSpherePos = transform.transformPoint(spheres[*clickedSphere].position);
					}
					else
					{
						originalSpherePos = spheres[*clickedSphere].position;
					}

					// �v���C���[�Ƌ������Ԓ�����x=3���ʂ̌�_���v�Z
					const auto intersection = GeometryUtils::GetLinePlaneIntersection(playerPos, originalSpherePos, Config::DragPlaneX);
					if (intersection)
					{
						spheres[*clickedSphere].position = *intersection;
					}
					else
					{
						// ��_���v�Z�ł��Ȃ��ꍇ�͊����̕��@���g�p
						spheres[*clickedSphere].position.x = Config::DragPlaneX;
					}

					dragState.initialDragPosition = spheres[*clickedSphere].position;
					dragState.lastMouseWorldPos = GeometryUtils::GetMouseWorldPosition(mousePos, camera, 5.0, true);

					// ���O��: ���̈ʒu�ɊD�F�̋����쐬
					if (spheres[*clickedSphere].isAttached)
					{
						spheres[*clickedSphere].isAttached = false;
						// �V�����D�F�̋������̈ʒu�i��]�ϊ��O�j�ɒǉ�
						spheres.emplace_back(gridPositions[*clickedSphere], true, false, *clickedSphere);
					}
				}
			}
		}

		// �h���b�O���̏���
		if (dragState.isDragging && MouseL.pressed())
		{
			const Vec3 currentMouseWorldPos = GeometryUtils::GetMouseWorldPosition(mousePos, camera, 5.0, true);
			const Vec3 delta = currentMouseWorldPos - dragState.lastMouseWorldPos;

			// �h���b�O���̋��̈ʒu���X�V�ix���W�͌Œ�j
			Vec3& spherePos = spheres[dragState.draggedSphereIndex].position;
			spherePos.y += delta.y;
			spherePos.z += delta.z;
			spherePos.x = Config::DragPlaneX; // x���W���Œ�

			dragState.lastMouseWorldPos = currentMouseWorldPos;
		}

		// �h���b�v����
		if (dragState.isDragging && MouseL.up())
		{
			// �X�i�b�v�^�[�Q�b�g������
			const auto snapTarget = FindSnapTarget(
				spheres[dragState.draggedSphereIndex].position,
				spheres,
				dragState.draggedSphereIndex,
				playerPos,
				transform
			);

			if (snapTarget)
			{
				// �X�i�b�v: �D�F�̋������F�ɕύX���A�h���b�O���Ă��������폜
				spheres[*snapTarget].isYellow = true;
				spheres.erase(spheres.begin() + dragState.draggedSphereIndex);
			}

			dragState.isDragging = false;
			dragState.draggedSphereIndex = -1;
		}

		// �f�o�b�O�p�F�h���b�O���̓X�i�b�v�����X�V
		if (dragState.isDragging)
		{
			dragState.snapCandidates = GetSnapCandidates(
				spheres[dragState.draggedSphereIndex].position,
				spheres,
				dragState.draggedSphereIndex,
				playerPos,
				transform
			);
		}
		else
		{
			dragState.snapCandidates.clear();
		}
	}

	void ProcessRotation(double& rotationAngle, bool isAutoRotationEnabled, bool isDragging)
	{
		// ������]
		if (isAutoRotationEnabled)
		{
			rotationAngle += (Scene::DeltaTime() * Math::ToRadians(Config::RotationSpeedDeg));
		}

		// �}�E�X�ɂ���]�i�h���b�O���łȂ��ꍇ�̂݁j
		if (MouseL.pressed() && !isDragging)
		{
			rotationAngle += (Cursor::Delta().y * Config::MouseRotationFactor * Math::Pi / 180.0);
		}
	}
}