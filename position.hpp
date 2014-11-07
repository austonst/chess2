/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Position Class Header-----
  Auston Sterling
  austonst@gmail.com

  A class to represent a position on a chessboard.
*/

#ifndef _position_hpp_
#define _position_hpp_

#include <string>

namespace c2
{

  class Position
  {
  public:
    //Constructors
    //Default constructor will initialize to (0,0), an invalid position
    Position();
  
    //x and y must be in [1,8]
    Position(char x, char y);
  
    //First character must be in ['a','h'], second in range ['1','8']
    //If the string is of the wrong length or a character is out of range,
    //it will default to 0, a position off the board
    Position(const std::string& notate);

    //Accessors/mutators
    char& x() {return _x;}
    char& y() {return _y;}
    char x() const {return _x;}
    char y() const {return _y;}

    //Can access the notation for the position
    std::string notation() const;

    //A valid position is one that's actually on the board
    bool isValid() const;

  private:
    //Coordinates
    char _x;
    char _y;
  
  };

  //Positions can be added and subtracted
  Position operator+(const Position& p1, const Position& p2);
  Position& operator+=(Position& p1, const Position& p2);
  Position operator-(const Position& p1, const Position& p2);
  Position& operator-=(Position& p1, const Position& p2);

  //Positions need to be comparable so they can be used in sets
  bool operator<(const Position& p1, const Position& p2);

  //I shouldn't need these, but oh well
  bool operator==(const Position& p1, const Position& p2);
  bool operator!=(const Position& p1, const Position& p2);

} //namespace

#endif
