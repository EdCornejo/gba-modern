//--------------------------------------------------------------------------------
// PlayerProjectiles.cpp
//--------------------------------------------------------------------------------
// The class that manages the player projectiles
//--------------------------------------------------------------------------------
#include "PlayerProjectiles.hpp"

#include "GameScene.hpp"
#include <algorithm>

#include "text/mGBADebugging.hpp"

#include "data/sprites/player-projectiles.hpp"

constexpr int PlayerProjectilePriority = 7;

static SinglePaletteAllocator palette EWRAM_BSS(data::sprites::player_projectiles.png.palette);

void PlayerProjectiles::init()
{
    // Set everything to zero
    memset32(projectiles, 0, sizeof(projectiles)/sizeof(u32));
    numProjectiles = 0;

    // Initialize the graphics pointers
    tilePtr = ObjectTilePointer(SpriteSize::s8x8_4bpp);
    tilePtr.setData(data::sprites::player_projectiles.png.tiles);

    palPtr = SinglePalettePointer(palette);
}

void PlayerProjectiles::update()
{
    numProjectiles = updateProjectiles(numProjectiles, projectiles);
    sortProjectiles(numProjectiles, projectiles);

    for (auto& enemy : gameScene().enemies)
    {
        auto epos = vec2<s16f7>(enemy.pos);
        auto eradius = s16f7(enemy.size.x)/2;

        for (u32 i = 0; i < numProjectiles; i++)
        {
            if (projectiles[i].tileId == graphics::NoTile) continue;

            auto diffsq = (epos - projectiles[i].pos).lensq();
            auto sumr = eradius + projectiles[i].size.x/2;
            if (diffsq < sumr*sumr)
            {
                if (enemy.damage())
                {
                    gameScene().explosions.addSmallExplosion(epos);
                    gameScene().enemies.remove(&enemy);
                }
                projectiles[i].tileId = graphics::NoTile;
                break;
            }
        }
    }

    numProjectiles = std::remove_if(projectiles, projectiles+numProjectiles,
        [](const Projectile& proj) { return proj.tileId == graphics::NoTile; }) - projectiles;
}

void PlayerProjectiles::pushGraphics()
{
    // Add a sprite for each projectile on screen
    for (u32 i = 0; i < numProjectiles; i++)
    {
        auto dp = vec2<int>(projectiles[i].pos) - vec2(4, 4);
        graphics::oam.pushRegular(dp, SpriteSize::s8x8_4bpp, tilePtr.getTileId(),
            palPtr.getPalette(), 1, PlayerProjectilePriority);
    }
}

GameScene& PlayerProjectiles::gameScene()
{
    // Don't worry, I know what I'm doing
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    return *reinterpret_cast<GameScene*>(
        reinterpret_cast<std::byte*>(this) - offsetof(GameScene, playerProjectiles));
#pragma GCC diagnostic pop
}