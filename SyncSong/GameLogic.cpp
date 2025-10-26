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

			// 取り付けられている球は回転変換を適用、取り外された球はそのまま
			if (spheres[i].isAttached)
			{
				worldPos = transform.transformPoint(spheres[i].position);
			}
			else
			{
				worldPos = spheres[i].position;
			}

			// x < 0 の球は相互作用しない
			if (worldPos.x < 0)
			{
				continue;
			}

			const Sphere sphere{ worldPos, Config::SphereRadius };

			// レイと球の交差判定
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

			// 回転変換された円筒上の球の世界座標を取得
			Vec3 sphereWorldPos = transform.transformPoint(spheres[i].position);

			// x < 0 の球はスナップしない
			if (sphereWorldPos.x < 0)
			{
				continue;
			}

			// プレイヤー-ドラッグ球の直線から円筒上の球への距離を計算
			const double lineDistance = GeometryUtils::CalculatePointToLineDistance(sphereWorldPos, playerPos, draggedPos);
			
			if (lineDistance < Config::SnapDistance)
			{
				return i;
			}
		}
		return none;
	}

	// スナップ可能な球のインデックスを取得（デバッグ表示用）
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

		// 球のドラッグ&ドロップ処理
		if (MouseL.down())
		{
			if (!dragState.isDragging)
			{
				// 球をクリックしたかチェック
				const auto clickedSphere = CheckSphereClick(mousePos, spheres, camera, transform);
				if (clickedSphere && spheres[*clickedSphere].isYellow)
				{
					dragState.isDragging = true;
					dragState.draggedSphereIndex = *clickedSphere;

					// 元の球の位置（変換後）を取得
					Vec3 originalSpherePos;
					if (spheres[*clickedSphere].isAttached)
					{
						originalSpherePos = transform.transformPoint(spheres[*clickedSphere].position);
					}
					else
					{
						originalSpherePos = spheres[*clickedSphere].position;
					}

					// プレイヤーと球を結ぶ直線とx=3平面の交点を計算
					const auto intersection = GeometryUtils::GetLinePlaneIntersection(playerPos, originalSpherePos, Config::DragPlaneX);
					if (intersection)
					{
						spheres[*clickedSphere].position = *intersection;
					}
					else
					{
						// 交点が計算できない場合は既存の方法を使用
						spheres[*clickedSphere].position.x = Config::DragPlaneX;
					}

					dragState.initialDragPosition = spheres[*clickedSphere].position;
					dragState.lastMouseWorldPos = GeometryUtils::GetMouseWorldPosition(mousePos, camera, 5.0, true);

					// 取り外し: 元の位置に灰色の球を作成
					if (spheres[*clickedSphere].isAttached)
					{
						spheres[*clickedSphere].isAttached = false;
						// 新しい灰色の球を元の位置（回転変換前）に追加
						spheres.emplace_back(gridPositions[*clickedSphere], true, false, *clickedSphere);
					}
				}
			}
		}

		// ドラッグ中の処理
		if (dragState.isDragging && MouseL.pressed())
		{
			const Vec3 currentMouseWorldPos = GeometryUtils::GetMouseWorldPosition(mousePos, camera, 5.0, true);
			const Vec3 delta = currentMouseWorldPos - dragState.lastMouseWorldPos;

			// ドラッグ中の球の位置を更新（x座標は固定）
			Vec3& spherePos = spheres[dragState.draggedSphereIndex].position;
			spherePos.y += delta.y;
			spherePos.z += delta.z;
			spherePos.x = Config::DragPlaneX; // x座標を固定

			dragState.lastMouseWorldPos = currentMouseWorldPos;
		}

		// ドロップ処理
		if (dragState.isDragging && MouseL.up())
		{
			// スナップターゲットを検索
			const auto snapTarget = FindSnapTarget(
				spheres[dragState.draggedSphereIndex].position,
				spheres,
				dragState.draggedSphereIndex,
				playerPos,
				transform
			);

			if (snapTarget)
			{
				// スナップ: 灰色の球を黄色に変更し、ドラッグしていた球を削除
				spheres[*snapTarget].isYellow = true;
				spheres.erase(spheres.begin() + dragState.draggedSphereIndex);
			}

			dragState.isDragging = false;
			dragState.draggedSphereIndex = -1;
		}

		// デバッグ用：ドラッグ中はスナップ候補を更新
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
		// 自動回転
		if (isAutoRotationEnabled)
		{
			rotationAngle += (Scene::DeltaTime() * Math::ToRadians(Config::RotationSpeedDeg));
		}

		// マウスによる回転（ドラッグ中でない場合のみ）
		if (MouseL.pressed() && !isDragging)
		{
			rotationAngle += (Cursor::Delta().y * Config::MouseRotationFactor * Math::Pi / 180.0);
		}
	}
}