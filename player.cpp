/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Player Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a player in a game of Chess 2.
*/

#include "player.hpp"

namespace c2
{

  Player::Player(const std::string& name, Board* board, ArmyType army,
                 SideType side, char stones) :
    _name(name), _board(board), _side(side), _army(army), _stones(stones) {}

  void Player::grantStone()
  {
    _stones++;
    if (_stones > 6) _stones = 6;
  }

  bool Player::bidStones(char num)
  {
    if (num > _stones) return false;
    _stones -= num;
    return true;
  }

  void Player::setupArmy()
  {
    _pieces = createPieces(_army, this);
  }

  void Player::pieceCaptured(Piece* p)
  {
    //Ensure that this piece belongs to this player
    for (auto i = _pieces.begin(); i != pieces.end(); i++)
      {
        if (*i == *p)
          {
            p->pos() = Position(0,0);
          }
      }
  }

} //Namespace
