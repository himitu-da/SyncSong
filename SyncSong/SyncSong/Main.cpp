#include <Siv3D.hpp>

// 定数定義
namespace Config
{
	constexpr Size WindowSize{ 800, 600 };
	constexpr double CylinderRadius = 2.0;
	constexpr double CylinderHeight = 6.0;
	constexpr int32 GradientHeight = 256;
	constexpr double RotationSpeedDeg = 15.0;
	constexpr int32 GridUDiv = 20;
	constexpr int32 GridVDiv = 8;
	constexpr double GridMargin = 1.0;
	constexpr double MouseRotationFactor = -0.3;
	constexpr double SphereRadius = 0.1;
	constexpr double SnapDistance = 0.3;
	constexpr double SnapLineDistance = 0.5; // プレイヤー-球直線からの距離閾値
	constexpr double DragPlaneX = 3.0; // ドラッグ時の制約平面 x=3
	const ColorF TopColor{ 0.4, 0.2, 0.8 };
	const ColorF BottomColor{ 0.8, 0.4, 0.6 };
	const ColorF BackgroundColor{ 0.8, 0.9, 1.0 };
}

// 球の状態を管理する構造体
struct SphereState
{
	Vec3 position;
	bool isAttached;
	bool isYellow;
	int32 originalIndex; // 元の位置のインデックス（取り外された球用）

	SphereState(Vec3 pos, bool attached = true, bool yellow = true, int32 index = -1)
		: position(pos), isAttached(attached), isYellow(yellow), originalIndex(index) {
	}
};

// ドラッグ状態を管理する構造体
struct DragState
{
	bool isDragging = false;
	int32 draggedSphereIndex = -1;
	Vec3 dragOffset;
	Vec3 lastMouseWorldPos;
	Vec3 initialDragPosition; // ドラッグ開始時の位置
};

// グラデーションテクスチャ生成
Texture CreateGradientTexture(const ColorF& topColor, const ColorF& bottomColor, int32 height)
{
	Image gradientImage{ 1, static_cast<size_t>(height) };

	for (int32 y = 0; y < height; ++y)
	{
		const double t = y / static_cast<double>(height - 1);
		gradientImage[0][static_cast<size_t>(height - 1 - y)] = Color{ topColor.lerp(bottomColor, t) };
	}

	return Texture{ gradientImage };
}

// 円筒メッシュ生成
Mesh CreateCylinderMesh(double radius, double height, int32 quality = 24)
{
	MeshData cylinderMesh = MeshData::Cylinder(radius, height, quality);
	cylinderMesh.rotate(Quaternion::RotateX(90_deg));
	return Mesh{ cylinderMesh };
}

// 円筒の曲面上にグリッドを生成
Array<Vec3> GenerateCylinderGridPositions(double radius, double height, int32 u_div, int32 v_div, double margin)
{
	Array<Vec3> positions;
	if (u_div == 0 || v_div == 0)
	{
		return positions;
	}

	const double effectiveHeight = height - (margin * 2.0);
	if (effectiveHeight <= 0)
	{
		return positions;
	}

	for (int32 v = 0; v < v_div; ++v)
	{
		const double z = (static_cast<double>(v) / (v_div - 1) - 0.5) * effectiveHeight;
		for (int32 u = 0; u < u_div; ++u)
		{
			const double angle = (static_cast<double>(u) / u_div) * Math::TwoPi;
			const double x = radius * Math::Cos(angle);
			const double y = radius * Math::Sin(angle);
			positions.push_back({ x, y, z });
		}
	}
	return positions;
}

// 3D線と平面 x=planeX の交点を計算
Optional<Vec3> GetLinePlaneIntersection(const Vec3& lineStart, const Vec3& lineEnd, double planeX)
{
	const Vec3 direction = lineEnd - lineStart;

	// 線がx軸に平行な場合は交差しない
	if (Math::Abs(direction.x) < 1e-6)
	{
		return none;
	}

	// t = (planeX - lineStart.x) / direction.x
	const double t = (planeX - lineStart.x) / direction.x;

	// 交点を計算
	return lineStart + direction * t;
}

