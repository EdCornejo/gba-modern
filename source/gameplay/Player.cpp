//--------------------------------------------------------------------------------
// Player.cpp
//--------------------------------------------------------------------------------
// <insert description here>
//--------------------------------------------------------------------------------
#include "Player.hpp"

#include "GameScene.hpp"
#include "data/sprites/player.hpp"

constexpr int PlayerPriority = 8;
constexpr s32f16 PlayerSpeed = 1.0; // pixels per frame
constexpr int CommonCooldown = 8; // frames
constexpr s16f7 ProjectileSpeed = 4.0; // pixels per frame
constexpr s16f7 ProjectileSize = 4.0; // pixels

static SinglePaletteAllocator palette EWRAM_BSS(data::sprites::player.png.palette);

void Player::init(s32f16 x, s32f16 y)
{
    // Copy the player's data to the main palette
    playerPtr = ObjectTilePointer(SpriteSize::s16x16_4bpp);
    playerPtr.setData(data::sprites::player.png.tiles);
    palPtr = SinglePalettePointer(palette);

    pos = vec2(x, y);

    health = maxHealth = 5;
    shootCooldown = 0;
}

void Player::update()
{
    // Move the player using the directional keys
    pos.x += PlayerSpeed * key_tri_horz();
    pos.y += PlayerSpeed * key_tri_vert();

    // Keep the player on screen
    pos.x = std::clamp<s32f16>(pos.x, PlayerWidth/2, SCREEN_WIDTH - PlayerWidth/2);
    pos.y = std::clamp<s32f16>(pos.y, HudSize + PlayerHeight/2, SCREEN_HEIGHT - HudSize - PlayerHeight/2);

    // Decrease the invcounter
    if (invCounter > 0) invCounter--;

    // If there is not a cooldown, allow the player to shoot
    if (shootCooldown > 0) shootCooldown--;
    else
    {
        if (key_held(KEY_A))
        {
            constexpr s32f16 Cos5 = 0.99619469809;
            constexpr s32f16 Sin5 = 0.08715574274;

            shootCooldown = CommonCooldown;
            gameScene().playerProjectiles.add(vec2<s16f7>(pos.x + PlayerWidth/2, pos.y),
                vec2<s16f7>(ProjectileSpeed, 0), vec2<s16f7>(ProjectileSize, ProjectileSize), 0);
            
            gameScene().playerProjectiles.add(vec2<s16f7>(pos.x + PlayerWidth/2 - 4, pos.y + 6),
                vec2<s16f7>(Cos5 * ProjectileSpeed, Sin5 * ProjectileSpeed),
                vec2<s16f7>(ProjectileSize, ProjectileSize), 0);
            
            gameScene().playerProjectiles.add(vec2<s16f7>(pos.x + PlayerWidth/2 - 4, pos.y - 6),
                vec2<s16f7>(Cos5 * ProjectileSpeed, -Sin5 * ProjectileSpeed),
                vec2<s16f7>(ProjectileSize, ProjectileSize), 0);
        }
    }
}

void Player::heal(int amount)
{
    if (amount == 0) return;
    if (amount < 0) damage(-amount);
    health += amount;
    if (health > maxHealth)
        health = maxHealth;
}

void Player::damage(int amount)
{
    if (amount == 0) return;
    if (amount < 0) heal(-amount);

    if (invCounter > 0) return;
    if (health > amount)
    {
        health -= amount;
        invCounter = 120;
    }
    else
    {
        // TODO: die
        for (;;); // Just hang up
    }
}

void Player::pushGraphics()
{
    // Push the sprite
    auto dp = vec2<int>(pos) - PlayerSize/2;

    if (!(invCounter & 1))
        graphics::oam.pushRegular(dp, SpriteSize::s16x16_4bpp, playerPtr.getTileId(), palPtr.getPalette(), 1, PlayerPriority);
}

GameScene& Player::gameScene()
{
    // Don't worry, I know what I'm doing
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
    return *reinterpret_cast<GameScene*>(
        reinterpret_cast<std::byte*>(this) - offsetof(GameScene, player));
#pragma GCC diagnostic pop
}