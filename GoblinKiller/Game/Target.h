#pragma once
class Target:public IGameObject
{
public:
	Target() {}
	~Target() {}

	bool Start() override;
	void Update()override;
	void Render(RenderContext& rc)override;

	void SetPosition(const Vector3& pos) { m_position = pos; }
	//レイが当たるか。当たったら、始点からの距離をdistに入れる
	bool IntersectRay(const Vector3& origin, const Vector3& dir, float& dist)const;
	void TakeDamage(int damage);

private:
	ModelRender m_modelRender;
	Vector3 m_position = Vector3::Zero;
	int m_hp = 1000;
	float m_radius = 50.0f;
	float m_centerHeight = 80.0f;
};

