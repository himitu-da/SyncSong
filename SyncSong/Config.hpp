#pragma once
#include <Siv3D.hpp>

namespace Config
{
	// �E�B���h�E�ݒ�
	constexpr Size WindowSize{ 800, 600 };
	const ColorF BackgroundColor{ 0.8, 0.9, 1.0 };

	// �~���ݒ�
	constexpr double CylinderRadius = 2.0;
	constexpr double CylinderHeight = 6.0;

	// �O���f�[�V�����ݒ�
	constexpr int32 GradientHeight = 256;
	const ColorF TopColor{ 0.4, 0.2, 0.8 };
	const ColorF BottomColor{ 0.8, 0.4, 0.6 };

	// ��]�ݒ�
	constexpr double RotationSpeedDeg = 15.0;
	constexpr double MouseRotationFactor = -0.3;

	// �O���b�h�ݒ�
	constexpr int32 GridUDiv = 20;
	constexpr int32 GridVDiv = 8;
	constexpr double GridMargin = 1.0;

	// ���̐ݒ�
	constexpr double SphereRadius = 0.1;
	constexpr double SnapDistance = 0.2;

	// �h���b�O�ݒ�
	constexpr double DragPlaneX = 3.0;
}