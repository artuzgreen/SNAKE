#pragma once

#include <cstdint>

namespace Snake
{

    enum Direction
    {
        Direction_UP = 0b00,
        Direction_DOWN = 0b10,
        Direction_LEFT = 0b01,
        Direction_RIGHT = 0b11
    };

    struct Coord
    {
        int x;
        int y;

        bool operator==(const Coord &other) const
        {
            return x == other.x && y == other.y;
        }
    };

    struct DirectionInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x10;

        Direction direction;
    };

    struct TimeoutInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x20;
    };

    enum Cell
    {
        Cell_FREE,
        Cell_FOOD,
        Cell_SNAKE
    };

    struct DisplayInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x30;

        DisplayInd() {}
        DisplayInd(Coord pos, Cell val) : position(pos), value(val) {}

        Coord position;
        Cell value;
    };

    struct FoodInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x40;

        FoodInd() {}
        FoodInd(Coord pos) : position(pos) {}

        Coord position;
    };

    struct FoodReq
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x41;
    };

    struct FoodResp
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x42;

        FoodResp() {}
        FoodResp(Coord pos) : position(pos) {}

        Coord position;
    };

    struct ScoreInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x70;
    };

    struct LooseInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x71;
    };

    struct PauseInd
    {
        static constexpr std::uint32_t MESSAGE_ID = 0x91;
    };

} // namespace Snake
