/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Player Class Header-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a player in a game of Chess 2.
*/

#ifndef _player_hpp_
#define _player_hpp_

#include <string>
#include <list>
#include "army.hpp"

class Piece;
class Board;

namespace c2
{

  class Player
  {
  public:
    //Constructors
    //Main constructor to set up for a game
    Player(const std::string& name = "Player", Board* board = nullptr,
           ArmyType army = ArmyType::CLASSIC, SideType side = SideType::NONE,
           char stones = 3);

    //Accessors/mutators
    std::string& name() {return _name;}
    Board* board() {return _board;}
    SideType& side() {return _side;}
    ArmyType& army() {return _army;}
    char& stones() {return _stones;}
    const std::string& name() const {return _name;}
    SideType side() const {return _side;}
    ArmyType army() const {return _army;}
    char stones() const {return _stones;}
    const std::list<Piece>& pieces() const {return _pieces;}

    //More useful functions
    //Stone functions
    //bidStones returns false if the player does not have enough stones
    void grantStone();
    bool bidStones(char num);

    //Sets up the player's army according to other parameters
    void setupArmy();

    //Notify the player that a piece has been captured
    //This sets its position off the board instead of deleting it
    //The Board itself also needs to know about this
    void pieceCaptured(Piece* p);
    
  private:
    //A player could have a name
    std::string _name;

    //A player plays on a board
    Board* _board;

    //A player plays on one side
    SideType _side;

    //A player plays with one army
    ArmyType _army;

    //A player has a bunch of pieces
    std::list<Piece> _pieces;

    //A player has a certain number of stones
    char _stones;

  };

} //Namespace

#endif
