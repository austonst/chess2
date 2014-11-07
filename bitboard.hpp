/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----BitBoard Class Header-----
  Auston Sterling
  austonst@gmail.com

  
*/

#ifndef _bitboard_hpp_
#define _bitboard_hpp_

#include <cstdint>

#include "board.hpp"

namespace c2
{

  class BitBoard : public Board
  {
  public:
    BitBoard();
    BitBoard(ArmyType white, ArmyType black);

    Board* clone() const;

    Piece operator()(Position p) const;

    std::list<Position> getPieces(SideType side);

    std::vector<Position> getKing(SideType side);

    void destroy(Position p);

    void promote(Position pos, PieceType type);
    
    bool move(const Move& m);

    void clear();
    void addArmy(SideType side, ArmyType army);
    
  private:
    //Helper functions
    std::uint64_t posToBit(Position p) const;
    Position bitToPos(std::uint64_t b) const;
    
    //Positions of white/black pieces
    std::uint64_t _side[2];

    //Positions of each type of piece
    std::uint64_t _type[PIECE_TYPES];
    
  };
  
} //Namespace

#endif
