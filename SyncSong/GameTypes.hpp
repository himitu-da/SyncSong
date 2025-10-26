#pragma once
#include <Siv3D.hpp>

// ���̏�Ԃ��Ǘ�����\����
struct SphereState
{
	Vec3 position;
	bool isAttached;
	bool isYellow;
	int32 originalIndex;

	SphereState(Vec3 pos, bool attached = true, bool yellow = true, int32 index = -1)
		: position(pos), isAttached(attached), isYellow(yellow), originalIndex(index) 
	{
	}
};

// �h���b�O��Ԃ��Ǘ�����\����
struct DragState
{
	bool isDragging = false;
	int32 draggedSphereIndex = -1;
	Vec3 dragOffset;
	Vec3 lastMouseWorldPos;
	Vec3 initialDragPosition;
	Array<int32> snapCandidates; // �X�i�b�v���̃C���f�b�N�X�i�f�o�b�O�\���p�j
};