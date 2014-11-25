/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----Game Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  A class to manage the high level aspects of a game of Chess 2: taking
  turns, placing bids, etc.
*/

#include "game.hpp"
#include "piece.hpp"

#include <cmath>

namespace c2
{

  //Immediately sets up a game
  Game::Game(Board* b, ArmyType white, ArmyType black) :
    _board(b), _whiteArmy(white), _blackArmy(black), _dummy(false)
  {
    //Check which inputs are properly set
    //Choose the corresponding game state
    setPreGameState();
  }

  //Sets up the board but requires armies to be set later
  Game::Game(Board* b) : _board(b), _whiteArmy(ArmyType::NONE),
                         _blackArmy(ArmyType::NONE), _dummy(false)
  {
    if (b)
      {
        _state = GameStateType::BOTH_CHOOSE_ARMY;
      }
    else
      {
        _state = GameStateType::SET_BOARD;
      }
  }

  //Sets up a game, but requires a Board to be passed in later
  Game::Game() :
    _board(nullptr), _whiteArmy(ArmyType::NONE), _blackArmy(ArmyType::NONE),
    _state(GameStateType::SET_BOARD), _dummy(false) {}

  void Game::setPreGameState()
  {
    if (_board)
      {
        if (_whiteArmy == ArmyType::NONE)
          {
            if (_blackArmy == ArmyType::NONE)
              {
                _state = GameStateType::BOTH_CHOOSE_ARMY;
              }
            else
              {
                _state = GameStateType::WHITE_CHOOSE_ARMY;
              }
          }
        else if (_blackArmy == ArmyType::NONE)
          {
            _state = GameStateType::BLACK_CHOOSE_ARMY;
          }
        else
          {
            _state = GameStateType::CONFIRM_START;
          }
      }
    else
      {
        _state = GameStateType::SET_BOARD;
      }
  }

  void Game::pawnCaptured(SideType side, std::uint8_t count)
  {
    if (side == SideType::WHITE)
      {
        _whiteStones += count;
        if (_whiteStones > 6) _whiteStones = 6;
      }
    else if (side == SideType::BLACK)
      {
        _blackStones += count;
        if (_blackStones > 6) _blackStones = 6;
      }
  }
  
  GameReturnType Game::setBoard(Board* b)
  {
    //Verify state
    if (_state > GameStateType::CONFIRM_START)
      {
        return GameReturnType::INVALID_STATE;
      }

    //Make the change
    _board = b;

    //Update state
    setPreGameState();
    return GameReturnType::SUCCESS;
  }

  GameReturnType Game::setArmy(SideType side, ArmyType army)
  {
    //Verify state
    if (_state > GameStateType::CONFIRM_START)
      {
        return GameReturnType::INVALID_STATE;
      }

    //Make the change
    if (side == SideType::WHITE)
      {
        _whiteArmy = army;
      }
    else if (side == SideType::BLACK)
      {
        _blackArmy = army;
      }
    else //SideType::NONE
      {
        return GameReturnType::INVALID_PARAM;
      }

    //Update state
    setPreGameState();
    return GameReturnType::SUCCESS;
  }

  GameReturnType Game::start()
  {
    //Verify state is CONFIRM_START
    if (_state != GameStateType::CONFIRM_START)
      {
        return GameReturnType::INVALID_STATE;
      }

    //Make sure all variables are in correct starting state
    _board->clear();
    _board->addArmy(SideType::WHITE, _whiteArmy);
    _board->addArmy(SideType::BLACK, _blackArmy);
    _whiteStones = 3;
    _blackStones = 3;
    _whiteBet = 3;
    _blackBet = 3;
    _moves.clear();
    _whiteKingCastle = _whiteQueenCastle =
      _blackKingCastle = _blackQueenCastle = true;
    _fiftyMoveRule = 0;

    //Update state
    _state = GameStateType::WHITE_MOVE;
    return GameReturnType::SUCCESS;
  }

