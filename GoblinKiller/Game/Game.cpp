#include "stdafx.h"
#include "Game.h"
#include "Player.h"
#include "Target.h"


Game::~Game() {
	//Gameが消えるときに、自分が作ったものも消す
	if (m_player != nullptr) {
		DeleteGO(m_player);
		m_player = nullptr;
	}
}


bool Game::Start()
{

	m_stageRender.Init("Assets/modelData/kariyuka.tkm");
	m_stageRender.SetPosition(0.0f, 0.0f, 0.0f);
	m_stageRender.Update();

	m_stagePhysics.CreateFromModel(
		m_stageRender.GetModel(),
		m_stageRender.GetModel().GetWorldMatrix()
	);



	m_player = NewGO<Player>(0, "player");

	Target* target = NewGO<Target>(0, "target");
	target->SetPosition(Vector3(0.0f, 0.0f, 500.0f));

	return true;
}

void Game::Update()
{
	////当たり判定を可視化する。
	//PhysicsWorld::GetInstance()->EnableDrawDebugWireFrame();


	m_stageRender.Update();
}

void Game::Render(RenderContext& rc) {
	m_stageRender.Draw(rc);
}