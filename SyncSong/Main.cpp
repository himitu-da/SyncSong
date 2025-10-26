#include <Siv3D.hpp>
#include "Config.hpp"
#include "GameTypes.hpp"
#include "RenderUtils.hpp"
#include "GeometryUtils.hpp"
#include "GameLogic.hpp"

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
	const Texture gradientTexture = RenderUtils::CreateGradientTexture(
		Config::TopColor,
		Config::BottomColor,
		Config::GradientHeight
	);
	const Mesh cylinderMesh = RenderUtils::CreateCylinderMesh(
		Config::CylinderRadius,
		Config::CylinderHeight
	);

	// 球の状態を初期化（すべて黄色で取り付けられた状態）
	const Array<Vec3> gridPositions = GeometryUtils::GenerateCylinderGridPositions(
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
		const Mat4x4 transform = Mat4x4::RotateZ(rotationAngle);

		// カメラ更新（ドラッグ中でない場合のみ）
		if (!dragState.isDragging)
		{
			camera.update(2.0);
		}

		// 回転処理
		GameLogic::ProcessRotation(rotationAngle, isAutoRotationEnabled, dragState.isDragging);

		// ドラッグ&ドロップ処理
		GameLogic::ProcessDragAndDrop(spheres, dragState, camera, transform, gridPositions);

		// 描画
		RenderUtils::Render3DScene(renderTexture, camera, cylinderMesh, gradientTexture, spheres, rotationAngle, dragState);
		RenderUtils::RenderToScreen(renderTexture);

		// UI
		if (SimpleGUI::Button(isAutoRotationEnabled ? U"ON" : U"OFF", Vec2{ Scene::Width() - 100, Scene::Height() - 40 }))
		{
			isAutoRotationEnabled = (not isAutoRotationEnabled);
		}
	}
}