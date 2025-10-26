#pragma once
#include <Siv3D.hpp>

// 球の状態を管理する構造体
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

// ドラッグ状態を管理する構造体
struct DragState
{
	bool isDragging = false;
	int32 draggedSphereIndex = -1;
	Vec3 dragOffset;
	Vec3 lastMouseWorldPos;
	Vec3 initialDragPosition;
	Array<int32> snapCandidates; // スナップ候補のインデックス（デバッグ表示用）
};