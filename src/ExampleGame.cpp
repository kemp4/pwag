#include <src/Objects/NonPlayerObject.h>
#include "baseHeader.h"
#include "ExampleGame.h"



ExampleGame::ExampleGame(GLFWwindow* window) : Game(window)
{

}

ExampleGame::~ExampleGame()
= default;

void ExampleGame::init()
{

    nextEnemySpawnTime = std::clock();

	Game::init();
	initSkybox();

	ResourceManager::loadShader("shader1VS.glsl", "shader1FS.glsl", "shader1");
	Shader* shader = ResourceManager::GetShader("shader1");
	shader->use();
	GLuint shaderId = shader->getID();
	GLint viewLoc = glGetUniformLocation(shaderId, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	GLint projLoc = glGetUniformLocation(shaderId, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	lcLoc = glGetUniformLocation(shaderId, "lightColor");
	lpLoc = glGetUniformLocation(shaderId, "lightPos");
	vpLoc = glGetUniformLocation(shaderId, "viewPos");

	lightColor = glm::vec3(255, 255, 255);

	loadTextures();


	loadModels();

	initRoad(6);
	spawnPlayer();
}


void ExampleGame::spawnPlayer() {
    player = new SimpleObject(glm::vec3(0, 0, 0),ResourceManager::GetShader("shader1"), ResourceManager::GetTexture("texture1"), ResourceManager::getVAO("player"));
    AbstractGameObject *particleSystem = new ParticleSystem(glm::vec3(-0.5, 0, -0.3), ResourceManager::GetShader("shader1"), ResourceManager::GetTexture("texture2"), ResourceManager::getVAO("particle"));
	AbstractGameObject *particleSystem2 = new ParticleSystem(glm::vec3(-0.5, 0, 0.3), ResourceManager::GetShader("shader1"), ResourceManager::GetTexture("texture2"), ResourceManager::getVAO("particle2"));
    player->addChildren(particleSystem);
	player->addChildren(particleSystem2);
    gameObjects.push_front((AbstractGameObject*)player);

}




void ExampleGame::loadModels() const {

	ResourceManager::loadVAOFromFile("floor.obj", "floor", true);
	ResourceManager::loadVAOFromFile("auto.obj", "player", true);
    ResourceManager::loadVAOFromFile("particle.obj", "particle", true);
	ResourceManager::loadVAOFromFile("particle2.obj", "particle2", true);

}

void ExampleGame::loadTextures() const {
	ResourceManager::LoadTexture("triangles.png", GL_TRUE, "texture1");
    ResourceManager::LoadTexture("particle.png", GL_TRUE, "texture2");
	ResourceManager::LoadTexture("road.png", GL_TRUE, "texture3");

}


void ExampleGame::update()
{
    if(options.paused) {
        Game::update();
        newTime = std::clock();
        deltaTime = (double) (newTime - oldTime) / (double) CLOCKS_PER_SEC;
        oldTime = std::clock();


        fps = 1.0f / deltaTime;

        processInput();
        camera.update();

        glm::vec3 testD = glm::normalize(camera.getPos());


        manageEnemies();
        checkCollisions();
    }
}

void ExampleGame::initSkybox()
{

	vector<std::string> faces;
	faces.emplace_back("right.jpg");
	faces.emplace_back("left.jpg");
	faces.emplace_back("top.jpg");
	faces.emplace_back("bottom.jpg");
	faces.emplace_back("back.jpg");
	faces.emplace_back("front.jpg");
	ResourceManager::loadShader("sbsVS.glsl", "sbsFS.glsl", "skyboxshader");
	cubemapTexture = ResourceManager::loadCubemap(faces);
	skyboxShader = ResourceManager::GetShader("skyboxshader");
	ResourceManager::loadVAO(skybox, 180, false, "skyboxvao");


	skyboxShader->use();

	GLint viewLoc = glGetUniformLocation(skyboxShader->getID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	GLint projLoc = glGetUniformLocation(skyboxShader->getID(), "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}


void ExampleGame::initRoad(GLuint x)
{
    Shader* shader = ResourceManager::GetShader("shader1");
    cubesGroundX = x;
    cubesGroundY = 1;
    for (GLuint i = 0; i < cubesGroundX; i++) {
        for (GLuint j = 0; j < cubesGroundY; j++) {
            AbstractGameObject * someCube = new NonPlayerObject(glm::vec3(i*6, 0, j*6),shader, ResourceManager::GetTexture("texture3"), ResourceManager::getVAO("floor"));
            gameObjects.push_front(someCube);
        }
    }
}

void ExampleGame::processInput() {
    Game::processInput();
    if (keys[GLFW_KEY_LEFT]) {
        if (player->getPosition().z-deltaTime*PLAYER_SPEED>-ROAD_SIZE/2) {
            player->move(glm::vec3(0, 0, -deltaTime*PLAYER_SPEED));
        }else{
            player->setPosition(glm::vec3(0, 0, -ROAD_SIZE/2));

        }
    } else
    if(keys[GLFW_KEY_RIGHT]){
        if (player->getPosition().z+deltaTime*PLAYER_SPEED<ROAD_SIZE/2) {
            player->move(glm::vec3(0, 0, deltaTime*PLAYER_SPEED));
        }else{
            player->setPosition(glm::vec3(0, 0, ROAD_SIZE/2));
        }
    }
}

void ExampleGame::manageEnemies() {
	if (std::clock()>nextEnemySpawnTime){
		spawnEnemy();
		nextEnemySpawnTime = std::clock()+CLOCKS_PER_SEC/10;
	}

}

void ExampleGame::spawnEnemy() {
	Shader* shader = ResourceManager::GetShader("shader1");
	float generatedRand = (float)(rand()%55)/10.f-2.75f;
	cout << generatedRand << "\n";
	AbstractGameObject * newEnemy = new Enemy(glm::vec3(5*6, 0, generatedRand),shader, ResourceManager::GetTexture("texture1"), ResourceManager::getVAO("player"));
	gameObjects.push_front(newEnemy);
}

void ExampleGame::checkCollisions() {

    list<AbstractGameObject *, std::allocator<AbstractGameObject *>>::iterator iterator;
    for (iterator = gameObjects.begin(); iterator != gameObjects.end(); ++iterator)
    {
        if (Enemy* enemy = dynamic_cast<Enemy*>(*iterator))
        {
            checkCollision(enemy);
            if(enemy->getPosition().x<=-6) {
                delete *iterator;
                iterator = gameObjects.erase(iterator);//todo
            }
        }
    }
}

bool ExampleGame::checkCollision(Enemy *enemy) {


    float dx = abs(enemy->getPosition().x - player->getPosition().x);
    float dy = abs(enemy->getPosition().z - player->getPosition().z);
    if ((dx < CAR_SIZE_X) && (dy < CAR_SIZE_Z)){
        cout << "GAME OVER\n" ;
        options.paused=false;
        options.gameOver = true;
    }
    return (dx < CAR_SIZE_X) && (dy < CAR_SIZE_Z);
}

