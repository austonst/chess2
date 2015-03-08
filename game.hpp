/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Game Class Header-----
  Auston Sterling
  austonst@gmail.com

  A class to manage the high level aspects of a game of Chess 2: taking
  turns, placing bids, etc.
*/

#ifndef _game_hpp_
#define _game_hpp_

#include "army.hpp"
#include "board.hpp"

namespace c2
{

  const std::size_t NUM_GAMESTATES = 22;

  enum class GameStateType : std::uint8_t
  {
    SET_BOARD,
    BOTH_CHOOSE_ARMY,
    WHITE_CHOOSE_ARMY,
    BLACK_CHOOSE_ARMY,
    CONFIRM_START,
    WHITE_MOVE,
    BLACK_MOVE,
    WHITE_KINGMOVE,
    BLACK_KINGMOVE,
    WHITE_DUEL,
    BLACK_DUEL,
    BOTH_BID,
    WHITE_BID,
    BLACK_BID,
    WHITE_PROMOTE,
    BLACK_PROMOTE,
    WHITE_WIN_CHECKMATE,
    BLACK_WIN_CHECKMATE,
    WHITE_WIN_MIDLINE,
    BLACK_WIN_MIDLINE,
    DRAW_THREEFOLD,
    DRAW_FIFTYMOVE
  };
  inline std::uint8_t num(GameStateType s) {return static_cast<std::uint8_t>(s);}

  enum class GameReturnType : std::uint8_t
  {
    SUCCESS,
    INVALID_STATE,
    INVALID_PARAM,
    WRONG_SIDE,
    INVALID_MOVE,
    INVALID_STONES,
    NOT_ENOUGH_STONES,
    INVALID_PROMOTE_TYPE,
    GAME_OVER_WHITE_WIN,
    GAME_OVER_BLACK_WIN,
    GAME_OVER_DRAW
  };

  class Game
  {
  public:
    //Constructors
    //Immediately sets up a game
    Game(Board* b, ArmyType white, ArmyType black);

    //Sets up the board but requires armies to be set later
    Game(Board* b);

    //Sets up a game, but requires a Board to be passed in later
    Game();

    //Main state-progressing functions
    //Sets the board pointer. This can only be done before CONFIRM_START
    GameReturnType setBoard(Board* b);

    //Sets an army type. This can only be done before CONFIRM_START
    GameReturnType setArmy(SideType side, ArmyType army);

    //Confirm the start of the game. After this, board and armies are set
    GameReturnType start();

    //Make a move. Can be for either side and can be king moves
    GameReturnType move(const Move& m);

    //Decide whether or not to duel
    GameReturnType startDuel(bool d);

    //Make a bid in a duel
    GameReturnType bid(SideType side, std::uint8_t stones);

    //Promotes a pawn if one just reached the back
    GameReturnType promote(PieceType newType);

    //Other helpful functions
    //Provides the set of possible positions a piece can move to
    std::set<Position> possibleMoves(Position pos);

    //Accessors
    GameStateType state() const {return _state;}
    std::uint8_t stones(SideType side) const;
    bool canCastle(SideType side, bool kingSide) const;
    ArmyType army(SideType side) const;
    size_t numMoves() const {return _moves.size();}
    Move getMove(size_t i) const {return _moves[i];}
    
  private:
    //Helper for pre-game state settings
    //Call after changing the board or armies
    void setPreGameState();

    //Helper for things that need to happen once, but after a duel if one
    void endTurnThings();

    //Helper for the things that need to be done when a pawn is captured
    //The side passed in is the side that gets the stones, that is,
    //the opposite of the side of the pawn
    void pawnCaptured(SideType side, std::uint8_t count = 1);
    
    //A board containing current piece locations
    Board* _board;

    //Stone counts for the two players
    std::uint8_t _whiteStones;
    std::uint8_t _blackStones;

    //The armies used by each side
    ArmyType _whiteArmy;
    ArmyType _blackArmy;

    //Complete move history
    std::vector<Move> _moves;

    //Current player bets, if duelling
    //A 3 indicates that no choice has been made yet
    std::uint8_t _whiteBet;
    std::uint8_t _blackBet;

    //Some info to track as one turn progresses
    Move _currentMove;
    Piece _justTaken;
    bool _isKingTurn;

    //Each side's ability to castle on each side, if using classic army
    bool _whiteKingCastle;
    bool _whiteQueenCastle;
    bool _blackKingCastle;
    bool _blackQueenCastle;

    //The number of moves since a pawn moved or a piece was captured
    std::uint8_t _fiftyMoveRule;

    //The current game state
    GameStateType _state;

    //If this is set, this is a dummy game set up for possibleMoves
    //and no further recursion should be made.
    bool _dummy;
  };

} //Namespace

#endif
