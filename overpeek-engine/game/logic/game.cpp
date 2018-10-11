#include "game.h"

#include "../creatures/player.h"
#include "../world/region.h"
#include "../logic/inventory.h"

#include <windows.h>

graphics::Window *Game::m_window;
graphics::Shader *Game::m_shader;
graphics::Renderer *Game::m_renderer;
logic::GameLoop *Game::m_loop;
glm::ivec2 Game::m_hover_tile;
Inventory *m_inventory;

Region *Game::m_region[RENDER_DST * 2][RENDER_DST * 2];
Player *Game::m_player;

float Game::lastRegionX = 0;
float Game::lastRegionY = 0;

int Game::hitCooldown = 513709;
bool holdingI = false;

void Game::init(graphics::Shader *shader, graphics::Window * window, logic::GameLoop *loop) {
	m_shader = shader; m_window = window; m_loop = loop;

	//Initializing
	system(("mkdir \"" + SAVE_PATH + WORLD_NAME + "\\regions\"").c_str());
	logic::Noise::seed(0);//tools::Clock::getMicroseconds());
	tools::Random::seed(0);

	m_renderer = new graphics::Renderer("arial.ttf");

	audio::AudioManager::init();
	audio::AudioManager::loadAudio("recourses/hit.wav", 0);
	audio::AudioManager::loadAudio("recourses/swing.wav", 1);

	graphics::TextureManager n;
	n.loadTexture("recourses/atlas.png", GL_RGBA, 0);
	Database::init();

	m_inventory = new Inventory(m_shader, m_window);

	//Instantiating
	for (int x = 0; x < RENDER_DST; x++)
	{
		for (int y = 0; y < RENDER_DST; y++)
		{
			m_region[x][y] = new Region(x, y);
		}
	}
	m_player = new Player(0.0, 0.0, m_inventory);


	m_shader->enable();
	m_shader->SetUniformMat4("ml_matrix", glm::mat4(1.0f));
}

void Game::render() {
	m_renderer->clear();
	for (int x = 0; x < RENDER_DST; x++)
	{
		for (int y = 0; y < RENDER_DST; y++)
		{
			if (m_region[x][y] != nullptr) m_region[x][y]->submitToRenderer(m_renderer, -m_player->x, -m_player->y);
		}
	}
	m_player->submitToRenderer(m_renderer, -m_player->x, -m_player->y);
	
	m_inventory->render(m_renderer);

	m_renderer->flush();
}

void Game::update() {
	//Player movement
	float playerSpeed = 0.01;
	if (m_window->getKey(GLFW_KEY_LEFT_SHIFT)) playerSpeed = 0.02;
	if (m_window->getKey(GLFW_KEY_S)) { m_player->acc_y = playerSpeed; }
	if (m_window->getKey(GLFW_KEY_D)) { m_player->acc_x = playerSpeed; }
	if (m_window->getKey(GLFW_KEY_W)) { m_player->acc_y = -playerSpeed; }
	if (m_window->getKey(GLFW_KEY_A)) { m_player->acc_x = -playerSpeed; }

	//m_enemy->hit();
	m_player->update();

	processNewArea();
	for (int x = 0; x < RENDER_DST; x++)
	{
		for (int y = 0; y < RENDER_DST; y++)
		{
			if (m_region[x][y]) m_region[x][y]->update();
		}
	}

	m_inventory->update();

	lastRegionX = m_player->getRegionX();
	lastRegionY = m_player->getRegionY();
}

void Game::rapidUpdate() {
	for (int x = 0; x < RENDER_DST; x++) {
		for (int y = 0; y < RENDER_DST; y++) {
			if (m_region[x][y] == nullptr) {
				m_region[x][y] = new Region(x + m_player->getRegionX(), y + m_player->getRegionY());
			}
		}
	}
}

void Game::keyPress(int key) {
	//Player Hitting
	if (m_player && key == GLFW_KEY_E) { m_player->hit(); return; }

	//Inventory
	if (key == GLFW_KEY_R) { m_inventory->visible = !m_inventory->visible; return; }
	if (key == GLFW_KEY_ESCAPE) { m_inventory->visible = false; return; }

	//Inventory slot selecting
	if (key == GLFW_KEY_1) { m_inventory->selectedSlot = 0; return; }
	else if (key == GLFW_KEY_2) { m_inventory->selectedSlot = 1; return; }
	else if (key == GLFW_KEY_3) { m_inventory->selectedSlot = 2; return; }
	else if (key == GLFW_KEY_4) { m_inventory->selectedSlot = 3; return; }
	else if (key == GLFW_KEY_5) { m_inventory->selectedSlot = 4; return; }
}

void Game::buttonPress(int button) {}

void Game::scroll(double y) {
	//Inventory slot scrolling
	if (y < 0) m_inventory->selectedSlot++;
	if (y > 0) m_inventory->selectedSlot--;
}

