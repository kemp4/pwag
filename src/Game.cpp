#include "baseHeader.h"

#include "Game.h"




std::map<std::string, Shader> ResourceManager::shaders;
std::map<std::string, Texture> ResourceManager::textures;
std::map<std::string, VaoObject> ResourceManager::vaos;
Game::Game(GLFWwindow* window)
{
	this->window = window;

	glfwMakeContextCurrent(window);

}


Game::~Game()
{
	ResourceManager::deleteAssets();
}

void Game::setKey(int keyIndex)
{
	keys[keyIndex] = GL_TRUE;
}

void Game::clearKey(int keyIndex)
{
	keys[keyIndex] = GL_FALSE;
}

void Game::changeCameraDirection(GLfloat x, GLfloat y)
{
	camera.rotateCamera(x, y);
}

void Game::startMainGameLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		update();
		glfwPollEvents();
		render();
		glfwSwapBuffers(window);
	}
	exitGame();
}

void Game::init()
{
	
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to setup GLEW\n");
		exit(1);
	}
	mainWindow = new NkWindow(window,&options);
	mainWindow->init();
	ambientStrength =0.3f;
	oldTime = std::clock();
	for (unsigned char &key : keys) {
        key = GL_FALSE;
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

}


void Game::update()
{
	mainWindow->update();

	updateObjects();

}

void Game::render()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthMask(GL_FALSE);

    skyboxShader->use();
    GLint viewLoc = glGetUniformLocation(skyboxShader->getID(), "view");
    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(skyboxView));

    GLuint skyboxVAO = ResourceManager::getVAO("skyboxvao")->getID();
    glBindVertexArray(skyboxVAO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthMask(GL_TRUE);

    Shader* shader = ResourceManager::GetShader("shader1");
    shader->use();
    glm::vec3 ambientColorI = options.ambientColor*options.ambientStrength/255.f;

    glUniform3fv(lcLoc, 1, glm::value_ptr(ambientColorI));
    glm::vec3 cameraPos = camera.getPos();
    glUniform3fv(vpLoc, 1, glm::value_ptr(cameraPos));

    GLint modelLoc2 = glGetUniformLocation(shader->getID(), "view");
    glUniformMatrix4fv(modelLoc2, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));

    renderObjects();
	mainWindow->render();

}

void Game::processInput()
{
	static bool clicked=false;
	if (keys[GLFW_KEY_W]) {
		camera.moveForward(deltaTime);
	}else if (keys[GLFW_KEY_S]) {
		camera.moveForward(-deltaTime);
	}
	if (keys[GLFW_KEY_A]) {
		camera.moveRight(deltaTime);
	}else if (keys[GLFW_KEY_D])
		camera.moveRight(-deltaTime);
	if (keys[GLFW_KEY_SPACE]) {
		camera.moveUp(-deltaTime);
	}
	else if (keys[GLFW_KEY_Z])
		camera.moveUp(+deltaTime);
	if (keys[GLFW_KEY_E]) {
		delCube = true;
	}
	if (keys[GLFW_KEY_X]) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	if (keys[GLFW_KEY_C]) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (keys[GLFW_KEY_P]) {
		if (!clicked) {
			if (options.paused) {
				options.paused = (GL_FALSE);
			}
			else {
				options.paused = (GL_TRUE);
			}
			clicked = true;
		}
	}
	else {
		clicked = false;
	}
}

void Game::exitGame()
{
	glfwTerminate();
}





void Game::renderObjects()
{
	for (auto &gameObject : gameObjects)
	{
	    drawGameObjectWithChildren(gameObject);

	}
}


void Game::deleteObjects()
{
	while (!gameObjects.empty())
		gameObjects.pop_back();
}

void Game::drawGameObjectWithChildren(AbstractGameObject *&gameObject) {
    gameObject->draw();

    for (auto &child : gameObject->getChildren())
    {
        drawGameObjectWithChildren(child);
    }
}

void Game::updateObjects() {
    if(options.paused) {
        for (auto &gameObject : gameObjects) {
            updateGameObjectWithChildren(gameObject);

        }
        options.points +=1;
    }
}

void Game::updateGameObjectWithChildren(AbstractGameObject *&gameObject) {
    gameObject->update();

    for (auto &child : gameObject->getChildren())
    {
        updateGameObjectWithChildren(child);
    }
}