  GameReturnType Game::move(const Move& m)
  {
    //Verify state
    if ((m.side == SideType::WHITE &&
         (_state == GameStateType::BLACK_MOVE ||
          _state == GameStateType::BLACK_KINGMOVE)) ||
        (m.side == SideType::BLACK &&
         (_state == GameStateType::WHITE_MOVE ||
          _state == GameStateType::WHITE_KINGMOVE)))
      {
        return GameReturnType::WRONG_SIDE;
      }
    
    if ((m.side == SideType::WHITE && _state != GameStateType::WHITE_MOVE &&
         _state != GameStateType::WHITE_KINGMOVE) ||
        (m.side == SideType::BLACK &&_state != GameStateType::BLACK_MOVE &&
         _state != GameStateType::BLACK_KINGMOVE))
      {
        return GameReturnType::INVALID_STATE;
      }

    //Verify the piece exists
    Piece currentPiece = (*_board)(m.start);
    if (currentPiece.side() != m.side)
      {
        return GameReturnType::WRONG_SIDE;
      }
    if (currentPiece.pos() != m.start || currentPiece.type() != m.type)
      {
        return GameReturnType::INVALID_MOVE;
      }

    //Verify that the destination is feasible
    if (!(m.end.isValid()))
      {
        return GameReturnType::INVALID_MOVE;
      }
    std::set<Position> possMoves = possibleMoves(m.start);
    if (possMoves.find(m.end) == possMoves.end())
      {
        return GameReturnType::INVALID_MOVE;
      }

    //If this is a kingmove, verify that it is in fact the king moving
    if (_state == GameStateType::WHITE_KINGMOVE ||
        _state == GameStateType::BLACK_KINGMOVE)
      {
        if (m.type != PieceType::TKG_WARRKING)
          {
            return GameReturnType::INVALID_MOVE;
          }
      }

    //The move looks good, let's carry it out
    //See if there's a piece at the end of the move getting taken, then move
    _justTaken = (*_board)(m.end);
    _currentMove = m;
    _board->move(m);
    _moves.push_back(m);

    //If this is a warrior king "moving" to the same spot, it's a whirlwind
    if (m.type == PieceType::TKG_WARRKING && m.start == m.end)
      {
        for (char i = -1; i < 2; i++)
          {
            for (char j = -1; j < 2; j++)
              {
                if (i == 0 && j == 0) continue;
                Piece dest = (*_board)(Position(m.end.x()+i,m.end.y()+j));
                if (dest.type() == PieceType::CLA_PAWN ||
                    dest.type() == PieceType::NEM_PAWN)
                  {
                    pawnCaptured(m.side);
                  }
                _board->destroy(dest.pos());
              }
          }
        //The king did not capture itself
        _justTaken = Piece();
      }

    //Pawns capturing through en passant don't move to the space they capture
    Move lastMove;
    if (_moves.size() > 1) lastMove = _moves[_moves.size()-2];
    if ((m.type == PieceType::CLA_PAWN || m.type == PieceType::NEM_PAWN) &&
        (lastMove.type == PieceType::CLA_PAWN ||
         lastMove.type == PieceType::NEM_PAWN) &&
        lastMove.start.x() == lastMove.end.x() &&
        std::abs(lastMove.start.y() - lastMove.end.y()) == 2 &&
        std::abs(m.end.y() - lastMove.start.y()) == 1 &&
        std::abs(m.end.y() - lastMove.end.y()) == 1 &&
        m.side != lastMove.side)
      {
        //Move to the passed pawn's space and back
        _justTaken = (*_board)(lastMove.end);
        _board->move(Move(m.end,lastMove.end,m.type,m.side));
        _board->move(Move(lastMove.end,m.end,m.type,m.side));
      }

    //Tigers and elephants will require special handling, but that should be
    //done after betting is complete

    //Update state, either to promotion, duelling, king move, checkmate,
    //midline, draw, or other player's turn
    //Check for midline victory
    std::vector<Position> kings = _board->getKing(m.side);
    bool midlineWin = true;
    for (size_t i = 0; i < kings.size(); i++)
      {
        if ((m.side == SideType::WHITE && kings[i].y() < 5) ||
            (m.side == SideType::BLACK && kings[i].y() > 4))
          {
            midlineWin = false;
          }
      }
    if (midlineWin)
      {
        if (m.side == SideType::WHITE)
          {
            _state = GameStateType::WHITE_WIN_MIDLINE;
            return GameReturnType::GAME_OVER_WHITE_WIN;
          }
        else
          {
            _state = GameStateType::BLACK_WIN_MIDLINE;
            return GameReturnType::GAME_OVER_BLACK_WIN;
          }
      }

    //Update fifty move rule state
    if (m.type == PieceType::CLA_PAWN || m.type == PieceType::NEM_PAWN ||
        _justTaken.type() != PieceType::NONE)
      {
        _fiftyMoveRule = 0;
      }
    else
      {
        _fiftyMoveRule++;
      }

    //Check for draws
    //LET'S ADD THREEFOLD REPETITION LATER BECAUSE NEED TO STORE ALL BOARD STATE?
    if (_fiftyMoveRule >= 100)
      {
        _state = GameStateType::DRAW_FIFTYMOVE;
        return GameReturnType::GAME_OVER_DRAW;
      }

    //Check if we're castling
    if (m.type == PieceType::CLA_KING)
      {
        int bwy = 1;
        if (m.side == SideType::BLACK) bwy = 8;
        //Castling queen side
        if (m.start.x() - m.end.x() > 1)
          {
            //Move the queen side rook over
            _board->move(Move(Position(1,bwy), Position(3,bwy),
                              PieceType::CLA_ROOK, m.side));
          }
        else if (m.start.x() - m.end.x() < -1) //King side
          {
            //Move the king side rook over
            _board->move(Move(Position(8,bwy), Position(6,bwy),
                              PieceType::CLA_ROOK, m.side));
          }
      }

    //Update ability to castle
    if (m.type == PieceType::CLA_KING)
      {
        //Can't castle with a king that's moved
        if (m.side == SideType::WHITE)
          {
            _whiteKingCastle = _whiteQueenCastle = false;
          }
        else
          {
            _blackKingCastle = _blackKingCastle = false;
          }
      }
    else if (m.type == PieceType::CLA_ROOK)
      {
        //Can't castle with a rook that's moved
        if (m.start == Position(1,1) && m.side == SideType::WHITE)
          {
            _whiteQueenCastle = false;
          }
        else if (m.start == Position(8,1) && m.side == SideType::WHITE)
          {
            _whiteKingCastle = false;
          }
        else if (m.start == Position(1,8) && m.side == SideType::BLACK)
          {
            _blackQueenCastle = false;
          }
        else if (m.start == Position(8,8) && m.side == SideType::BLACK)
          {
            _blackKingCastle = false;
          }
      }
    
    //Check for duelling
    if (_justTaken.type() != PieceType::NONE)
      {
        //A piece was taken; move to duelling unless defender has no stones
        //or the attacker was a king
        bool rankPayExtra = pieceRank(_justTaken.type()) > pieceRank(m.type);
        bool kingTook = m.type == PieceType::CLA_KING ||
          m.type == PieceType::ANY_KING ||
          m.type == PieceType::TKG_WARRKING;
        
        if (m.side == SideType::WHITE && m.side != _justTaken.side() &&
            !kingTook &&
            (_blackStones>1 || (_blackStones==1 && !rankPayExtra)))
          {
            _state = GameStateType::BLACK_DUEL;
            return GameReturnType::SUCCESS;
          }
        else if (m.side == SideType::BLACK && m.side != _justTaken.side() &&
                 !kingTook &&
                 (_whiteStones>1 || (_whiteStones==1 && !rankPayExtra)))
          {
            _state = GameStateType::WHITE_DUEL;
            return GameReturnType::SUCCESS;
          }
      }

    //Wrap up the end of the turn
    endTurnThings();
    return GameReturnType::SUCCESS;
  }

