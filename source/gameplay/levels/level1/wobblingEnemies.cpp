//--------------------------------------------------------------------------------
// wobblingEnemies.hpp
//--------------------------------------------------------------------------------
// Provides a script for the first wobbling enemies on level 1
//--------------------------------------------------------------------------------
#include "wobblingEnemies.hpp"

#include "data/sprites/wobbling-enemy.hpp"
#include "graphics/StillImagePointer.hpp"
#include "graphics/PalettePointer.hpp"

#include "math/fixedmath.hpp"
#include "gameplay/GameScene.hpp"

// Declaration
static StillImageAllocator image EWRAM_BSS(data::sprites::wobbling_enemy.png.tiles, SpriteSize::s16x16_4bpp);
static SinglePaletteAllocator palette EWRAM_BSS(data::sprites::wobbling_enemy.png.palette);

constexpr s32f16 Acceleration = 1.0 / 128;

void wobblingEnemyCommon(Enemy& enemy)
{
    enemy.vel.x = -1.5;
    enemy.size = vec2<s32f16>(16, 16);
    enemy.sprSize = SpriteSize::s16x16_4bpp;
    enemy.imagePtr = StillImagePointer(image);
    enemy.palPtr = SinglePalettePointer(palette);
    enemy.health = 3;
}

void shootTowardsPlayer(Enemy& enemy, GameScene& gameScene);

void wobblingEnemyUp(Enemy& enemy, GameScene& gameScene)
{
    wobblingEnemyCommon(enemy);

    enemy.pos = vec2<s32f16>(SCREEN_WIDTH + 16, 44);
    HANDLE_TERM(enemy.waitForFrames(40));
    shootTowardsPlayer(enemy, gameScene);
    enemy.acc.y = -Acceleration;
}

void wobblingEnemyDown(Enemy& enemy, GameScene& gameScene)
{
    wobblingEnemyCommon(enemy);

    enemy.pos = vec2<s32f16>(SCREEN_WIDTH + 16, SCREEN_HEIGHT - 44);
    HANDLE_TERM(enemy.waitForFrames(40));
    shootTowardsPlayer(enemy, gameScene);
    enemy.acc.y = Acceleration;
}

constexpr s16f7 WobblingProjectileSpeed = 3;

constexpr s32f<10> Cos15 = 0.9659258262890682867497431997289;
constexpr s32f<10> Sin15 = 0.25881904510252076234889883762405;

void shootTowardsPlayer(Enemy& enemy, GameScene& gameScene)
{
    // get the difference and normalize it
    auto diff = gameScene.player.pos - enemy.pos;
    auto distsq = vec2<s32f8>(diff).lensq();
    auto recdist = reciprocal(sqrt(distsq)).with_exp<8>();
    diff *= recdist;

    auto diffp = vec2<s32f16>(diff.x * Cos15 + diff.y * Sin15, -diff.x * Sin15 + diff.y * Cos15);
    auto diffm = vec2<s32f16>(diff.x * Cos15 - diff.y * Sin15, diff.x * Sin15 + diff.y * Cos15);

    for (auto vec : { diff, diffp, diffm })
    {
        auto vel = vec * WobblingProjectileSpeed;
        gameScene.enemyProjectiles.add(vec2<s16f7>(enemy.pos), vec2<s16f7>(vel), vec2<s16f7>(2, 2), 0);
    }
}
