/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Position Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a position on a chessboard.
*/

#include "position.hpp"

namespace c2
{

  Position::Position() : _x(0), _y(0) {}

  Position::Position(char x, char y) : _x(x), _y(y) {}

  Position::Position(const std::string& notate)
  {
    if (notate.length() != 2)
      {
        _x = _y = 0;
      }
    else
      {
        //First character should be 'a'-'h'
        _x = (notate[0] < 'a' || notate[0] > 'h') ? 0 : notate[0] - 'a' + 1;

        //Second character should be '1'-'8'
        _y = (notate[1] < '1' || notate[1] > '8') ? 0 : notate[1] - '1' + 1;
      }
  }

  std::string Position::notation() const
  {
    if (!isValid())
      {
        return "00";
      }
    std::string ret(2,'0');
    ret[0] = _x + 'a' - 1;
    ret[1] = _y + '1' - 1;
    return ret;
  }

  bool Position::isValid() const
  {
    return _x > 0 && _x < 9 && _y > 0 && _y < 9;
  }

  Position operator+(const Position& p1, const Position& p2)
  {
    return Position(p1.x()+p2.x(), p1.y()+p2.y());
  }

  Position& operator+=(Position& p1, const Position& p2)
  {
    p1.x() += p2.x();
    p1.y() += p2.y();
    return p1;
  }
  
  Position operator-(const Position& p1, const Position& p2)
  {
    return Position(p1.x()-p2.x(), p1.y()-p2.y());
  }
  
  Position& operator-=(Position& p1, const Position& p2)
  {
    p1.x() -= p2.x();
    p1.y() -= p2.y();
    return p1;
  }

  bool operator<(const Position& p1, const Position& p2)
  {
    if (p1.y() < p2.y()) return true;
    if (p1.y() > p2.y()) return false;
    if (p1.x() < p2.x()) return true;
    return false;
  }

  bool operator==(const Position& p1, const Position& p2)
  {
    return p1.x() == p2.x() && p1.y() == p2.y();
  }

  bool operator!=(const Position& p1, const Position& p2)
  {
    return p1.x() != p2.x() || p1.y() != p2.y();
  }
  
} //namespace