  void Game::endTurnThings()
  {
    Move m = _currentMove;
    Piece postDuelPiece = (*_board)(m.end);
    
    if (_justTaken.type() != PieceType::NONE)
      {
        //If it was a pawn taken, the attack gains a stone
        if (_justTaken.type() == PieceType::CLA_PAWN ||
            _justTaken.type() == PieceType::NEM_PAWN)
          {
            pawnCaptured(m.side);
          }
    
        //We also need to handle tigers and elephants now, if survived duel
        //Need to check both m and postDuelPiece in case of promotion
        if (postDuelPiece.type() == PieceType::ANI_TIGER &&
            m.type == PieceType::ANI_TIGER)
          {
            //Move back to original position
            _board->move(Move(m.end,m.start,m.type,m.side));
          }
        else if (postDuelPiece.type() == PieceType::ANI_ELEPHANT &&
                 m.type == PieceType::ANI_ELEPHANT)
          {
            //A piece was taken, we need to move step by step taking pieces
            char distance = std::abs(m.start.x()-m.end.x()) +
              std::abs(m.start.y()-m.end.y());
                
            //Keep moving
            Position currentPos = m.end;
            while (distance < 3)
              {
                Position newPos;
                if (m.start.x() < m.end.x())
                  {
                    newPos = Position(currentPos.x()+1, currentPos.y());
                  }
                else if (m.start.x() > m.end.x())
                  {
                    newPos = Position(currentPos.x()-1, currentPos.y());
                  }
                else if (m.start.y() < m.end.y())
                  {
                    newPos = Position(currentPos.x(), currentPos.y()+1);
                  }
                else
                  {
                    newPos = Position(currentPos.x(), currentPos.y()-1);
                  }

                //Make the move
                _board->move(Move(currentPos, newPos, m.type, m.side));
                currentPos = newPos;
                distance++;
              }
          }
      }

    //Check for promotion
    if ((postDuelPiece.type() == PieceType::CLA_PAWN ||
         postDuelPiece.type() == PieceType::NEM_PAWN) &&
        ((m.end.y() == 8 && m.side == SideType::WHITE) ||
         (m.end.y() == 1 && m.side == SideType::BLACK)))
      {
        if (m.end.y() == 1 && m.side == SideType::BLACK)
          {
            _state = GameStateType::BLACK_PROMOTE;
          }
        else if (m.end.y() == 8 && m.side == SideType::WHITE)
          {
            _state = GameStateType::WHITE_PROMOTE;
          }
        return;
      }

    //Check for checkmate
    //If the opponent has no possible moves, that's mate.
    //In some odd situations, the person who just moved could have put himself
    //in check. In this case, the opponent immediately wins.
    //If any enemy piece can move to friendly king, that's mate
    if (!_dummy)
      {
        std::list<Position> enemyPos = _board->getPieces(otherSide(m.side));
        std::vector<Position> friendKing = _board->getKing(m.side);
        SideType winner = m.side;
        for (auto i = enemyPos.begin(); i != enemyPos.end(); i++)
          {
            //If the opponent's piece can move, the opponent is not mated
            std::set<Position> poss = possibleMoves(*i);
            if (poss.size() != 0)
              {
                //If that piece can move to the mover's king, mover is mated
                for (auto j = friendKing.begin(); j != friendKing.end(); j++)
                  {
                    if (poss.find(*j) != poss.end())
                      {
                        winner = otherSide(m.side);
                      }
                  }
                if (winner == m.side) winner = SideType::NONE;
              }
          }
        
        if (winner == SideType::WHITE)
          {
            _state = GameStateType::WHITE_WIN_CHECKMATE;
            return;
          }
        else if (winner == SideType::BLACK)
          {
            _state = GameStateType::BLACK_WIN_CHECKMATE;
            return;
          }
      }
      
    //We just move to next turn; figure out which one
    if (_state == GameStateType::WHITE_MOVE)
      {
        if (hasKingTurn(_whiteArmy))
          {
            _state = GameStateType::WHITE_KINGMOVE;
            _isKingTurn = true;
          }
        else
          {
            _state = GameStateType::BLACK_MOVE;
          }
      }
    else if (_state == GameStateType::BLACK_MOVE)
      {
        if (hasKingTurn(_blackArmy))
          {
            _state = GameStateType::BLACK_KINGMOVE;
            _isKingTurn = true;
          }
        else
          {
            _state = GameStateType::WHITE_MOVE;
          }
      }
    else if (_state == GameStateType::WHITE_KINGMOVE)
      {
        _state = GameStateType::BLACK_MOVE;
        _isKingTurn = false;
      }
    else if (_state == GameStateType::BLACK_KINGMOVE)
      {
        _state = GameStateType::WHITE_MOVE;
        _isKingTurn = false;
      }
    else if (_state == GameStateType::WHITE_DUEL)
      {
        //We are here because black attacked and white declined a duel
        if (hasKingTurn(_blackArmy) && !_isKingTurn)
          {
            _state = GameStateType::BLACK_KINGMOVE;
            _isKingTurn = true;
          }
        else
          {
            _state = GameStateType::WHITE_MOVE;
          }
      }
    else if (_state == GameStateType::BLACK_DUEL)
      {
        //We are here because white attacked and black declined a duel
        if (hasKingTurn(_whiteArmy) && !_isKingTurn)
          {
            _state = GameStateType::WHITE_KINGMOVE;
            _isKingTurn = true;
          }
        else
          {
            _state = GameStateType::BLACK_MOVE;
          }
      }
    else //State is *_BID, state gives us no cues to next player
      {
        if (m.side == SideType::WHITE)
          {
            if (hasKingTurn(_whiteArmy) && !_isKingTurn)
              {
                _state = GameStateType::WHITE_KINGMOVE;
              }
            else
              {
                _state = GameStateType::BLACK_MOVE;
              }
          }
        else
          {
            if (hasKingTurn(_blackArmy) && !_isKingTurn)
              {
                _state = GameStateType::BLACK_KINGMOVE;
              }
            else
              {
                _state = GameStateType::WHITE_MOVE;
              }
          }
      }
  }

