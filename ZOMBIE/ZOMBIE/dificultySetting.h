#pragma once

struct DifficultySettings
{
	enum class Level { Easy, Medium, Hard };

	Level level{ Level::Medium };

	// Player
	int playerHealth{ 100 };
	int playerAmmo{ 100 };
	float playerFireRate{ 0.15f };

	// Lighting
	int ambientLight{ 80 };
	float playerLightRadius{ 280.f };
	float torchLightRadius{ 140.f };

	// Enemies per room type Normal
	int zombieCountNormal{ 5 };
	int minionCountNormal{ 4 };
	int zombieCountTrap{ 5 };
	int minionCountTrap{ 5 };
	int zombieCountTreasure{ 1 };
	int minionCountTreasure{ 3 };
	int zombieCountStart{ 1 };
	int minionCountStart{ 0 };

	// Enemy stats
	int zombieDamage{ 10 };
	int zombieHealth{ 100 };
	int minionDamage{ 5 };
	int minionHealth{ 50 };
	int bossHealth{ 500 };

	// Traps
	int trapCountNormal{ 3 };
	int trapCountTrap{ 8 };
	int trapCountBoss{ 10 };
	int trapCountDefault{ 1 };

	// Pickups
	int healthPickupMin{ 25 };
	int healthPickupMax{ 50 };
	int ammoPickupMin{ 20 };
	int ammoPickupMax{ 30 };

	static DifficultySettings Easy()
	{
		DifficultySettings s;
		s.level = Level::Easy;
		s.playerHealth = 150;
		s.playerAmmo = 200;
		s.playerFireRate = 0.10f;
		s.ambientLight = 120;
		s.playerLightRadius = 350.f;
		s.torchLightRadius = 180.f;
		s.zombieCountNormal = 1;
		s.minionCountNormal = 1;
		s.zombieCountTrap = 1;
		s.minionCountTrap = 2;
		s.zombieCountTreasure = 1;
		s.minionCountTreasure = 1;
		s.zombieCountStart = 1;
		s.minionCountStart = 0;
		s.zombieDamage = 8;
		s.zombieHealth = 80;
		s.minionDamage = 3;
		s.minionHealth = 30;
		s.bossHealth = 300;
		s.trapCountNormal = 1;
		s.trapCountTrap = 4;
		s.trapCountBoss = 5;
		s.trapCountDefault = 0;
		s.healthPickupMin = 40;
		s.healthPickupMax = 70;
		s.ammoPickupMin = 30;
		s.ammoPickupMax = 40;
		return s;
	}

	static DifficultySettings Medium()
	{
		DifficultySettings s;
		s.level = Level::Medium;
		return s;
	}

	static DifficultySettings Hard()
	{
		DifficultySettings s;
		s.level = Level::Hard;
		s.playerHealth = 75;
		s.playerAmmo = 50;
		s.playerFireRate = 0.25f;
		s.ambientLight = 40;
		s.playerLightRadius = 200.f;
		s.torchLightRadius = 100.f;
		s.zombieCountNormal = 10;
		s.minionCountNormal = 7;
		s.zombieCountTrap = 10;
		s.minionCountTrap = 10;
		s.zombieCountTreasure = 5;
		s.minionCountTreasure = 5;
		s.zombieCountStart = 2;
		s.minionCountStart = 2;
		s.zombieDamage = 20;
		s.zombieHealth = 150;
		s.minionDamage = 10;
		s.minionHealth = 100;
		s.bossHealth = 800;
		s.trapCountNormal = 6;
		s.trapCountTrap = 12;
		s.trapCountBoss = 15;
		s.trapCountDefault = 3;
		s.healthPickupMin = 10;
		s.healthPickupMax = 25;
		s.ammoPickupMin = 10;
		s.ammoPickupMax = 15;
		return s;
	}
};