// 点と直線の距離を計算（無限直線版）
double CalculatePointToLineDistance(const Vec3& point, const Vec3& lineStart, const Vec3& lineEnd)
{
	const Vec3 lineVec = lineEnd - lineStart;
	const Vec3 pointVec = point - lineStart;

	const double lineLength = lineVec.length();
	if (lineLength < 1e-6)
	{
		return point.distanceFrom(lineStart);
	}

	const Vec3 lineDir = lineVec.normalized();
	const Vec3 cross = pointVec.cross(lineDir);

	return cross.length();
}

// 球をクリックしたかチェック（3Dレイキャストを使用）
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

// x=planeX の平面とレイの交点を計算
Optional<Vec3> GetRayPlaneIntersection(const Ray& ray, double planeX)
{
	const Vec3 origin = ray.getOrigin();
	const Vec3 direction = ray.getDirection();

	// レイの方向がx軸に平行な場合は交差しない
	if (Math::Abs(direction.x) < 1e-6)
	{
		return none;
	}

	// t = (planeX - origin.x) / direction.x
	const double t = (planeX - origin.x) / direction.x;

	// 交点がレイの後方にある場合は無効
	if (t < 0)
	{
		return none;
	}

	// 交点を計算
	return origin + direction * t;
}

// マウス位置に対応する3D位置を取得（カメラレイを使用）
Vec3 GetMouseWorldPosition(const Vec2& mousePos, const DebugCamera3D& camera, double distance = 5.0, bool constrainToPlane = false)
{
	const Ray ray = camera.screenToRay(mousePos);

	if (constrainToPlane)
	{
		// x=3の平面との交点を計算
		const auto intersection = GetRayPlaneIntersection(ray, Config::DragPlaneX);
		if (intersection)
		{
			return *intersection;
		}
	}

	// デフォルトの距離ベースの計算
	const Vec3 origin = ray.getOrigin();
	const Vec3 direction = ray.getDirection();
	return origin + direction * distance;
}

// スナップできる球を検索（改良版）
Optional<int32> FindSnapTarget(const Vec3& draggedPos, const Array<SphereState>& spheres, int32 excludeIndex,
	const Vec3& playerPos, const Mat4x4& transform)
{
	for (int32 i = 0; i < spheres.size(); ++i)
	{
		if (i == excludeIndex || !spheres[i].isAttached || spheres[i].isYellow)
			continue;

		Vec3 sphereWorldPos;
		if (spheres[i].isAttached)
		{
			sphereWorldPos = transform.transformPoint(spheres[i].position);
		}
		else
		{
			sphereWorldPos = spheres[i].position;
		}

		// x < 0 の球はスナップしない
		if (sphereWorldPos.x < 0)
		{
			continue;
		}

		// プレイヤー-ドラッグ球の直線からの距離をチェック
		const double lineDistance = CalculatePointToLineDistance(sphereWorldPos, playerPos, draggedPos);
		if (lineDistance > Config::SnapLineDistance)
		{
			continue;
		}

		// 距離チェック
		const double distance = draggedPos.distanceFrom(sphereWorldPos);
		if (distance < Config::SnapDistance)
		{
			return i;
		}
	}
	return none;
}

// 3Dシーン初期設定
void Setup3DScene()
{
	Graphics3D::SetGlobalAmbientColor(ColorF{ 0.4 });
	Graphics3D::SetSunDirection(Vec3{ -1, -1, 0.5 }.normalized());
}

