#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "EventT.hpp"
#include "IPort.hpp"

namespace Snake
{
    ConfigurationError::ConfigurationError()
        : std::logic_error("Bad configuration of Snake::Controller.")
    {
    }

    UnexpectedEventException::UnexpectedEventException()
        : std::runtime_error("Unexpected event received!")
    {
    }

    Controller::Controller(IPort &p_displayPort, IPort &p_foodPort, IPort &p_scorePort, std::string const &p_config)
        : m_displayPort(p_displayPort),
          m_foodPort(p_foodPort),
          m_scorePort(p_scorePort)
    {
        std::istringstream istr(p_config);
        char w, f, s, d;

        int width, height, length;
        int foodX, foodY;
        istr >> w >> width >> height >> f >> foodX >> foodY >> s;

        if (w != 'W' or f != 'F' or s != 'S')
            throw ConfigurationError();

        m_mapDimension = {width, height};
        m_foodPosition = {foodX, foodY};

        istr >> d;
        switch (d)
        {
        case 'U':
            m_currentDirection = Direction_UP;
            break;
        case 'D':
            m_currentDirection = Direction_DOWN;
            break;
        case 'L':
            m_currentDirection = Direction_LEFT;
            break;
        case 'R':
            m_currentDirection = Direction_RIGHT;
            break;
        default:
            throw ConfigurationError();
        }

        istr >> length;

        while (length--)
        {
            Segment seg;
            istr >> seg.position.x >> seg.position.y;

            m_segments.push_back(seg);
        }
    }

    void Controller::receive(std::unique_ptr<Event> e)
    {
        auto messageId = e->getMessageId();

        switch (messageId)
        {
        case TimeoutInd::MESSAGE_ID:
            receiveTimeoutInd();
            break;
        case DirectionInd::MESSAGE_ID:
            receiveDirectionInd(payload<DirectionInd>(*e));
            break;
        case FoodInd::MESSAGE_ID:
            receiveFoodInd(payload<FoodInd>(*e));
            break;
        case FoodResp::MESSAGE_ID:
            receiveFoodResp(payload<FoodResp>(*e));
            break;
        case PauseInd::MESSAGE_ID:
            receivePauseInd();
            break;
        default:
            throw UnexpectedEventException();
            break;
        }
    }

    Controller::Segment Controller::getNewHead()
    {
        Segment newHead(m_segments.front().position);

        switch (m_currentDirection)
        {
        case Direction_RIGHT:
            newHead.position.x += 1;
            break;
        case Direction_LEFT:
            newHead.position.x -= 1;
            break;
        case Direction_UP:
            newHead.position.y -= 1;
            break;
        case Direction_DOWN:
            newHead.position.y += 1;
            break;
        }

        return newHead;
    }

    void Controller::receiveTimeoutInd()
    {
        if (m_pauseActive)
            return;

        Segment newHead = getNewHead();

        if (headToSnakeCollision(newHead) or headToWallCollision(newHead))
        {
            m_scorePort.send(std::make_unique<EventT<LooseInd>>());
            return;
        }

        if (newHead.position == m_foodPosition)
        {
            m_scorePort.send(std::make_unique<EventT<ScoreInd>>());
            m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        }
        else
        {
            moveSnakeBody();
        }

        addSnakeHead(newHead);
    }

    void Controller::receiveDirectionInd(DirectionInd directionInd)
    {
        if (m_pauseActive)
            return;
        
        auto const newDirection = directionInd.direction;
        
        if ((m_currentDirection & 0b01) != (newDirection & 0b01))
            m_currentDirection = newDirection;
    }

    void Controller::receiveFoodInd(FoodInd receivedFood)
    {
        bool requestedFoodCollidedWithSnake = false;
        for (auto const &segment : m_segments)
        {
            if (segment.position == receivedFood.position)
            {
                requestedFoodCollidedWithSnake = true;
                break;
            }
        }

        if (requestedFoodCollidedWithSnake)
        {
            m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        }
        else
        {
            sendCellToDisplay({m_foodPosition, Cell_FREE});
            sendCellToDisplay({receivedFood.position, Cell_FOOD});
        }

        m_foodPosition = receivedFood.position;
    }

    void Controller::receiveFoodResp(FoodResp requestedFood)
    {
        bool requestedFoodCollidedWithSnake = false;
        for (auto const &segment : m_segments)
        {
            if (segment.position == requestedFood.position)
            {
                requestedFoodCollidedWithSnake = true;
                break;
            }
        }

        if (requestedFoodCollidedWithSnake)
        {
            m_foodPort.send(std::make_unique<EventT<FoodReq>>());
        }
        else
        {
            sendCellToDisplay({requestedFood.position, Cell_FOOD});
        }

        m_foodPosition = requestedFood.position;
    }

    bool Controller::headToSnakeCollision(Segment newHead)
    {
        for (auto segment : m_segments)
            if (newHead.collide(segment))
                return true;

        return false;
    }

    bool Controller::headToWallCollision(Segment newHead)
    {
        return m_mapDimension.collide(newHead.position);
    }

    void Controller::sendCellToDisplay(DisplayInd displayInd)
    {
        m_displayPort.send(std::make_unique<EventT<DisplayInd>>(displayInd));
    }

    void Controller::moveSnakeBody()
    {
        sendCellToDisplay({m_segments.back().position, Cell_FREE});
        m_segments.pop_back();
    }

    void Controller::addSnakeHead(Segment newHead)
    {
        m_segments.push_front(newHead);
        sendCellToDisplay({newHead.position, Cell_SNAKE});
    }

    void Controller::receivePauseInd()
    {
        m_pauseActive = !m_pauseActive;
    }

} // namespace Snake
