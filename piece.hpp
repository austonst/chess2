/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Piece Class Header-----
  Auston Sterling
  austonst@gmail.com

  Contains information to represent a chess 2 piece.
*/

#ifndef _piece_hpp_
#define _piece_hpp_

#include <vector>
#include <list>
#include <set>
#include <cstdint>
#include "move.hpp"

namespace c2
{

  //All possible pieces in Sirlin Chess 2
  const std::uint8_t PIECE_TYPES = 20;
  enum class PieceType : std::uint8_t
    {
      CLA_PAWN,
      CLA_ROOK,
      CLA_KNIGHT,
      CLA_BISHOP,
      CLA_QUEEN,
      CLA_KING,
      NEM_PAWN,
      NEM_QUEEN,
      ANY_KING,
      EMP_ROOK,
      EMP_KNIGHT,
      EMP_BISHOP,
      EMP_QUEEN,
      RPR_REAPER,
      RPR_GHOST,
      TKG_WARRKING,
      ANI_WILDHORSE,
      ANI_TIGER,
      ANI_ELEPHANT,
      ANI_JUNGQUEEN,
      NONE
    };
  inline std::uint8_t num(PieceType p) {return static_cast<std::uint8_t>(p);}

  //Quick global function for determining the rank of a piece
  std::uint8_t pieceRank(PieceType type);

  //The names of the pieces, for any purposes that might need them
  const std::vector<std::string> PIECE_NAME =
    {
      "Classic Pawn", //CLA_PAWN,
      "Classic Rook", //CLA_ROOK,
      "Classic Knight", //CLA_KNIGHT,
      "Classic Bishop", //CLA_BISHOP,
      "Classic Queen", //CLA_QUEEN,
      "Classic King", //CLA_KING,
      "Nemesis Pawn", //NEM_PAWN,
      "Nemesis Queen", //NEM_QUEEN,
      "King", //ANY_KING,
      "Empowered Rook", //EMP_ROOK,
      "Empowered Knight", //EMP_KNIGHT,
      "Empowered Bishop", //EMP_BISHOP,
      "Empowered Queen", //EMP_QUEEN,
      "Reaper", //RPR_REAPER,
      "Ghost", //RPR_GHOST,
      "Warrior King", //TKG_WARRKING,
      "Wild Horse", //ANI_WILDHORSE,
      "Tiger", //ANI_TIGER,
      "Elephant", //ANI_ELEPHANT,
      "Jungle Queen", //ANI_JUNGQUEEN,
      "None"//NONE
    };

  //The possible ways of moving, broken down to base components
  enum class MoveType : std::uint8_t
    {
      PAWN_CLA,
      PAWN_NEM,
      ROOK_CLA,
      ROOK_GHOST,
      ROOK_ELEPHANT,
      KNIGHT_CLA,
      KNIGHT_WILDHORSE,
      BISHOP_CLA,
      BISHOP_TIGER,
      QUEEN_NEM,
      QUEEN_RPR,
      KING_ANY,
      KING_CLA,
      KING_2KG
    };
  inline std::uint8_t num(MoveType m) {return static_cast<std::uint8_t>(m);}

  //The possible moves for each of the types of pieces
  //IMPORTANT: The order here must match the PieceType order
  const std::vector<std::list<MoveType> > MOVE_TYPES =
    { {MoveType::PAWN_CLA},                       //CLA_PAWN
      {MoveType::ROOK_CLA},                       //CLA_ROOK
      {MoveType::KNIGHT_CLA},                     //CLA_KNIGHT
      {MoveType::BISHOP_CLA},                     //CLA_BISHOP
      {MoveType::ROOK_CLA, MoveType::BISHOP_CLA}, //CLA_QUEEN
      {MoveType::KING_ANY, MoveType::KING_CLA},   //CLA_KING
      {MoveType::PAWN_CLA, MoveType::PAWN_NEM},   //NEM_PAWN
      {MoveType::QUEEN_NEM},                      //NEM_QUEEN
      {MoveType::KING_ANY},                       //ANY_KING
      {MoveType::ROOK_CLA},                       //EMP_ROOK
      {MoveType::KNIGHT_CLA},                     //EMP_KNIGHT
      {MoveType::BISHOP_CLA},                     //EMP_BISHOP
      {MoveType::KING_ANY},                       //EMP_QUEEN
      {MoveType::QUEEN_RPR},                      //RPR_REAPER
      {MoveType::ROOK_GHOST},                     //RPR_GHOST
      {MoveType::KING_2KG},                       //2KG_WARRKING
      {MoveType::KNIGHT_WILDHORSE},               //ANI_WILDHORSE
      {MoveType::BISHOP_TIGER},                   //ANI_TIGER
      {MoveType::ROOK_ELEPHANT},                  //ANI_ELEPHANT
      {MoveType::ROOK_CLA, MoveType::KNIGHT_CLA}, //ANI_JUNGQUEEN
      {} };                        //NONE

  enum class SideType : std::uint8_t
    {
      WHITE,
      BLACK,
      NONE
    };
  inline std::uint8_t num(SideType s) {return static_cast<std::uint8_t>(s);}

  //Quick global function for swapping between white and black
  inline SideType otherSide(SideType s)
  {
    return static_cast<SideType>(~num(s) & 0x01);
  }

  class Piece
  {
  public:
    //Constructors
    //General use constructor, covers everything
    Piece(PieceType type = PieceType::NONE, const Position& pos = Position(0,0),
          SideType owner = SideType::NONE);

    //Accessors/mutators
    PieceType& type() {return _type;}
    Position& pos() {return _pos;}
    SideType& side() {return _owner;}
    PieceType type() const {return _type;}
    const Position& pos() const {return _pos;}
    SideType side() const {return _owner;}

    //Mutators for slightly different inputs
    //These return true if it passes basic checks and false otherwise
    //Checks board bounds
    bool moveTo(const Position& pos);

    //Verifies that any provided information in the Move matches current state
    bool move(const Move& move);

    //A static function to get the possible moves for a piece
    //This ignores all other pieces on the board!
    //Some moves need to be filtered out or added depending on board state.
    //In addition to things like "can't capture own team", these are:
    //PAWN_CLA: Diagonal moves only when capturing (en passant, too)
    //PAWN_NEM: Diagonal moves on capture/ep. Radial moves on nem move
    //ROOK_CLA: Stops at first othogonal piece
    //ROOK_GHOST: Can't move to any piece (more imp: other pieces can't cap)
    //ROOK_ELEPHANT: Stop at first ortho piece, rampage handled elsewhere
    //KNIGHT_CLA: None
    //KNIGHT_WILDHORSE: Even more none because don't have to filter all allies
    //BISHOP_CLA: Stops at first diagonal piece
    //BISHOP_TIGER: Stops at first diagonal piece
    //QUEEN_NEM: Stops one before any other piece, but stops ON enemy king
    //QUEEN_RPR: Cannot move to king
    //KING_CLA and KING_2KG: Cannot move into check
    static std::set<Position> allMoves(Piece p);
  
  private:
    //The type of piece
    PieceType _type;

    //The board position
    Position _pos;

    //The controlling side
    SideType _owner;
  
  };

} //namespace

#endif
