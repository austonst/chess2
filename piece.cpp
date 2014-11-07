/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Piece Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  Contains information to represent a chess 2 piece.
*/

#include "piece.hpp"

namespace c2
{

  //Quick global function for determining the rank of a piece
  std::uint8_t pieceRank(PieceType type)
  {
    if (type == PieceType::CLA_PAWN || type == PieceType::NEM_PAWN)
      {
        return 1;
      }
    else if (type == PieceType::CLA_KNIGHT || type == PieceType::CLA_BISHOP ||
             type == PieceType::EMP_KNIGHT || type == PieceType::EMP_BISHOP ||
             type == PieceType::ANI_WILDHORSE || type == PieceType::ANI_TIGER)
      {
        return 2;
      }
    else if (type == PieceType::CLA_ROOK || type == PieceType::EMP_ROOK ||
             type == PieceType::RPR_GHOST || type == PieceType::ANI_ELEPHANT)
      {
        return 3;
      }
    else if (type == PieceType::CLA_QUEEN || type == PieceType::NEM_QUEEN ||
             type == PieceType::EMP_QUEEN || type == PieceType::RPR_REAPER ||
             type == PieceType::ANI_JUNGQUEEN)
      {
        return 4;
      }
    else
      {
        //What is a rank?
        return 0;
      }
  }

  Piece::Piece(PieceType type, const Position& pos, SideType owner) :
    _type(type), _pos(pos), _owner(owner) {}

  bool Piece::moveTo(const Position& pos)
  {
    if (pos.isValid())
      {
        _pos = pos;
        return true;
      }
    return false;
  }

  bool Piece::move(const Move& move)
  {
    //Destination must be valid
    if (!move.end.isValid())
      {
        return false;
      }

    //If start is valid, must match current position
    if (!move.start.isValid())
      {
        return false;
      }
    else
      {
        if (move.start != _pos)
          {
            return false;
          }
      }

    //If additional data is provided, they must match
    if (move.type != _type || move.side != _owner)
      {
        return false;
      }

    //If all those checks passed, we can move
    _pos = move.end;
    return true;
  }

  /*
  std::set<Position> Piece::allMoves(Piece p)
  {
    //The only differences between a CLA_QUEEN and NEM_QUEEN are in filtering
    //For now, NEM_QUEEN's are CLA_QUEEN's.
    PieceType pt = p.type();
    if (pt == PieceType::NEM_QUEEN) pt = PieceType::CLA_QUEEN;
    
    //We may need to gather possibilities from multiple MoveTypes
    std::set<Position> ret;
    for (auto i = POSSIBLE_MOVES[pt].begin(); i != POSSIBLE_MOVES[pt].end(); i++)
      {
        //Big switch for each possible MoveType
        switch(*i)
          {
          case MoveType::PAWN_CLA:
            //Three possible moves
            char dir = p.side()==SideType::WHITE?1:-1;
            for (char i = -1; i < 2; i++)
              {
                Position option(p.pos().x()+i,p.pos().y()+dir);
                if (option.isValid()) ret.insert(option);
              }
            break;

          case MoveType::PAWN_NEM:
            //Can potentially move in any direction depending on king location
            for (char i = -1; i < 2; i++)
              {
                for (char j = -1; j < 2; j++)
                  {
                    if ((p.pos() + Position(i,j)).isValid())
                      {
                        ret.insert(p.pos() + Position(i,j));
                      }
                  }
              }
            ret.erase(p.pos());
            break;
            
          case MoveType::ROOK_CLA:
            for (Position dir :
              {Position(0,1), Position(0,-1), Position(1,0), Position(-1,0)})
              {
                Position step = p.pos();
                while ((step+=dir).isValid())
                  {
                    ret.insert(step);
                  }
              }
            break;
            
          case MoveType::ROOK_GHOST:
            //Can move anywhere but itself
            for (char i = 1; i < 9; i++)
              {
                for (char j = 1; j < 9; j++)
                  {
                    ret.insert(Position(i,j));
                  }
              }
            ret.erase(p.pos());
            break;
            
          case MoveType::ROOK_ELEPHANT:
            for (Position dir :
              {Position(0,1), Position(0,-1), Position(1,0), Position(-1,0)})
              {
                Position step = p.pos();
                for (char i = 0; i < 3 && (step+=dir).isValid(); i++)
                  {
                    ret.insert(step);
                  }
              }
            break;
            
          case MoveType::KNIGHT_CLA:
          case MoveType::KNIGHT_WILDHORSE:
            //These move the same, but filter differently
            //Easiest to explicitly list them
            for (Position offset :
              {Position(-2,-1), Position(-2,1), Position(-1,2), Position(-1,-2),
                  Position(1,-2), Position(1,2), Position(2,-1), Position(2,1)})
              {
                if ((p.pos()+offfset).isValid())
                  {
                    ret.insert(p.pos() + offset);
                  }
              }
            break;
            
          case MoveType::BISHOP_CLA:
            for (Position dir :
              {Position(-1,-1), Position(1,-1), Position(-1,1), Position(1,1)})
              {
                Position step = p.pos();
                for (char i = 0; (step+=dir).isValid(); i++)
                  {
                    ret.insert(step);
                  }
              }
            break;
            
          case MoveType::BISHOP_TIGER:
            for (Position dir :
              {Position(-1,-1), Position(1,-1), Position(-1,1), Position(1,1)})
              {
                Position step = p.pos();
                for (char i = 0; i < 2 && (step+=dir).isValid(); i++)
                  {
                    ret.insert(step);
                  }
              }
            break;

            //case MoveType::QUEEN_NEM:
            //Not actually a move type here, we're pretending they're CLA_QUEEN
            
          case MoveType::QUEEN_RPR:
            //Can't move to back row or itself
            char startRow = p.side()==SideType::WHITE?1:2;
            char endRow = p.side()==SideType::WHITE?7:8;
            for (char i = 1; i < 9; i++)
              {
                for (char j = startRow; j < endRow; j++)
                  {
                    ret.insert(Position(i,j));
                  }
              }
            ret.erase(p.pos());
            break;
            
          case MoveType::KING_CLA:
            for (char i = -1; i < 2; i++)
              {
                for (char j = -1; j < 2; j++)
                  {
                    if ((p.pos()+Position(i,j)).isValid())
                      {
                        ret.insert(p.pos() + Position(i,j));
                      }
                  }
              }
            ret.erase(p.pos());
            break;
            
          case MoveType::KING_2KG:
            //Just like KING_CLA, but can "move" to itself for whirlwind
            for (char i = -1; i < 2; i++)
              {
                for (char j = -1; j < 2; j++)
                  {
                    if ((p.pos()+Position(i,j)).isValid())
                      {
                        ret.insert(p.pos() + Position(i,j));
                      }
                  }
              }
            break;
            
          default: break;
            
          }
      }

    //All done, return the options
    return ret;
  }
  */
  
} //namespace
