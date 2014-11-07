/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Board Class Header-----
  Auston Sterling
  austonst@gmail.com

  An abstract class which represents any Chess 2 board. Derived classes
  can have different underlying representations to maximize speed or ease
  of extensibility or something.

  OOBoard is an unoptimized board which integrates object oriented practices.
*/

#ifndef _board_hpp_
#define _board_hpp_

#include <array>

#include "army.hpp"

class Player;

namespace c2
{

  class Board
  {
  public:
    //Virtual destructor to make things happy
    virtual ~Board() {};
    
    //Every board must be able to clone itself
    virtual Board* clone() const = 0;
    
    //Every Board must provide a Piece accessor
    virtual Piece operator()(Position p) const = 0;

    //Every Board must provide a function to get the positions of each piece
    //of a given side
    virtual std::list<Position> getPieces(SideType side) = 0;

    //Every board must be able to provide the location of the kings
    //This allows for quicker checking of "check" and midline for 2 kings army
    virtual std::vector<Position> getKing(SideType side) = 0;

    //Every board must have a function for destroying a piece
    virtual void destroy(Position p) = 0;

    //Every board must provide a function for promoting a pawn
    virtual void promote(Position pos, PieceType type) = 0;

    //Every Board must provide a move function
    //If a piece moves to a spot with another piece, destroy the old piece
    virtual bool move(const Move& m) = 0;

    //Every Board must have functions for clearing and initializing the board
    //Clears the board of all pieces
    virtual void clear() = 0;
    //Adds the specified army on the specified side
    virtual void addArmy(SideType side, ArmyType army) = 0;
  };

} //Namespace

#endif