  GameReturnType Game::startDuel(bool d)
  {
    //Verify state
    if (_state != GameStateType::WHITE_DUEL &&
        _state != GameStateType::BLACK_DUEL)
      {
        return GameReturnType::INVALID_STATE;
      }

    if (!d)
      {
        //Skipping the duel, we finish up the end of turn things
        endTurnThings();
        return GameReturnType::SUCCESS;
      }
    //Compare the ranks of the two pieces for extra cost possibility
    //We should never be here if the defender can't pay the fee, so don't check
    if (pieceRank(_moves[_moves.size()-1].type) > pieceRank(_justTaken.type()))
      {
        if (_state == GameStateType::WHITE_DUEL)
          {
            _whiteStones--;
          }
        else
          {
            _blackStones--;
          }
      }

    //3 indicates "no bid"
    _whiteBet = _blackBet = 3;

    //Duel starts, begin bidding
    _state = GameStateType::BOTH_BID;
    return GameReturnType::SUCCESS;
  }

  GameReturnType Game::bid(SideType side, std::uint8_t stones)
  {
    //Verify state
    if (_state != GameStateType::BOTH_BID &&
        _state != GameStateType::WHITE_BID &&
        _state != GameStateType::BLACK_BID)
      {
        return GameReturnType::INVALID_STATE;
      }

    //Make sure stones is in the right range
    if (stones > 2)
      {
        return GameReturnType::INVALID_STONES;
      }

    if ((side == SideType::WHITE && stones > _whiteStones) ||
        (side == SideType::BLACK && stones > _blackStones))
      {
        return GameReturnType::NOT_ENOUGH_STONES;
      }

    //Bid is good, store it
    if (side == SideType::WHITE)
      {
        _whiteBet = stones;
      }
    else if (side == SideType::BLACK)
      {
        _blackBet = stones;
      }
    else //SideType::NONE
      {
        return GameReturnType::INVALID_PARAM;
      }

    //If both have bid, resolve
    if (_whiteBet < 3 && _blackBet < 3)
      {
        _whiteStones -= _whiteBet;
        _blackStones -= _blackBet;

        if ((_justTaken.side() == SideType::WHITE && _whiteBet > _blackBet) ||
            (_justTaken.side() == SideType::BLACK && _blackBet > _whiteBet))
          {
            Piece alsoDead = (*_board)(_justTaken.pos());
            _board->destroy(_justTaken.pos());

            //Give the defender a stone if the attacker was a pawn
            if (alsoDead.type() == PieceType::CLA_PAWN ||
                alsoDead.type() == PieceType::NEM_PAWN)
              {
                if (_moves[_moves.size()-1].side == SideType::WHITE)
                  {
                    pawnCaptured(SideType::BLACK);
                  }
                else
                  {
                    pawnCaptured(SideType::WHITE);
                  }
              }
          }

        //Bid resolved, we move to next turn
        endTurnThings();
      }
    else if (_whiteBet > 2 && _blackBet > 2)
      {
        _state = GameStateType::BOTH_BID;
      }
    else if (_whiteBet > 2 && _blackBet < 3)
      {
        _state = GameStateType::WHITE_BID;
      }
    else if (_whiteBet < 3 && _blackBet > 2)
      {
        _state = GameStateType::BLACK_BID;
      }

    return GameReturnType::SUCCESS;
  }

