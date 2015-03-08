/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Army Header-----
  Auston Sterling
  austonst@gmail.com

  Contains the information about each army: which pieces they use, where
  they start, and what their special rules are (king turn).
*/

#ifndef _army_hpp_
#define _army_hpp_

#include <list>
#include "piece.hpp"

namespace c2
{

  const std::size_t NUM_ARMIES = 6;
  
  enum class ArmyType : std::uint8_t
    {
      CLASSIC,
      NEMESIS,
      EMPOWERED,
      REAPER,
      TWOKINGS,
      ANIMALS,
      NONE
    };
  inline std::uint8_t num(ArmyType a) {return static_cast<std::uint8_t>(a);}
  inline ArmyType toArmy(std::uint8_t n) {return static_cast<ArmyType>(n);}

  //ARMY_PROMOTE(num(ArmyTpe)) gives you the pieces a pawn of that army
  //can promote to
  const std::vector<std::set<PieceType> > ARMY_PROMOTE =
    {
      {PieceType::CLA_ROOK, PieceType::CLA_KNIGHT, PieceType::CLA_BISHOP,
       PieceType::CLA_QUEEN},
      {PieceType::CLA_ROOK, PieceType::CLA_KNIGHT, PieceType::CLA_BISHOP,
       PieceType::NEM_QUEEN},
      {PieceType::EMP_ROOK, PieceType::EMP_KNIGHT, PieceType::EMP_BISHOP,
       PieceType::EMP_QUEEN},
      {PieceType::RPR_GHOST, PieceType::CLA_KNIGHT, PieceType::CLA_BISHOP,
       PieceType::RPR_REAPER},
      {PieceType::CLA_ROOK, PieceType::CLA_KNIGHT, PieceType::CLA_BISHOP},
      {PieceType::ANI_ELEPHANT, PieceType::ANI_WILDHORSE, PieceType::ANI_TIGER,
       PieceType::ANI_JUNGQUEEN}
    };

  //Returns true if the army can take an extra king turn
  inline bool hasKingTurn(ArmyType army)
  {
    return (army == ArmyType::TWOKINGS);
  }

  //Returns true if the army can castle
  inline bool canCastle(ArmyType army)
  {
    return (army == ArmyType::CLASSIC);
  }

  //A function to get the equivalent PieceType for another army
  //given the CLA version.
  PieceType corresponding(PieceType p, ArmyType a);

} //Namespace

#endif
