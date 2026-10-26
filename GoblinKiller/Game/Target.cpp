#include "stdafx.h"
#include "Target.h"

bool Target::Start() {
	m_modelRender.Init("Assets/modelData/target.tkm");
	m_modelRender.SetPosition(m_position);
	m_modelRender.Update();
	return true;
}

void Target::Update() {
	m_modelRender.Update();
}

void Target::Render(RenderContext& rc) {
	m_modelRender.Draw(rc);
}

bool Target::IntersectRay(const Vector3& origin, const Vector3& dir, float& dist)const {
	//レイと弾の交差判定(dirは長さ１であること)
	Vector3 center = m_position;
	center.y += m_centerHeight;

	Vector3 toCenter = center - origin;
	float t = toCenter.Dot(dir);
	if (t < 0.0f) {
		return false;
	}
	float d2 = toCenter.LengthSq() - t * t;
	if (d2 > m_radius * m_radius) {
		return false;
	}
	dist = t - sqrtf(m_radius * m_radius - d2);
	if (dist < 0.0f) {
		dist = 0.0f;
	}
	return true;
}

void Target::TakeDamage(int damage) {
	m_hp -= damage;
	if (m_hp <= 0) {
		DeleteGO(this);
	}
}