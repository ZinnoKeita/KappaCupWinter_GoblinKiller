#pragma once
class Bullet: public IGameObject
{
public:
	Bullet() {}
	~Bullet() {}

	bool Start()override;
	void Update()override;
	void Render(RenderContext& rc)override;

	//発射位置と着弾位置を設定する
	void Setup(const Vector3& start, const Vector3& end);

private:
	ModelRender m_modelRender;
	Vector3 m_position = Vector3::Zero;//今の位置
	Vector3 m_dir = Vector3::AxisZ;//飛ぶ方向（長さ１）
	float m_remain = 0.0f;//着弾までの残りの距離
};