// 3Dシーン描画
void Render3DScene(
	const MSRenderTexture& renderTexture,
	DebugCamera3D& camera,
	const Mesh& cylinderMesh,
	const Texture& gradientTexture,
	const Array<SphereState>& spheres,
	double rotationAngle,
	const DragState& dragState)
{
	const ScopedRenderTarget3D target{ renderTexture.clear(Scene::GetBackground()) };
	Graphics3D::SetCameraTransform(camera);
	Setup3DScene();

	const auto transform = Mat4x4::RotateZ(rotationAngle);
	cylinderMesh.draw(transform, gradientTexture);

	// 球を描画
	for (int32 i = 0; i < spheres.size(); ++i)
	{
		const auto& sphere = spheres[i];
		ColorF color = sphere.isYellow ? Palette::Yellow : Palette::Gray;

		// ドラッグ中の球は少し透明にする
		if (dragState.isDragging && dragState.draggedSphereIndex == i)
		{
			color.a = 0.7;
		}

		// 取り付けられている球は回転変換を適用、取り外された球はそのまま描画
		if (sphere.isAttached)
		{
			Sphere{ sphere.position, Config::SphereRadius }.draw(transform, color);
		}
		else
		{
			// 取り外された球は回転変換を適用しない
			Sphere{ sphere.position, Config::SphereRadius }.draw(color);
		}
	}
}

// 画面への描画
void RenderToScreen(const MSRenderTexture& renderTexture)
{
	Graphics3D::Flush();
	renderTexture.resolve();
	Shader::LinearToScreen(renderTexture);
}

void Main()
{
	// ウィンドウ初期化
	Window::Resize(Config::WindowSize);
	Scene::SetBackground(Config::BackgroundColor);

	// レンダーテクスチャ
	const MSRenderTexture renderTexture{
		Scene::Size(),
		TextureFormat::R8G8B8A8_Unorm_SRGB,
		HasDepth::Yes
	};

	// カメラ
	DebugCamera3D camera{ renderTexture.size(), 45_deg, Vec3{ 10, 0, 0 } };

	// リソース生成
	const Texture gradientTexture = CreateGradientTexture(
		Config::TopColor,
		Config::BottomColor,
		Config::GradientHeight
	);
	const Mesh cylinderMesh = CreateCylinderMesh(
		Config::CylinderRadius,
		Config::CylinderHeight
	);

	// 球の状態を初期化（すべて黄色で取り付けられた状態）
	const Array<Vec3> gridPositions = GenerateCylinderGridPositions(
		Config::CylinderRadius,
		Config::CylinderHeight,
		Config::GridUDiv,
		Config::GridVDiv,
		Config::GridMargin
	);

	Array<SphereState> spheres;
	for (int32 i = 0; i < gridPositions.size(); ++i)
	{
		spheres.emplace_back(gridPositions[i], true, true, i);
	}

	double rotationAngle = 0.0;
	bool isAutoRotationEnabled = false;
	DragState dragState;

	// メインループ
	while (System::Update())
	{
		const Vec2 mousePos = Cursor::Pos();
		const Mat4x4 transform = Mat4x4::RotateZ(rotationAngle);
		const Vec3 playerPos = camera.getEyePosition(); // プレイヤー位置

		// カメラ更新（ドラッグ中でない場合のみ）
		if (!dragState.isDragging)
		{
			camera.update(2.0);
		}

		// 自動回転
		if (isAutoRotationEnabled)
		{
			rotationAngle += (Scene::DeltaTime() * Math::ToRadians(Config::RotationSpeedDeg));
		}

		// マウスによる回転（ドラッグ中でない場合のみ）
		if (MouseL.pressed() && !dragState.isDragging)
		{
			rotationAngle += (Cursor::Delta().y * Config::MouseRotationFactor * Math::Pi / 180.0);
		}

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
					const auto intersection = GetLinePlaneIntersection(playerPos, originalSpherePos, Config::DragPlaneX);
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
					dragState.lastMouseWorldPos = GetMouseWorldPosition(mousePos, camera, 5.0, true);

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
			const Vec3 currentMouseWorldPos = GetMouseWorldPosition(mousePos, camera, 5.0, true);
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
			// スナップターゲットを検索（改良版）
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

		Render3DScene(renderTexture, camera, cylinderMesh, gradientTexture, spheres, rotationAngle, dragState);
		RenderToScreen(renderTexture);

		if (SimpleGUI::Button(isAutoRotationEnabled ? U"ON" : U"OFF", Vec2{ Scene::Width() - 100, Scene::Height() - 40 }))
		{
			isAutoRotationEnabled = (not isAutoRotationEnabled);
		}
	}
}
