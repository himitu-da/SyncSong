#include <Siv3D.hpp>
#include "Config.hpp"
#include "GameTypes.hpp"
#include "RenderUtils.hpp"
#include "GeometryUtils.hpp"
#include "GameLogic.hpp"

void Main()
{
	// �E�B���h�E������
	Window::Resize(Config::WindowSize);
	Scene::SetBackground(Config::BackgroundColor);

	// �����_�[�e�N�X�`��
	const MSRenderTexture renderTexture{
		Scene::Size(),
		TextureFormat::R8G8B8A8_Unorm_SRGB,
		HasDepth::Yes
	};

	// �J����
	DebugCamera3D camera{ renderTexture.size(), 45_deg, Vec3{ 10, 0, 0 } };

	// ���\�[�X����
	const Texture gradientTexture = RenderUtils::CreateGradientTexture(
		Config::TopColor,
		Config::BottomColor,
		Config::GradientHeight
	);
	const Mesh cylinderMesh = RenderUtils::CreateCylinderMesh(
		Config::CylinderRadius,
		Config::CylinderHeight
	);

	// ���̏�Ԃ��������i���ׂĉ��F�Ŏ��t����ꂽ��ԁj
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

	// ���C�����[�v
	while (System::Update())
	{
		const Mat4x4 transform = Mat4x4::RotateZ(rotationAngle);

		// �J�����X�V�i�h���b�O���łȂ��ꍇ�̂݁j
		if (!dragState.isDragging)
		{
			camera.update(2.0);
		}

		// ��]����
		GameLogic::ProcessRotation(rotationAngle, isAutoRotationEnabled, dragState.isDragging);

		// �h���b�O&�h���b�v����
		GameLogic::ProcessDragAndDrop(spheres, dragState, camera, transform, gridPositions);

		// �`��
		RenderUtils::Render3DScene(renderTexture, camera, cylinderMesh, gradientTexture, spheres, rotationAngle, dragState);
		RenderUtils::RenderToScreen(renderTexture);

		// UI
		if (SimpleGUI::Button(isAutoRotationEnabled ? U"ON" : U"OFF", Vec2{ Scene::Width() - 100, Scene::Height() - 40 }))
		{
			isAutoRotationEnabled = (not isAutoRotationEnabled);
		}
	}
}