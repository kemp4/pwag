#pragma once

#include <random>
#include "Game.h"
#include <src/Objects/Enemy.h>
#include <src/Objects/Particle/ParticleSystem.h>

class ExampleGame :
	public Game
{
public:
	ExampleGame(GLFWwindow* window);
	~ExampleGame();
	virtual void init();
	static int points ;
protected:

	virtual void processInput();
	virtual  void update();
	void initSkybox();
	void loadTextures() const;
    void initRoad(GLuint x);
	void loadModels() const;
	void spawnPlayer();
	SimpleObject * player;
    const float PLAYER_SPEED=25;
    const float ROAD_SIZE=5.6;
	const float CAR_SIZE_X=1.8;
	const float CAR_SIZE_Z=0.8;

    void manageEnemies();

    clock_t nextEnemySpawnTime;

	void spawnEnemy();

    void checkCollisions();

	bool checkCollision(Enemy *enemy);

};

