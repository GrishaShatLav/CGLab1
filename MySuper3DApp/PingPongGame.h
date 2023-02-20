#pragma once
#include "Game.h"
#include "GameObject.h"
#include "SquareRenderComponent.h"

class PingPongGame : public Game {
private:
	PingPongGame(LPCWSTR name, int screenWidth, int screenHeight, bool windowed);

	void Update() override;

public:
	std::shared_ptr<GameObject> leftPlayer;
	std::shared_ptr<GameObject> rightPlayer;
	std::shared_ptr<GameObject> ball;

	static void CreateInstance(LPCWSTR name, int screenWidth, int screenHeight, bool windowed);

	void Run() override;
	void ConfigureGameObjects();
};
