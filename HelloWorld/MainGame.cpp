#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

struct GameState {
	int score = 0;
};

GameState gameState;

enum GameObjectType {
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

void HandlePlayerControls();
void UpdateFan();
void UpdateTools();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::StartAudioLoop("music");
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8");
	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140, 217 }, 0, "fan");
	Play::GetGameObject(id_fan).velocity = { 0, 3 };
	Play::GetGameObject(id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();
	HandlePlayerControls();
	UpdateFan();
	UpdateTools();
	Play::PresentDrawingBuffer();
	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void HandlePlayerControls() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	// climb up at constant velocity
	if (Play::KeyDown(VK_UP)) {
		obj_agent8.velocity = { 0, -4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}
	// accelerate downwards
	else if (Play::KeyDown(VK_DOWN)) {
		obj_agent8.acceleration = { 0, 1 };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}
	// reduce speed and stop
	else {
		Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
		obj_agent8.velocity *= 0.5f;
		obj_agent8.acceleration = { 0,0 };
	}
	Play::UpdateGameObject(obj_agent8);

	// keep player within bounds
	if (Play::IsLeavingDisplayArea(obj_agent8)) {
		obj_agent8.pos = obj_agent8.oldPos;
	}

	// draw web line
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);
	Play::DrawObjectRotated(obj_agent8);
}

void UpdateFan() {
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN);

	// 1 in 50 chance of spawning tool
	if (Play::RandomRoll(50) == 50) {
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);

		// either set tool to fly straight, or diagonally up or down
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		// 1 in 2 chance of spawning a spanner as the tool
		if (Play::RandomRoll(2) == 1) {
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");
	}
	// animate fan to move within screen bounds by reversing direction
	Play::UpdateGameObject(obj_fan);
	if (Play::IsLeavingDisplayArea(obj_fan)) {
		obj_fan.pos = obj_fan.oldPos;
		obj_fan.velocity.y *= -1;
	}
	Play::DrawObject(obj_fan);
}

void UpdateTools() {
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);

	for (int id : vTools) {
		GameObject& obj_tool = Play::GetGameObject(id);

		// kill player if hit
		if (Play::IsColliding(obj_tool, obj_agent8)) {
			Play::StopAudioLoop("music");
			Play::PlayAudio("die");
			obj_agent8.pos = { -100, -100 };
		}
		Play::UpdateGameObject(obj_tool);

		// rebound tool
		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL)) {
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}
		Play::DrawObjectRotated(obj_tool);

		if (!Play::IsVisible(obj_tool))
			Play::DestroyGameObject(id);
	}
}

