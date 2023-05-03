#pragma once

#include <list>
#include <memory>
#include <stdexcept>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"

class Event;
class IPort;

namespace Snake
{
struct ConfigurationError : std::logic_error
{
    ConfigurationError();
};

struct UnexpectedEventException : std::runtime_error
{
    UnexpectedEventException();
};

class Controller : public IEventHandler
{
public:
    Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config);

    Controller(Controller const& p_rhs) = delete;
    Controller& operator=(Controller const& p_rhs) = delete;

    void receive(std::unique_ptr<Event> e) override;

private:
    struct Segment
    {
        Segment() {}
        Segment(Coord pos) : position(pos) {}

        bool collide(const Segment &other) const
        {
            return position == other.position;
        }

        Coord position;
    };
    
    struct MapDimension
    {
        int width;
        int height;

        bool collide(const Coord &pos) const
        {
            return pos.x < 0 or pos.y < 0 or pos.x >= width or pos.y >= height;
        }
    };

    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;

    MapDimension m_mapDimension;
    Coord m_foodPosition;

    Direction m_currentDirection;
    std::list<Segment> m_segments;
    bool m_pauseActive = false;

    Segment getNewHead();
    void receiveTimeoutInd();
    void receiveDirectionInd(DirectionInd);
    void receiveFoodInd(FoodInd);
    void receiveFoodResp(FoodResp);
    void receivePauseInd();

    bool headToSnakeCollision(Segment);
    bool headToWallCollision(Segment);
    void moveSnakeBody();
    void addSnakeHead(Segment);
    void sendCellToDisplay(DisplayInd);
};

} // namespace Snake
