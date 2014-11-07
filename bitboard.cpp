/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----BitBoard Class Implementation-----
  Auston Sterling
  austonst@gmail.com


*/

#include "bitboard.hpp"

namespace c2
{

  BitBoard::BitBoard()
  {
    _side[0] = 0x0ULL;
    _side[1] = 0x0ULL;
    
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] = 0x0ULL;
      }
  }
  
  BitBoard::BitBoard(ArmyType white, ArmyType black)
  {
    _side[0] = 0x0ULL;
    _side[1] = 0x0ULL;
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] = 0x0ULL;
      }

    addArmy(SideType::WHITE, white);
    addArmy(SideType::BLACK, black);
  }

  Board* BitBoard::clone() const
  {
    BitBoard* bCopy = new BitBoard;
    bCopy->_side[0] = _side[0];
    bCopy->_side[1] = _side[1];
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        bCopy->_type[i] = _type[i];
      }
    return bCopy;
  }

  Piece BitBoard::operator()(Position p) const
  {
    //Compute the bit from the coordinates
    uint64_t b = posToBit(p);

    //Determine the SideType
    Piece ret;
    ret.pos() = p;
    if (b & _side[num(SideType::WHITE)])
      {
        ret.side() = SideType::WHITE;
      }
    else if (b & _side[num(SideType::BLACK)])
      {
        ret.side() = SideType::BLACK;
      }
    else
      {
        ret.side() = SideType::NONE;
      }

    //Determine the PieceType
    ret.type() = PieceType::NONE;
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        if (b & _type[i])
          {
            ret.type() = PieceType(i);
            break;
          }
      }

    //All done!
    return ret;
  }

  std::list<Position> BitBoard::getPieces(SideType side)
  {
    //Get the right bitboard
    std::list<Position> ret;
    if (side == SideType::NONE) return ret;
    std::uint64_t sideBoard = _side[num(side)];

    //Pull out the positions
    //We need to check all 64 spots
    std::uint64_t bit = 1;
    for (uint8_t i = 0; i < 64; i++)
      {
        if (sideBoard & bit)
          {
            ret.push_back(bitToPos(bit));
          }
        bit <<= 1;
      }

    //Return what we've got
    return ret;
  }

  std::vector<Position> BitBoard::getKing(SideType side)
  {
    //Get the kings for the right side
    std::vector<Position> ret;
    if (side == SideType::NONE) return ret;
    std::uint64_t kings = _side[num(side)] &
      (_type[num(PieceType::CLA_KING)] | _type[num(PieceType::ANY_KING)] |
       _type[num(PieceType::TKG_WARRKING)]);

    //Pull out the positions
    //We need to check all 64 spots
    std::uint64_t bit = 1;
    for (uint8_t i = 0; i < 64; i++)
      {
        if (kings & bit)
          {
            ret.push_back(bitToPos(bit));
          }
        bit <<= 1;
      }

    //Return what we've got
    return ret;
  }

  void BitBoard::destroy(Position p)
  {
    //Wipe that spot from every board
    std::uint64_t bit = posToBit(p);
    _side[0] &= ~bit;
    _side[1] &= ~bit;

    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] &= ~bit;
      }
  }

  void BitBoard::promote(Position pos, PieceType type)
  {
    std::uint64_t bit = posToBit(pos);

    //Unset this bit from all boards, then set it for the new type
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] &= ~bit;
      }
    _type[num(type)] |= bit;
  }
  
  bool BitBoard::move(const Move& m)
  {
    uint64_t startBit = posToBit(m.start);
    uint64_t endBit = posToBit(m.end);
    //Perform one last check that this Move makes sense
    //Though most checking should have been done before this is called.
    if (!(startBit & _type[num(m.type)]) || !(startBit & _side[num(m.side)]) ||
        !(m.start.isValid()) || !(m.end.isValid()))
      {
        return false;
      }

    //Remove the piece from the initial position
    _type[num(m.type)] &= ~startBit;
    _side[num(m.side)] &= ~startBit;

    //Make sure there is now a piece of the correct side at the end
    _side[num(m.side)] |= endBit;

    //Any other piece that was at the end is gone
    _side[num(otherSide(m.side))] &= ~endBit;
    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] &= ~endBit;
      }

    //And now our piece moves in
    _type[num(m.type)] |= endBit;

    return true;
  }

  void BitBoard::clear()
  {
    _side[0] = 0x0;
    _side[1] = 0x0;

    for (size_t i = 0; i < PIECE_TYPES; i++)
      {
        _type[i] = 0x0;
      }
  }

  void BitBoard::addArmy(SideType side, ArmyType army)
  {
    //All armies take up the same spots, so we can fill in the side at least
    _side[num(side)] |= 0xFFFFULL << (num(side) * 0x30);

    //Set up pawns
    //Black pawns are 5 rows * 8 cols = 40 spots down from white pawns
    PieceType pawnType = corresponding(PieceType::CLA_PAWN, army);
    std::uint8_t blackRowShift = 40 * num(side);
    _type[num(pawnType)] |= 0xFF00ULL << blackRowShift;

    //Create rooks
    //Black back row is 7 rows * 8 cols = 56 spots down
    PieceType rookType = corresponding(PieceType::CLA_ROOK, army);
    blackRowShift = 56 * num(side);
    _type[num(rookType)] |= 0x81ULL << blackRowShift;

    //Create knights
    PieceType knightType = corresponding(PieceType::CLA_KNIGHT, army);
    _type[num(knightType)] |= 0x42ULL << blackRowShift;

    //Create bishops
    PieceType bishopType = corresponding(PieceType::CLA_BISHOP, army);
    _type[num(bishopType)] |= 0x24ULL << blackRowShift;

    //Create queen
    PieceType queenType = corresponding(PieceType::CLA_QUEEN, army);
    _type[num(queenType)] |= 0x08ULL << blackRowShift;

    //Create queen
    PieceType kingType = corresponding(PieceType::CLA_KING, army);
    _type[num(kingType)] |= 0x10ULL << blackRowShift;
  }

  std::uint64_t BitBoard::posToBit(Position p) const
  {
    return 1ULL<<((p.y()-1)*8+(p.x()-1));
  }

  Position BitBoard::bitToPos(uint64_t b) const
  {
    char bit = 0;
    while (b >>= 1) bit++;
    return Position(bit%8 + 1, bit/8 + 1);
  }

} //Namespace
