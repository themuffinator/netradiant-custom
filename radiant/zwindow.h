/*
   Copyright (C) 1999-2006 Id Software, Inc. and contributors.
   For a list of contributors, see the accompanying CONTRIBUTORS file.

   This file is part of GtkRadiant.

   GtkRadiant is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   GtkRadiant is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GtkRadiant; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#pragma once

#include "map.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "gtkutil/cursor.h"
#include "view.h"

class QWidget;
class ZGLWidget;
class SelectionSystemWindowObserver;

class ZWnd
{
	friend class ZGLWidget;

	QWidget* m_gl_widget;
	DeferredDraw m_deferredDraw;
	SelectionSystemWindowObserver* m_window_observer{};

	Vector3 m_origin{ 0, 20, 46 };
	float m_selectionOriginX = 0.0f;
	FreezePointer m_originDrag;
	bool m_originDragging = false;
	bool m_selectionDragging = false;

	void syncOriginXY();
	float screenToWorldZ( int y ) const;
	void drawGrid( float w, float h );
	void drawBrushes( float xcam );
	void drawCameraIcon() const;
	void setScale( float scale );
	void updateProjection();
	void updateModelview();
	void beginSelectionDrag();
	void endSelectionDrag();

public:
	ZWnd();
	~ZWnd();

	QWidget* GetWidget(){
		return m_gl_widget;
	}

	void queueDraw();
	void SetOriginZ( float z );
	void ZoomIn();
	void ZoomOut();
	float OriginZ() const {
		return m_origin[2];
	}
	void beginOriginDrag();
	void endOriginDrag();

	void Z_Draw();

	bool m_drawRequired{};
	int m_nWidth{};
	int m_nHeight{};
	float m_scale{ 1.0f };
	Matrix4 m_projection;
	Matrix4 m_modelview;
	View m_view;
};
