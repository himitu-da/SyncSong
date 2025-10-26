#pragma once
#include <Siv3D.hpp>
#include "Config.hpp"
#include "GameTypes.hpp"

namespace RenderUtils
{
	// �O���f�[�V�����e�N�X�`������
	Texture CreateGradientTexture(const ColorF& topColor, const ColorF& bottomColor, int32 height);

	// �~�����b�V������
	Mesh CreateCylinderMesh(double radius, double height, int32 quality = 24);

	// 3D�V�[�������ݒ�
	void Setup3DScene();

	// 3D�V�[���`��
	void Render3DScene(
		const MSRenderTexture& renderTexture,
		DebugCamera3D& camera,
		const Mesh& cylinderMesh,
		const Texture& gradientTexture,
		const Array<SphereState>& spheres,
		double rotationAngle,
		const DragState& dragState);

	// ��ʂւ̕`��
	void RenderToScreen(const MSRenderTexture& renderTexture);
}