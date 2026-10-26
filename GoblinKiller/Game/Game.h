#pragma once

#include "Level3DRender/LevelRender.h"

class Player;

class Game : public IGameObject
{
public:
	Game() {}
	~Game();

	bool Start()override;
	void Update()override;

	void Render(RenderContext& rc);


private:
	Player* m_player = nullptr;

	ModelRender m_stageRender;
	PhysicsStaticObject m_stagePhysics;
};

