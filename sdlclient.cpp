/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.

  -----SDLClient Implementation-----
  Auston Sterling
  austonst@gmail.com

  A client for Sirlin's Chess 2 using SDL.
*/

#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include "netgame.hpp"
#include "bitboard.hpp"
#include <sstream>

using namespace c2;

const int SCREEN_WIDTH = 405;
const int SCREEN_HEIGHT = 405;
const int BORDER_WIDTH = 2;
const int TILE_SIZE = 50;

//Returns 0 on X or error, or i+1 if button[i] was pressed
int dialogBox(const std::string& text, const std::vector<std::string>& button,
              SDL_Window* parent, size_t enterDefault = 0,
              size_t escapeDefault = 0)
{
  //Set up message box data
  SDL_MessageBoxData msgDat;
  msgDat.flags = SDL_MESSAGEBOX_INFORMATION;
  msgDat.title = "SDL Chess 2 Prompt";
  msgDat.message = text.c_str();
  msgDat.numbuttons = button.size();
  msgDat.colorScheme = NULL;
  msgDat.window = parent;

  //Set up each button to match the passed strings
  std::vector<SDL_MessageBoxButtonData> buttonDat;
  for (size_t i = 0; i < button.size(); i++)
    {
      SDL_MessageBoxButtonData dat;
      dat.text = button[i].c_str();
      dat.buttonid = i+1;
      if (i+1 == enterDefault)
        {
          dat.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        }
      else if (i+1 == escapeDefault)
        {
          dat.flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
        }
      else
        {
          dat.flags = 0;
        }
      buttonDat.push_back(dat);
    }
  msgDat.buttons = &buttonDat[0];

  //Display, get result
  int result;
  SDL_ShowMessageBox(&msgDat, &result);
  return result;
}

