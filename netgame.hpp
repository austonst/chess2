/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----NetGame Class Header-----
  Auston Sterling
  austonst@gmail.com

  A class extending the Chess 2 Game to automatically synchronize between
  two players on separate clients.

  It is assmed that a Board* will be set before connecting, so setBoard is local
  Messages start with one byte, 0xCE (it's the hex values in the word chess)
  The second byte represents the function applied or type of status message
  0 = setArmy, 1 = start, 2 = move, 3 = startDuel, 4 = bid, 5 = promote
  6 = state, 7 = version

  Followed by:
  setArmy 2B   - 1B side, 1B army
  start 0B     - none
  move 6B      - 1B startx, 1B starty, 1B endx, 1B endy, 1B type, 1B side
  startDuel 1B - 1B (0 == no, 1 == yes)
  bid 2B       - 1B side, 1B stones
  promote 1B   - 1B type
  state 1B     - 1B state
  version 2B   - 2B version big-endian
*/

#ifndef _netgame_hpp_
#define _netgame_hpp_

#include <queue>
#include <vector>
#include <string>

#include "game.hpp"

namespace c2
{

  typedef std::vector<std::uint8_t> Message;
  const std::string DEFAULT_PORT = "38519";
  const std::uint8_t MAGIC_NUM = 0xCE;
  const std::uint16_t NET_VERSION = 1;
  const std::size_t HEADER_SIZE = 2;
  
  class NetGame
  {
  public:
    //Constructors
    NetGame(Board* b = nullptr, std::string ip = std::string(),
            std::string port = DEFAULT_PORT);

    //Network functions
    //Connects to the specified address and starts a thread
    bool connectStart(std::string ip, std::string port = DEFAULT_PORT);
    
    //Listens for connections, starting a thread when we get one
    bool listenStart(std::string port = DEFAULT_PORT);

    //Disconnects from a running connection
    void disconnect() {_killThread = true;}

    //Check if the thread has been killed
    bool connected() {return !_killThread;}

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

    //Pass through to the game itself
    std::set<Position> possibleMoves(Position pos) {return _game.possibleMoves(pos);}
    GameStateType state() {return _game.state();}
    std::uint8_t stones(SideType side) const {return _game.stones(side);}
    ArmyType army(SideType side) const {return _game.army(side);}
    
  private:
    //Handles the back and forth communication
    friend void* connect_thread(void* data);

    //The game itself, which we're synchronizing
    Game _game;

    //The queue of messages going out
    std::queue<Message> _outMessage;

    //The socket fd
    int _sockfd;

    //The IP address of the peer
    //std::vector<std::uint8_t> _address;

    //When set, the thread will kill itself asap
    bool _killThread;
    
  };

  void* connect_thread(void* data);

} //Namespace

#endif
