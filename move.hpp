/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Move Class Header-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a move on a chessboard. Not much more than PoD.
*/

#ifndef _move_hpp_
#define _move_hpp_

#include <cstdint>
#include "position.hpp"

namespace c2
{

  enum class PieceType : std::uint8_t;
  enum class SideType : std::uint8_t;

  struct Move
  {
  public:
    //Constructors
    Move();

    //Construct with data
    Move(const Position& start, const Position& end,
         PieceType type, SideType side);

    //The starting and ending positions of the move
    Position start;
    Position end;

    //Stats for the piece which made the move.
    PieceType type;
    SideType side;
  };

  bool operator<(const Move& m1, const Move& m2);

} //namespace

#endif