void Game::close() {
	for (int x = 0; x < RENDER_DST; x++) {
		for (int y = 0; y < RENDER_DST; y++) {
			if (m_region[x][y]) m_region[x][y]->saveTiles();
		}
	}
}





//long long startTime = tools::Clock::getMicroseconds();
//std::cout << "ms: " << (tools::Clock::getMicroseconds() - startTime) / 1000.0 << std::endl;
/*

NOTHING GOOD TO EDIT AFTER THIS
ALL JUST SOME GETTERS AND STUFF

*/





void Game::processNewArea() {
	if (m_player->getRegionX() > lastRegionX) {
		for (int y = 0; y < RENDER_DST; y++) delete m_region[0][y];
		for (int x = 0; x < RENDER_DST - 1; x++) {
			for (int y = 0; y < RENDER_DST; y++) m_region[x][y] = m_region[x + 1][y];
		}
		for (int y = 0; y < RENDER_DST; y++) m_region[RENDER_DST - 1][y] = nullptr;
	}
	else if (m_player->getRegionX() < lastRegionX) {
		for (int y = 0; y < RENDER_DST; y++) delete m_region[RENDER_DST - 1][y];
		for (int x = RENDER_DST - 1; x > 0; x--) {
			for (int y = 0; y < RENDER_DST; y++) m_region[x][y] = m_region[x - 1][y];
		}
		for (int y = 0; y < RENDER_DST; y++) m_region[0][y] = nullptr;
	}
	if (m_player->getRegionY() > lastRegionY) {
		for (int x = 0; x < RENDER_DST; x++) delete m_region[x][0];
		for (int y = 0; y < RENDER_DST - 1; y++) {
			for (int x = 0; x < RENDER_DST; x++) m_region[x][y] = m_region[x][y + 1];
		}
		for (int x = 0; x < RENDER_DST; x++) m_region[x][RENDER_DST - 1] = nullptr;
	}
	else if (m_player->getRegionY() < lastRegionY) {
		for (int x = 0; x < RENDER_DST; x++) delete m_region[x][RENDER_DST - 1];
		for (int y = RENDER_DST - 1; y > 0; y--) {
			for (int x = 0; x < RENDER_DST; x++) m_region[x][y] = m_region[x][y - 1];
		}
		for (int x = 0; x < RENDER_DST; x++) m_region[x][0] = nullptr;
	}
}

int Game::screenToWorldX(float x) {
	if (x / TILE_SIZE + m_player->x >= 0) return x / TILE_SIZE + m_player->x;
	else return x / TILE_SIZE - 1 + m_player->x;
}
int Game::screenToWorldY(float y) {
	if (y / TILE_SIZE + m_player->y >= 0) return y / TILE_SIZE + m_player->y;
	else return y / TILE_SIZE - 1 + m_player->y;
}

Region* Game::getRegion(float x, float y) {
	x = floor(x); y = floor(y);
	if (m_player) {
		int cursorRegionX = floor((x / (float)REGION_SIZE) + 0.5);
		int cursorRegionY = floor((y / (float)REGION_SIZE) + 0.5);
		int playerRegionX = floor((m_player->x / (float)REGION_SIZE) + 0.5);
		int playerRegionY = floor((m_player->y / (float)REGION_SIZE) + 0.5);

		int finalRegionX = cursorRegionX - playerRegionX + ceil(RENDER_DST / 2);
		int finalRegionY = cursorRegionY - playerRegionY + ceil(RENDER_DST / 2);

		if (finalRegionX < 0 || finalRegionX > RENDER_DST) return nullptr;
		if (finalRegionY < 0 || finalRegionY > RENDER_DST) return nullptr;
		if (m_region[finalRegionX][finalRegionY] != nullptr) return m_region[finalRegionX][finalRegionY];
	}
	return nullptr;
}

Tile* Game::getTile(float x, float y) {
	x = floor(x); y = floor(y);
	Region *regionAt = getRegion(x, y);
	if (regionAt) {
		int finalX = floor(x) + ceil(REGION_SIZE / 2.0) - regionAt->getX();
		int finalY = floor(y) + ceil(REGION_SIZE / 2.0) - regionAt->getY();
		if (finalX < 0 || finalX > REGION_SIZE || finalY < 0 || finalY > REGION_SIZE) return nullptr;
		return regionAt->getTile(finalX, finalY);
	}
	return nullptr;
}

bool Game::trySetTileObject(float x, float y, int id) {
	Tile *tile = getTile(x, y);
	if (!tile) return false;
	if (tile->getObjectId() != 5) return false;
	tile->setObjectId(id);
	return true;
}

bool Game::trySetTileObject(Tile *tile, int id) {
	if (!tile) return false;
	if (tile->getObjectId() != 5) return false;
	tile->setObjectId(id);
	return true;
}