  GameReturnType Game::promote(PieceType newType)
  {
    //Verify state
    if (_state != GameStateType::WHITE_PROMOTE &&
        _state != GameStateType::BLACK_PROMOTE)
      {
        return GameReturnType::INVALID_STATE;
      }

    //We can only reach promote state when a pawn moves to end, so don't check
    //Verify the promoted type is part of the army
    ArmyType army;
    if (_state == GameStateType::WHITE_PROMOTE)
      {
        army = _whiteArmy;
      }
    else
      {
        army = _blackArmy;
      }

    if (ARMY_PROMOTE[num(army)].find(newType) == ARMY_PROMOTE[num(army)].end())
      {
        return GameReturnType::INVALID_PROMOTE_TYPE;
      }

    //Replace the piece
    _board->promote(_currentMove.end, newType);

    //Update state
    endTurnThings();
    return GameReturnType::SUCCESS;
  }
  
  //This'll be a fun one...
  std::set<Position> Game::possibleMoves(Position pos)
  {
    //Get a bunch of data one time so it can just be reused
    Board* b = _board;
    Piece p = (*b)(pos);
    SideType sf = p.side();
    SideType se = otherSide(sf);
    std::list<Position> friendPos = b->getPieces(sf);
    std::vector<Position> enemyKings = b->getKing(se);
    
    //If this is an empowered piece, see what extra move types it has
    std::list<MoveType> types = MOVE_TYPES[num(p.type())];
    if (p.type() == PieceType::EMP_ROOK || p.type() == PieceType::EMP_KNIGHT ||
        p.type() == PieceType::EMP_BISHOP)
    {
      for (int i = -1; i < 2; i++)
        {
          for (int j = -1; j < 2; j++)
            {
              if (std::abs(i) + std::abs(j) != 1) continue;
              Position check(p.pos().x() + i, p.pos().y() + j);
              PieceType neighbor = (*b)(check).type();
              if (neighbor == PieceType::EMP_ROOK)
                {
                  types.push_back(MoveType::ROOK_CLA);
                }
              if (neighbor == PieceType::EMP_KNIGHT)
                {
                  types.push_back(MoveType::KNIGHT_CLA);
                }
              if (neighbor == PieceType::EMP_BISHOP)
                {
                  types.push_back(MoveType::BISHOP_CLA);
                }
            }
        }
    }
    
    //Get the possible moves for this piece
    std::set<Position> moves;
    for (auto mt = types.begin(); mt != types.end(); mt++)
      {
        switch (*mt)
          {
          case MoveType::PAWN_CLA:
            {
              //Can't move forward if blocked by either side
              char pawnDir = p.side()==SideType::WHITE?1:-1;

              bool canForward = false;
              if ((*b)(p.pos() + Position(0,pawnDir)).type() == PieceType::NONE &&
                  (p.pos() + Position(0,pawnDir)).isValid())
                {
                  moves.insert(p.pos() + Position(0,pawnDir));
                  canForward = true;
                }

              //If we have never moved, we can take two steps
              bool nevermoved = true;
              for (size_t i = 0; i < _moves.size(); i++)
                {
                  if (_moves[i].start == p.pos() || _moves[i].end == p.pos())
                    {
                      nevermoved = false;
                    }
                }
              Position twosteps(0,pawnDir+pawnDir);
              if (canForward &&(p.pos() + twosteps).isValid() && nevermoved &&
                  (*b)(p.pos() + twosteps).type() == PieceType::NONE)
                {
                  moves.insert(p.pos() + twosteps);
                }

              //Can only move diagonally if enemy piece there or last move was ep
              Move lastMove;
              if (_moves.size() > 0) lastMove = _moves[_moves.size()-1];
              bool maybeEP = lastMove.type == PieceType::CLA_PAWN &&
                lastMove.side == se &&
                std::abs(lastMove.start.y()-lastMove.end.y()) == 2 &&
                std::abs(p.pos().y()+pawnDir - lastMove.start.y()) == 1 &&
                std::abs(p.pos().y()+pawnDir - lastMove.end.y()) == 1;
            
              if ((*b)(p.pos() + Position(1,pawnDir)).side() == se ||
                  (maybeEP && p.pos().x()+1==lastMove.end.x()))
                {
                  if ((p.pos() + Position(1,pawnDir)).isValid())
                    {
                      moves.insert(p.pos() + Position(1,pawnDir));
                    }
                }
              if ((*b)(p.pos() + Position(-1,pawnDir)).side() == se ||
                  (maybeEP && p.pos().x()-1==lastMove.end.x()))
                {
                  if ((p.pos() + Position(-1,pawnDir)).isValid())
                    {
                      moves.insert(p.pos() + Position(-1,pawnDir));
                    }
                }
              break;
            }

          case MoveType::PAWN_NEM:
            //Nemesis pawns may be able to make nemesis moves
            for (size_t j = 0; j < enemyKings.size(); j++)
              {
                //Let's just cycle over all eight options
                //It's long, I know. I'll fix it some other time
                bool right = enemyKings[j].x() > p.pos().x();
                bool left = enemyKings[j].x() < p.pos().x();
                bool up = enemyKings[j].y() > p.pos().y();
                bool down = enemyKings[j].y() < p.pos().y();
                if (right &&
                    (*b)(p.pos() + Position(1,0)).type() == PieceType::NONE &&
                    (p.pos() + Position(1,0)).isValid())
                  {
                    moves.insert(p.pos() + Position(1,0));
                  }
                if (right && up &&
                    (*b)(p.pos() + Position(1,1)).type() == PieceType::NONE &&
                    (p.pos() + Position(1,1)).isValid())
                  {
                    moves.insert(p.pos() + Position(1,1));
                  }
                if (up &&
                    (*b)(p.pos() + Position(0,1)).type() == PieceType::NONE &&
                    (p.pos() + Position(0,1)).isValid())
                  {
                    moves.insert(p.pos() + Position(0,1));
                  }
                if (left && up &&
                    (*b)(p.pos() + Position(-1,1)).type() == PieceType::NONE &&
                    (p.pos() + Position(-1,1)).isValid())
                  {
                    moves.insert(p.pos() + Position(-1,1));
                  }
                if (left &&
                    (*b)(p.pos() + Position(-1,0)).type() == PieceType::NONE &&
                    (p.pos() + Position(-1,0)).isValid())
                  {
                    moves.insert(p.pos() + Position(-1,0));
                  }
                if (left && down &&
                    (*b)(p.pos() + Position(-1,-1)).type() == PieceType::NONE &&
                    (p.pos() + Position(-1,-1)).isValid())
                  {
                    moves.insert(p.pos() + Position(-1,-1));
                  }
                if (down &&
                    (*b)(p.pos() + Position(0,-1)).type() == PieceType::NONE &&
                    (p.pos() + Position(0,-1)).isValid())
                  {
                    moves.insert(p.pos() + Position(0,-1));
                  }
                if (right && down &&
                    (*b)(p.pos() + Position(1,-1)).type() == PieceType::NONE &&
                    (p.pos() + Position(1,-1)).isValid())
                  {
                    moves.insert(p.pos() + Position(1,-1));
                  }
              }
            break;
            
          case MoveType::ROOK_CLA:
            for (Position dir :
              {Position(0,1), Position(1,0), Position(0,-1), Position(-1,0)})
              {
                Position step = p.pos();
                while((step+=dir).isValid())
                  {
                    //If there's a friendly, stop before adding this spot
                    if ((*b)(step).side() == sf)
                      {
                        break;
                      }
                    moves.insert(step);
                    //If there's an enemy, stop after adding this spot
                    if ((*b)(step).side() == se)
                      {
                        break;
                      }
                  }
              }
            break;
            
          case MoveType::ROOK_GHOST:
            {
              for (char i = 1; i < 9; i++)
                {
                  for (char j = 1; j < 9; j++)
                    {
                      if ((*b)(Position(i,j)).type() == PieceType::NONE)
                        {
                          moves.insert(Position(i,j));
                        }
                    }
                }
              break;
            }
            
          case MoveType::ROOK_ELEPHANT:
            {
              for (Position dir :
                {Position(0,1), Position(1,0), Position(0,-1), Position(-1,0)})
                {
                  Position step = p.pos();
                  for (char i = 0; i < 3 && (step+=dir).isValid(); i++)
                    {
                      moves.insert(step);
                      //If there's any piece, taking it would cause a rampage
                      Piece cap = (*b)(step);
                      if (cap.type() != PieceType::NONE)
                        {
                          //Check this spot and all rampaged spots for validity
                          Position rampageStep = step;
                          while (i < 3)
                            {
                              //Cannot capture nemesis, ghost, or friendly king
                              if ((cap.type() == PieceType::CLA_KING ||
                                   cap.type() == PieceType::ANY_KING ||
                                   cap.type() == PieceType::TKG_WARRKING ||
                                   cap.type() == PieceType::NEM_QUEEN ||
                                   cap.type() == PieceType::RPR_GHOST) &&
                                  cap.side() == sf)
                                {
                                  moves.erase(step);
                                  break;
                                }

                              //Cannot go off the board
                              if (!rampageStep.isValid())
                                {
                                  moves.erase(step);
                                  break;
                                }

                              //Advance position
                              i++;
                              rampageStep += dir;
                              cap = (*b)(rampageStep);
                            }

                          break;
                        }
                    }
                }
              break;
            }
            
          case MoveType::KNIGHT_CLA:
          case MoveType::KNIGHT_WILDHORSE:
            {
              for (Position offset :
                {Position(-2,-1), Position(-2,1), Position(-1,2), Position(-1,-2),
                    Position(1,-2), Position(1,2), Position(2,-1), Position(2,1)})
                {
                  //Wild horses can capture friends, but not friend kings
                  if ((p.pos()+offset).isValid() &&
                      ((*b)(p.pos()+offset).side() != sf ||
                       ((*mt) == MoveType::KNIGHT_WILDHORSE &&
                        (*b)(p.pos()+offset).type() != PieceType::ANY_KING)))
                    {
                      moves.insert(p.pos() + offset);
                    }
                }
              break;
            }
            
          case MoveType::BISHOP_CLA:
            for (Position dir :
              {Position(-1,-1), Position(1,-1), Position(-1,1), Position(1,1)})
              {
                Position step = p.pos();
                while ((step+=dir).isValid())
                  {
                    //If there's a friendly, stop before adding this spot
                    if ((*b)(step).side() == sf)
                      {
                        break;
                      }
                    moves.insert(step);
                    //If there's an enemy, stop after adding this spot
                    if ((*b)(step).side() == se)
                      {
                        break;
                      }
                  }
              }
            break;
            
          case MoveType::BISHOP_TIGER:
            {
              for (Position dir :
                {Position(-1,-1), Position(1,-1), Position(-1,1), Position(1,1)})
                {
                  Position step = p.pos();
                  for (char i = 0; i < 2 && (step+=dir).isValid(); i++)
                    {
                      //If there's a friendly, stop before adding this spot
                      if ((*b)(step).side() == sf)
                        {
                          break;
                        }
                      moves.insert(step);
                      //If there's an enemy, stop after adding this spot
                      if ((*b)(step).side() == se)
                        {
                          break;
                        }
                    }
                }
              break;
            }

          case MoveType::QUEEN_NEM:
            for (Position dir :
              {Position(-1,-1), Position(1,-1), Position(-1,1), Position(1,1),
                  Position(1,0), Position(0,1), Position(-1,0), Position(0,-1)})
              {
                Position step = p.pos();
                while ((step+=dir).isValid())
                  {
                    //Stop before hitting any piece but an enemy king
                    if ((*b)(step).side() == sf ||
                        ((*b)(step).side() == se &&
                         (*b)(step).type() != PieceType::CLA_KING &&
                         (*b)(step).type() != PieceType::ANY_KING &&
                         (*b)(step).type() != PieceType::TKG_WARRKING))
                      {
                        break;
                      }
                    moves.insert(step);
                    //If an enemy and we haven't stopped yet: is king; stop after
                    if ((*b)(step).side() == se)
                      {
                        break;
                      }
                  }
              }
            break;

          case MoveType::QUEEN_RPR:
            {
              int blackRowShift = 0;
              if (sf == SideType::BLACK)
                {
                  blackRowShift = 1;
                }
              for (int i = 1+blackRowShift; i < 8+blackRowShift; i++)
                {
                  for (int j = 1; j < 9; j++)
                    {
                      Position spot(j,i);
                      if ((*b)(spot).side() != sf &&
                          (*b)(spot).type() != PieceType::CLA_KING &&
                          (*b)(spot).type() != PieceType::ANY_KING &&
                          (*b)(spot).type() != PieceType::TKG_WARRKING)
                        {
                          moves.insert(spot);
                        }
                    }
                }
              break;
            }

          case MoveType::KING_2KG:
            {
              //See if they're next to each other and can't whirlwind
              bool whirlwind = true;
              for (int i = -1; i < 2; i++)
                {
                  for (int j = -1; j < 2; j++)
                    {
                      if (i != 0 || j != 0)
                        {
                          Position spot(pos.x() + i, pos.y() + j);
                          if ((*b)(spot).type() == PieceType::TKG_WARRKING)
                            {
                              whirlwind = false;
                            }
                        }
                    }
                }
              if (whirlwind)
                {
                  moves.insert(pos);
                }
              break;
            }

          case MoveType::KING_ANY:
            for (int i = -1; i < 2; i++)
              {
                for (int j = -1; j < 2; j++)
                  {
                    Position spot(pos.x() + i, pos.y() + j);
                    if (spot != pos && spot.isValid() &&
                        (*b)(spot).side() != sf)
                      {
                        moves.insert(spot);
                      }
                  }
              }
            break;

          case MoveType::KING_CLA:
            //We can't use canCastle here because may be using different Board
            if (sf == SideType::WHITE && _whiteKingCastle &&
                (*b)(Position(7,1)).side() == SideType::NONE &&
                (*b)(Position(6,1)).side() == SideType::NONE)
              {
                moves.insert(Position(7,1));
              }
            if (sf == SideType::WHITE && _whiteQueenCastle &&
                (*b)(Position(2,1)).side() == SideType::NONE &&
                (*b)(Position(3,1)).side() == SideType::NONE)
              {
                moves.insert(Position(2,1));
              }
            if (sf == SideType::BLACK && _blackKingCastle &&
                (*b)(Position(7,8)).side() == SideType::NONE &&
                (*b)(Position(6,8)).side() == SideType::NONE)
              {
                moves.insert(Position(7,8));
              }
            if (sf == SideType::WHITE && _blackQueenCastle &&
                (*b)(Position(2,8)).side() == SideType::NONE &&
                (*b)(Position(3,8)).side() == SideType::NONE)
              {
                moves.insert(Position(2,8));
              }
            break;
            
          default: break;
          }
      }

    //No matter what the move is, nothing can capture a ghost rook and only
    //a king can capture a nemesis queen
    for (auto i = moves.begin(); i != moves.end();)
      {
        if (((*b)(*i).type() == PieceType::RPR_GHOST) ||
            ((*b)(*i).type() == PieceType::NEM_QUEEN &&
             (p.type() != PieceType::CLA_KING &&
              p.type() != PieceType::ANY_KING &&
              p.type() != PieceType::TKG_WARRKING)))
          {
            moves.erase(i++);
          }
        else
          {
            ++i;
          }
      }

    //The remaining moves are all good unless the king would be checked after
    //Make game copy, make move on copy, call possibleMoves for enemy to see
    // if checked, if so, can't make that move
    if (!_dummy)
      {
        for (auto i = moves.begin(); i != moves.end();)
          {
            //Make the move
            Game gCopy = *this;
            gCopy._board = b->clone();
            gCopy._dummy = true;
            if (p.side() == SideType::WHITE)
              {
                gCopy._state = GameStateType::WHITE_MOVE;
              }
            else
              {
                gCopy._state = GameStateType::BLACK_MOVE;
              }
            gCopy.move(Move(pos, *i, p.type(), p.side()));

            //Special cases requiring manual progress through endTurnThings:
            //A pinned tiger taking a piece needs to resolve the duel to get back
            if (p.type() == PieceType::ANI_TIGER &&
                (gCopy.state() == GameStateType::BLACK_DUEL ||
                 gCopy.state() == GameStateType::WHITE_DUEL))
              {
                gCopy.startDuel(true);
                
                //The attacker can make the move even if he could or will lose
                //the duel, so assume attacker win
                gCopy.bid(se, 0);
                gCopy.bid(sf, 0);
              }
            
            std::vector<Position> friendKings = gCopy._board->getKing(sf);
            std::list<Position> enemyPos = gCopy._board->getPieces(se);
            bool needToInc = true;
            for (auto j = enemyPos.begin(); j != enemyPos.end();)
              {
                std::set<Position> enemyMoves = gCopy.possibleMoves(*j);
                for (size_t k = 0; k < friendKings.size(); k++)
                  {
                    //If the enemy piece can move to the king, it's check
                    if (enemyMoves.find(friendKings[k]) != enemyMoves.end())
                      {
                        moves.erase(i++);
                        needToInc = false;
                        j = enemyPos.end();
                        break;
                      }
                  }
                if (needToInc) j++;
              }
            if (needToInc) ++i;
            delete gCopy._board;
          }
      }

    //Finally, remaining moves can be returned
    return moves;
  }