int main(int argc, char* argv[])
{
  std::string arg_whiteArmy, arg_blackArmy, arg_localSide, arg_ip;
  if (argc == 5)
    {
      arg_whiteArmy = argv[1];
      arg_blackArmy = argv[2];
      arg_localSide = argv[3];
      arg_ip = argv[4];
    }
  else
    {
      std::cout << "Enter first character of name of white army (2 for two kings): ";
      std::cin >> arg_whiteArmy;
      std::cout << "Enter first character of name of black army (2 for two kings): ";
      std::cin >> arg_blackArmy;
      std::cout << "Enter which armies you control locally (white, black, or both): ";
      std::cin >> arg_localSide;
      std::cout << "Enter \"host\" if hosting, host's ip address if connecting, or\n"
                << "\"no\" for non-networked play: ";
      std::cin >> arg_ip;
    }
  bool whiteControl = std::string(arg_localSide) == "white" ||
    std::string(arg_localSide) == "both";
  bool blackControl = std::string(arg_localSide) == "black" ||
    std::string(arg_localSide) == "both";
  if (!whiteControl && !blackControl)
    {
      std::cerr << "You must put \"white\", \"black\", or \"both\" for localcontrol!\n";
      return 1;
    }

      //Initialize all SDL subsystems
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
	  std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
	  return 1;
	}

  //Set up the screen
  SDL_Window* screen = SDL_CreateWindow("SDL Chess 2", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                        SCREEN_HEIGHT, 0);
  if (!screen) return 1;
  
  SDL_Renderer* rend = SDL_CreateRenderer(screen, -1, 0);
  if (!rend) return 1;
  
  std::string errnew = SDL_GetError();
  if (errnew.length() > 0)
    {
      std::cerr << "Ignoring error: " << errnew << std::endl;
      SDL_ClearError();
    }
  else
    {
      std::cout << "STRANGE ERROR DOES NOT SHOW UP!" << std::endl;
    }
  SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);

  //Load the board and piece images
  SDL_Texture* boardTex = IMG_LoadTexture(rend, "images/board.png");
  SDL_Texture* pieceTex = IMG_LoadTexture(rend, "images/pieces.png");
  SDL_Texture* moveTex = IMG_LoadTexture(rend, "images/move.png");

  //Look at args to determine networky stuff and set up game
  std::string ip;
  BitBoard board;
  NetGame ng(&board);
  if (std::string(arg_ip) == "host")
    {
      std::cout << "Listening for connections..." << std::endl;
      ng.listenStart();
    }
  else if (std::string(arg_ip) == "no")
    {
      //Do nothing!
    }
  else
    {
      std::cout << "Connecting to " << arg_whiteArmy << std::endl;
      ip = arg_whiteArmy;
      ng.connectStart(ip);
    }

  //Set up armies
  if (std::string(arg_whiteArmy) == "c")
    {
      ng.setArmy(SideType::WHITE, ArmyType::CLASSIC);
    }
  else if (std::string(arg_whiteArmy) == "e")
    {
      ng.setArmy(SideType::WHITE, ArmyType::EMPOWERED);
    }
  else if (std::string(arg_whiteArmy) == "n")
    {
      ng.setArmy(SideType::WHITE, ArmyType::NEMESIS);
    }
  else if (std::string(arg_whiteArmy) == "r")
    {
      ng.setArmy(SideType::WHITE, ArmyType::REAPER);
    }
  else if (std::string(arg_whiteArmy) == "2")
    {
      ng.setArmy(SideType::WHITE, ArmyType::TWOKINGS);
    }
  else if (std::string(arg_whiteArmy) == "a")
    {
      ng.setArmy(SideType::WHITE, ArmyType::ANIMALS);
    }
  else
    {
      std::cerr << "The army for white must be given as one of 'cenr2a', you put: " << arg_whiteArmy << std::endl;
      return 1;
    }
  
  if (std::string(arg_blackArmy) == "c")
    {
      ng.setArmy(SideType::BLACK, ArmyType::CLASSIC);
    }
  else if (std::string(arg_blackArmy) == "e")
    {
      ng.setArmy(SideType::BLACK, ArmyType::EMPOWERED);
    }
  else if (std::string(arg_blackArmy) == "n")
    {
      ng.setArmy(SideType::BLACK, ArmyType::NEMESIS);
    }
  else if (std::string(arg_blackArmy) == "r")
    {
      ng.setArmy(SideType::BLACK, ArmyType::REAPER);
    }
  else if (std::string(arg_blackArmy) == "2")
    {
      ng.setArmy(SideType::BLACK, ArmyType::TWOKINGS);
    }
  else if (std::string(arg_blackArmy) == "a")
    {
      ng.setArmy(SideType::BLACK, ArmyType::ANIMALS);
    }
  else
    {
      std::cerr << "The army for black must be given as one character in 'cenr2a', you put: " << arg_blackArmy << std::endl;
      return 1;
    }

  //We should be able to start now! We only need one side to do it, so make white
  if (whiteControl)
    {
      ng.start();
    }

  //Check for any errors during setup
  std::string errstr = SDL_GetError();
  if (errstr.length() > 0)
    {
      std::cerr << "Error during setup: " << errstr << std::endl;
      return 1;
    }

  //Start the main loop
  Sint32 mouseLX;
  Sint32 mouseLY;
  Sint32 mouseRX;
  Sint32 mouseRY;
  Piece selectedPiece;
  std::set<Position> moves;
  bool quit = false;
  while (!quit)
    {
      const Uint8* keystates = SDL_GetKeyboardState(nullptr);
      SDL_Event e;
      bool lClick = false;
      bool rClick = false;
      while (SDL_PollEvent(&e))
        {
          if (e.type == SDL_QUIT) quit = true;

          //Nothing happens on mouse down, but the click as a whole only
          //happens if both down and up are on the same tile
          if (e.type == SDL_MOUSEBUTTONDOWN)
            {
              if (e.button.button == SDL_BUTTON_LEFT)
                {
                  mouseLX = (e.button.x-BORDER_WIDTH) / TILE_SIZE;
                  mouseLY = (e.button.y-BORDER_WIDTH) / TILE_SIZE;
                }
              else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                  mouseRX = (e.button.x-BORDER_WIDTH) / TILE_SIZE;
                  mouseRY = (e.button.y-BORDER_WIDTH) / TILE_SIZE;
                }
            }

          if (e.type == SDL_MOUSEBUTTONUP)
            {
              if (e.button.button == SDL_BUTTON_LEFT)
                {
                  if ((e.button.x-BORDER_WIDTH) / TILE_SIZE == mouseLX &&
                      (e.button.y-BORDER_WIDTH) / TILE_SIZE == mouseLY)
                    {
                      lClick = true;
                    }
                }
              else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                  if ((e.button.x-BORDER_WIDTH) / TILE_SIZE == mouseRX &&
                      (e.button.y-BORDER_WIDTH) / TILE_SIZE == mouseRY)
                    {
                      rClick = true;
                    }
                }
            }
        }

      //Check keys and clicks
      //Escape quits
      if (keystates[SDL_SCANCODE_ESCAPE])
        {
          quit = true;
        }

      //Left click changes the selected piece
      if (lClick)
        {
          Position clickPos(mouseLX+1, 8-mouseLY);
          selectedPiece = board(clickPos);
          if (clickPos.isValid() && selectedPiece.type() != PieceType::NONE)
            {
              moves = ng.possibleMoves(selectedPiece.pos());
            }
          else
            {
              moves.clear();
            }
        }

      //Right click attempts to make a move
      if (rClick)
        {
          Position newPos(mouseRX+1, 8-mouseRY);
          if (moves.find(newPos) != moves.end() &&
              ((selectedPiece.side() == SideType::WHITE && whiteControl) ||
               (selectedPiece.side() == SideType::BLACK && blackControl)))
            {
              Move m(selectedPiece.pos(), newPos, selectedPiece.type(),
                     selectedPiece.side());
              GameReturnType result = ng.move(m);
              if (result == GameReturnType::SUCCESS)
                {
                  moves.clear();
                }
            }
        }

      //Escape quits

      //Depending on game state, we may need to do other things
      //Accept/decline duel
      if ((ng.state() == GameStateType::WHITE_DUEL && whiteControl) ||
          (ng.state() == GameStateType::BLACK_DUEL && blackControl))
        {
          int accept = dialogBox("Would you like to duel?",
                                 {"Accept", "Decline"}, screen, 1, 2);
          ng.startDuel(accept == 1);
        }

      //Make a bid in a duel
      if (ng.state() == GameStateType::BOTH_BID ||
          (ng.state() == GameStateType::WHITE_BID && whiteControl))
        {
          std::vector<std::string> choices;
          for (std::uint8_t i = 0; i < ng.stones(SideType::WHITE)+1 && i < 3; i++)
            {
			  std::stringstream ss;
			  ss << char(i);
              choices.push_back(ss.str());
            }
		  std::stringstream ss;
		  ss << "Select number of stones for white to bid (has " <<
		    char(ng.stones(SideType::WHITE)) << ").";
          int stones = dialogBox(ss.str(), choices, screen);
          ng.bid(SideType::WHITE, stones-1);
          
        }
      if (ng.state() == GameStateType::BOTH_BID ||
          (ng.state() == GameStateType::BLACK_BID && blackControl))
        {
          std::vector<std::string> choices;
          for (std::uint8_t i = 0; i < ng.stones(SideType::BLACK)+1 && i < 3; i++)
            {
			  std::stringstream ss;
			  ss << char(i);
              choices.push_back(ss.str());
            }
		  std::stringstream ss;
		  ss << "Select number of stones for black to bid (has " <<
		    char(ng.stones(SideType::BLACK)) << ").";
          int stones = dialogBox(ss.str(), choices, screen);
          ng.bid(SideType::BLACK, stones-1);
        }

      //Promote a pawn
      if ((ng.state() == GameStateType::WHITE_PROMOTE && whiteControl) ||
          (ng.state() == GameStateType::BLACK_PROMOTE && blackControl))
        {
          ArmyType army = (ng.state() == GameStateType::WHITE_PROMOTE) ?
            ng.army(SideType::WHITE) : ng.army(SideType::BLACK);
          std::vector<std::string> choices;
          for (auto i : ARMY_PROMOTE[num(army)])
            {
              choices.push_back(PIECE_NAME[num(i)]);
            }
          int type = dialogBox("Select piece to promote to.", choices, screen);

          //We have the id corresponding to the name of the piece
          for (std::uint8_t i = 0; i < PIECE_TYPES; i++)
            {
              if (PIECE_NAME[i] == choices[type-1])
                {
                  ng.promote(static_cast<PieceType>(i));
                }
            }
        }

      //Win states come AFTER drawing so we can see the result

      //Draw board
      SDL_RenderCopy(rend, boardTex, NULL, NULL);

      //Draw pieces
      std::list<Position> pieces = board.getPieces(SideType::WHITE);
      pieces.splice(pieces.end(), board.getPieces(SideType::BLACK));
      for (Position i : pieces)
        {
          Piece p = board(i);

          //Convert position to screen coordinates
          SDL_Rect destRec;
          destRec.x = (i.x()-1) * TILE_SIZE + BORDER_WIDTH;
          destRec.y = (8-i.y()) * TILE_SIZE + BORDER_WIDTH;

          //Get location of piece in image
          SDL_Rect srcRec;
          switch (p.type())
            {
              //Simple king sprite
            case PieceType::CLA_KING:
            case PieceType::ANY_KING:
              srcRec.x = 0;
              srcRec.y = 0;
              break;

              //Fancy king sprite
            case PieceType::TKG_WARRKING:
              srcRec.x = 0;
              srcRec.y = 2;
              break;

              //Simple queen sprite
            case PieceType::CLA_QUEEN:
            case PieceType::EMP_QUEEN:
              srcRec.x = 1;
              srcRec.y = 0;
              break;

              //Fancy queen sprite
            case PieceType::NEM_QUEEN:
            case PieceType::RPR_REAPER:
            case PieceType::ANI_JUNGQUEEN:
              srcRec.x = 1;
              srcRec.y = 2;
              break;

              //Simple rook sprite
            case PieceType::CLA_ROOK:
              srcRec.x = 2;
              srcRec.y = 0;
              break;

              //Fancy rook sprite
            case PieceType::EMP_ROOK:
            case PieceType::RPR_GHOST:
            case PieceType::ANI_ELEPHANT:
              srcRec.x = 2;
              srcRec.y = 2;
              break;

              //Simple bishop sprite
            case PieceType::CLA_BISHOP:
              srcRec.x = 3;
              srcRec.y = 0;
              break;

              //Fancy bishop sprite
            case PieceType::EMP_BISHOP:
            case PieceType::ANI_TIGER:
              srcRec.x = 3;
              srcRec.y = 2;
              break;

              //Simple knight sprite
            case PieceType::CLA_KNIGHT:
              srcRec.x = 4;
              srcRec.y = 0;
              break;

              //Fancy knight sprite
            case PieceType::EMP_KNIGHT:
            case PieceType::ANI_WILDHORSE:
              srcRec.x = 4;
              srcRec.y = 2;
              break;

              //Simple pawn sprite
            case PieceType::CLA_PAWN:
              srcRec.x = 5;
              srcRec.y = 0;
              break;

              //Fancy pawn sprite
            case PieceType::NEM_PAWN:
              srcRec.x = 5;
              srcRec.y = 2;
              break;

            default: //NOTE: If you see glitchy simple white kings, check this!
              srcRec.x = 0;
              srcRec.y = 0;
              break;
            }
          if (p.side() == SideType::BLACK)
            {
              srcRec.y++;
            }
          srcRec.x *= TILE_SIZE;
          srcRec.y *= TILE_SIZE;
          destRec.w = destRec.h = srcRec.w = srcRec.h = TILE_SIZE;
          SDL_RenderCopy(rend, pieceTex, &srcRec, &destRec);
        }

      //Draw possible moves
      for (Position i : moves)
        {
          SDL_Rect srcRec;
          srcRec.x = 0;
          srcRec.y = 0;
          SDL_Rect destRec;
          destRec.x = (i.x()-1) * TILE_SIZE + BORDER_WIDTH;
          destRec.y = (8-i.y()) * TILE_SIZE + BORDER_WIDTH;
          srcRec.w = srcRec.h = destRec.w = destRec.h = TILE_SIZE;
          SDL_RenderCopy(rend, moveTex, &srcRec, &destRec);
        }
      
      //Finalize drawing
      SDL_RenderPresent(rend);

      //Check for win state now that final configuration is displayed
      if (ng.state() == GameStateType::WHITE_WIN_CHECKMATE ||
          ng.state() == GameStateType::WHITE_WIN_MIDLINE)
        {
          dialogBox("White wins!", {"Woohoo!", "Boo"}, screen, 1, 2);
          quit = true;
        }
      else if (ng.state() == GameStateType::BLACK_WIN_CHECKMATE ||
                ng.state() == GameStateType::BLACK_WIN_MIDLINE)
        {
          dialogBox("Black wins!", {"Woohoo!", "Boo"}, screen, 1, 2);
          quit = true;
        }
      else if (ng.state() == GameStateType::DRAW_THREEFOLD ||
               ng.state() == GameStateType::DRAW_FIFTYMOVE)
        {
          dialogBox("Draw!", {"Woohoo!", "Boo"}, screen, 1, 2);
          quit = true;
        }

      //See if any errors arose this frame
      errstr = SDL_GetError();
      if (errstr.length() > 0)
        {
          std::cerr << errstr << std::endl;
          quit = true;
        }
    }

  //Clean up
  SDL_DestroyTexture(boardTex);
  SDL_DestroyTexture(pieceTex);
  SDL_DestroyTexture(moveTex);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(screen);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
