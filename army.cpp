/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Army Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  Contains the information about each army: which pieces they use, where
  they start, and what their special rules are (king turn).
*/

#include "army.hpp"

#include "player.hpp"

namespace c2
{

  PieceType corresponding(PieceType p, ArmyType army)
  {
    if (p == PieceType::CLA_PAWN)
      {
        if (army == ArmyType::NEMESIS)
          {
            return PieceType::NEM_PAWN;
          }
        else
          {
            return PieceType::CLA_PAWN;
          }
      }
    else if (p == PieceType::CLA_ROOK)
      {
        if (army == ArmyType::EMPOWERED)
          {
            return PieceType::EMP_ROOK;
          }
        else if (army == ArmyType::REAPER)
          {
            return PieceType::RPR_GHOST;
          }
        else if (army == ArmyType::ANIMALS)
          {
            return PieceType::ANI_ELEPHANT;
          }
        else
          {
            return PieceType::CLA_ROOK;
          }
      }
    else if (p == PieceType::CLA_KNIGHT)
      {
        if (army == ArmyType::EMPOWERED)
          {
            return PieceType::EMP_KNIGHT;
          }
        else if (army == ArmyType::ANIMALS)
          {
            return PieceType::ANI_WILDHORSE;
          }
        else
          {
            return PieceType::CLA_KNIGHT;
          }
      }
    else if (p == PieceType::CLA_BISHOP)
      {
         if (army == ArmyType::EMPOWERED)
          {
            return PieceType::EMP_BISHOP;
          }
         else if (army == ArmyType::ANIMALS)
          {
            return PieceType::ANI_TIGER;
          }
        else
          {
            return PieceType::CLA_BISHOP;
          }
      }
    else if (p == PieceType::CLA_QUEEN)
      {
        if (army == ArmyType::NEMESIS)
          {
            return PieceType::NEM_QUEEN;
          }
        else if (army == ArmyType::EMPOWERED)
          {
            return PieceType::EMP_QUEEN;
          }
        else if (army == ArmyType::REAPER)
          {
            return PieceType::RPR_REAPER;
          }
        else if (army == ArmyType::TWOKINGS)
          {
            return PieceType::TKG_WARRKING;
          }
        else if (army == ArmyType::ANIMALS)
          {
            return PieceType::ANI_JUNGQUEEN;
          }
        else
          {
            return PieceType::CLA_QUEEN;
          }
      }
    else if (p == PieceType::CLA_KING)
      {
        if (army == ArmyType::TWOKINGS)
          {
            return PieceType::TKG_WARRKING;
          }
        else
          {
            return PieceType::CLA_KING;
          }
      }
    else //Not a CLA piece?
      {
        return PieceType::NONE;
      }
  }

} //Namespace
