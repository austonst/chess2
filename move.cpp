/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Move Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a move on a chessboard.
*/

#include "move.hpp"
#include "piece.hpp"

namespace c2
{

  Move::Move() :
    start(-1,-1), end(-1,-1),
    type(PieceType::NONE),side(SideType::NONE) {}

  Move::Move(const Position& start, const Position& end,
       PieceType type, SideType side) :
    start(start), end(end), type(type),side(side) {}

  bool operator<(const Move& m1, const Move& m2)
  {
    if (m1.start < m2.start) return true;
    if (m1.end < m2.end) return true;
    if (m1.type < m2.type) return true;
    if (m1.side < m2.side) return true;
    return false;
  }
  
} //namespace