  std::uint8_t Game::stones(SideType side) const
  {
    if (side == SideType::WHITE)
      {
        return _whiteStones;
      }
    else if (side == SideType::BLACK)
      {
        return _blackStones;
      }
    return 0;
  }
  
  bool Game::canCastle(SideType side, bool kingSide) const
  {
    if (side == SideType::NONE ||
        (side == SideType::WHITE && _whiteArmy != ArmyType::CLASSIC) ||
        (side == SideType::BLACK && _blackArmy != ArmyType::CLASSIC))
      {
        return false;
      }
    int smallx = kingSide ? 6 : 2;
    int y = (side == SideType::WHITE) ? 1 : 8;
    bool checkBool = (side == SideType::WHITE && kingSide && _whiteKingCastle) ||
      (side == SideType::WHITE && !kingSide && _whiteQueenCastle) ||
      (side == SideType::BLACK && kingSide && _blackKingCastle) ||
      (side == SideType::BLACK && !kingSide && _blackQueenCastle);

    return (*_board)(Position(smallx,y)).type() == PieceType::NONE &&
      (*_board)(Position(smallx+1,y)).type() == PieceType::NONE && checkBool;
  }
  
  ArmyType Game::army(SideType side) const
  {
     if (side == SideType::WHITE)
      {
        return _whiteArmy;
      }
    else if (side == SideType::BLACK)
      {
        return _blackArmy;
      }
     return ArmyType::NONE;
  }
       
} //Namespace
