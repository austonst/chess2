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
#include <map>
#include "netgame.hpp"
#include "bitboard.hpp"
#include "sidebar.hpp"
#include <sstream>

using namespace c2;

const int BOARD_WIDTH = 405;
const int BOARD_HEIGHT = 405;
const int BORDER_WIDTH = 2;
const int TILE_SIZE = 50;
const int SIDEBAR_WIDTH = 200;

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
  std::string arg_localSide, arg_ip;
  if (argc == 3)
    {
      arg_localSide = argv[1];
      arg_ip = argv[2];
    }
  else
    {
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

  //Init SDL_image
  if((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
    {
    std::cerr << "IMG_Init: Failed to init required png support!" << std::endl;
    std::cerr << "IMG_Init: " << IMG_GetError() << std::endl;
  }

  //Set up the screen
  SDL_Window* screen = SDL_CreateWindow("SDL Chess 2", SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        BOARD_WIDTH + SIDEBAR_WIDTH,
                                        BOARD_HEIGHT, 0);
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

  //Create a sidebar
  SDL_Color whiteColor = {255, 255, 255, 255};
  Sidebar sidebar(SIDEBAR_WIDTH, BOARD_HEIGHT, whiteColor);

  //Put a header image at the top
  //TODO THIS

  //Make a quit button and an initially invisible follow-up confirmation button
  //Both should be the heaviest objects
  Sidebar::iterator button = sidebar.createObject(2000, "quit");
  button->resizeAndRespace(1);
  SDL_Texture* quitTex = IMG_LoadTexture(rend, "images/button_quit.png");
  button->setTexture(0, quitTex);
  button->respace();

  button = sidebar.createObject(2000, "quitConfirm");
  button->resizeAndRespace(2);
  SDL_Texture* reallyTex = IMG_LoadTexture(rend, "images/button_really.png");
  SDL_Texture* cancelTex = IMG_LoadTexture(rend, "images/button_cancel.png");
  button->setTexture(0, reallyTex);
  button->setTexture(1, cancelTex);
  button->respace(SpacingType::SQUISH_CENTER, 10);
  button->setVisibility(false);

  //Create army selection objects, set visibility depending on control
  //10 weight puts them pretty high up
  std::vector<SDL_Texture*> armyTex(NUM_ARMIES, nullptr);
  std::vector<SDL_Texture*> armySelTex(NUM_ARMIES, nullptr);
  armyTex[num(ArmyType::CLASSIC)] = IMG_LoadTexture(rend, "images/button_c.png");
  armySelTex[num(ArmyType::CLASSIC)] = IMG_LoadTexture(rend, "images/button_sel_c.png");
  armyTex[num(ArmyType::EMPOWERED)] = IMG_LoadTexture(rend, "images/button_e.png");
  armySelTex[num(ArmyType::EMPOWERED)] = IMG_LoadTexture(rend, "images/button_sel_e.png");
  armyTex[num(ArmyType::NEMESIS)] = IMG_LoadTexture(rend, "images/button_n.png");
  armySelTex[num(ArmyType::NEMESIS)] = IMG_LoadTexture(rend, "images/button_sel_n.png");
  armyTex[num(ArmyType::REAPER)] = IMG_LoadTexture(rend, "images/button_r.png");
  armySelTex[num(ArmyType::REAPER)] = IMG_LoadTexture(rend, "images/button_sel_r.png");
  armyTex[num(ArmyType::ANIMALS)] = IMG_LoadTexture(rend, "images/button_a.png");
  armySelTex[num(ArmyType::ANIMALS)] = IMG_LoadTexture(rend, "images/button_sel_a.png");
  armyTex[num(ArmyType::TWOKINGS)] = IMG_LoadTexture(rend, "images/button_2.png");
  armySelTex[num(ArmyType::TWOKINGS)] = IMG_LoadTexture(rend, "images/button_sel_2.png");
  
  SidebarObject buildSBO(armyTex, SIDEBAR_WIDTH, SpacingType::UNIFORM);
  buildSBO.prepareForInsert(10, "whiteArmy");
  buildSBO.setVisibility(whiteControl);
  sidebar.insertObject(buildSBO);

  buildSBO.prepareForInsert(10, "blackArmy");
  buildSBO.setVisibility(blackControl);
  sidebar.insertObject(buildSBO);

  //Create start object to lock in army selection and try to start the game
  //Weight is 15 to appear below army selection
  //Button always starts visible
  button = sidebar.createObject(15, "start");
  button->resizeAndRespace(1);
  SDL_Texture* startTex = IMG_LoadTexture(rend, "images/button_start.png");
  button->setTexture(0, startTex);
  button->respace();

  //Create game status tracker, showing player's armies, whose turn it is,
  //and some state information
  button = sidebar.createObject(15, "state");
  button->resizeAndRespace(3);
  button->setVertAlign(VertAlignType::CENTER);
  std::vector<SDL_Texture*> statusTex(NUM_GAMESTATES, nullptr);
  statusTex[num(GameStateType::WHITE_MOVE)] =
    statusTex[num(GameStateType::BLACK_MOVE)] =
    IMG_LoadTexture(rend, "images/state_move.png");
  statusTex[num(GameStateType::WHITE_KINGMOVE)] =
    statusTex[num(GameStateType::BLACK_KINGMOVE)] =
    IMG_LoadTexture(rend, "images/state_kingmove.png");
  statusTex[num(GameStateType::WHITE_DUEL)] =
    statusTex[num(GameStateType::BLACK_DUEL)] =
    IMG_LoadTexture(rend, "images/state_duel.png");
  statusTex[num(GameStateType::BOTH_BID)] =
    statusTex[num(GameStateType::WHITE_BID)] =
    statusTex[num(GameStateType::BLACK_BID)] =
    IMG_LoadTexture(rend, "images/state_bid.png");
  statusTex[num(GameStateType::WHITE_PROMOTE)] =
    statusTex[num(GameStateType::BLACK_PROMOTE)] =
    IMG_LoadTexture(rend, "images/state_promote.png");
  statusTex[num(GameStateType::WHITE_WIN_CHECKMATE)] =
    statusTex[num(GameStateType::WHITE_WIN_MIDLINE)] =
    IMG_LoadTexture(rend, "images/state_whitewin.png");
  statusTex[num(GameStateType::BLACK_WIN_CHECKMATE)] =
    statusTex[num(GameStateType::BLACK_WIN_MIDLINE)] =
    IMG_LoadTexture(rend, "images/state_blackwin.png");
  statusTex[num(GameStateType::DRAW_THREEFOLD)] =
    statusTex[num(GameStateType::DRAW_FIFTYMOVE)] =
    IMG_LoadTexture(rend, "images/state_draw.png");
  //No textures are initially set since none are yet known
  button->setVisibility(false);

  //Create king move skip object
  //Weight of 30 to put it below most things
  button = sidebar.createObject(30, "skipking");
  button->resizeAndRespace(1);
  SDL_Texture* skipKingTex = IMG_LoadTexture(rend, "images/button_skipking.png");
  button->setTexture(0, skipKingTex);
  button->respace();
  button->setVisibility(false);

  //Create stone tracking objects
  //Weight of 16 and 17 put them right below state
  SDL_Texture* stoneNoneTex = IMG_LoadTexture(rend, "images/stone_none.png");
  SDL_Texture* stoneWhiteTex = IMG_LoadTexture(rend, "images/stone_white.png");
  SDL_Texture* stoneBlackTex = IMG_LoadTexture(rend, "images/stone_black.png");
  SidebarObject stonesObj(std::vector<SDL_Texture*>(6, stoneNoneTex),
                          SIDEBAR_WIDTH, SpacingType::SQUISH_CENTER, 5);
  stonesObj.setVisibility(false);
  sidebar.insertObject(stonesObj, 16, "blackstones");
  sidebar.insertObject(stonesObj, 17, "whitestones");

  //Put other stuff here, like side selection
  //TODO THIS

  //Look at args to determine networky stuff and set up game
  std::string ip;
  BitBoard board;
  NetGame ng(&board);
  if (std::string(arg_ip) == "host")
    {
      std::cout << "Listening for connections..." << std::endl;
      if (!ng.listenStart())
        {
          std::cerr << "Failed to start up networking.\n";
          return 1;
        }
    }
  else if (std::string(arg_ip) == "no")
    {
      //Do nothing!
    }
  else
    {
      std::cout << "Connecting to " << arg_ip << std::endl;
      ip = arg_ip;
      if (!ng.connectStart(ip))
        {
          std::cerr << "Failed to connect to " << arg_ip << "\n";
          return 1;
        }
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
  SidebarClickResponse mouseDownClick;
  Piece selectedPiece;
  std::set<Position> moves;
  int timer = SDL_GetTicks();
  bool quit = false;
  while (!quit)
    {
      const Uint8* keystates = SDL_GetKeyboardState(nullptr);
      SDL_Event e;
      bool lClick = false;
      bool rClick = false;
      bool sidebarClick = false;
      while (SDL_PollEvent(&e))
        {
          if (e.type == SDL_QUIT) quit = true;

          //Nothing happens on mouse down, but the click as a whole only
          //happens if both down and up are on the same tile
          if (e.type == SDL_MOUSEBUTTONDOWN)
            {
              if (e.button.button == SDL_BUTTON_LEFT)
                {
                  //Check for board vs sidebar left mouse down
                  if (e.button.x < BOARD_WIDTH)
                    {
                      mouseLX = (e.button.x-BORDER_WIDTH) / TILE_SIZE;
                      mouseLY = (e.button.y-BORDER_WIDTH) / TILE_SIZE;
                    }
                  else //Sidebar click
                    {
                      mouseDownClick = sidebar.click(e.button.x - BOARD_WIDTH,
                                                     e.button.y);
                    }
                }
              else if (e.button.button == SDL_BUTTON_RIGHT)
                {
                  mouseRX = (e.button.x-BORDER_WIDTH) / TILE_SIZE;
                  mouseRY = (e.button.y-BORDER_WIDTH) / TILE_SIZE;
                }
            }

          //Board mouse up
          else if (e.type == SDL_MOUSEBUTTONUP)
            {
              if (e.button.button == SDL_BUTTON_LEFT)
                {
                  //Check for board vs sidebar left mouse up
                  if (e.button.x < BOARD_WIDTH)
                    {
                      if ((e.button.x-BORDER_WIDTH) / TILE_SIZE == mouseLX &&
                          (e.button.y-BORDER_WIDTH) / TILE_SIZE == mouseLY)
                        {
                          lClick = true;
                        }
                    }
                  else //Sidebar click
                    {
                      //First, clear moves to be consistent with "only show
                      //moves when a piece is clicked"
                      moves.clear();

                      //Actually handle the click
                      SidebarClickResponse mouseUpClick;
                      mouseUpClick = sidebar.click(e.button.x - BOARD_WIDTH,
                                                   e.button.y);
                      if (mouseDownClick.sbo == mouseUpClick.sbo &&
                          sidebar.isValid(mouseDownClick.sbo) &&
                          mouseDownClick.texture == mouseUpClick.texture)
                        {
                          sidebarClick = true;
                          
                          //Also use the coordinates of the mouse down
                          mouseDownClick = mouseUpClick;
                        }
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
      //CURRENTLY NONE TO CHECK--ALL MOUSE!

      //Left click changes the selected piece or interacts with sidebar
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

      //Sidebar clicks behave depending on the clicked object
      if (sidebarClick)
        {
          //Handle each sidebar object
          if (mouseDownClick.sbo->id() == "quit")
            {
              //Only one button, but make sure we hit it
              if (mouseDownClick.texture == 0)
                {
                  //Disable this object, enable the confirmation object
                  mouseDownClick.sbo->setVisibility(false);
                  sidebar.object("quitConfirm")->setVisibility(true);
                }
            }
          else if (mouseDownClick.sbo->id() == "quitConfirm")
            {
              //Two buttons here
              if (mouseDownClick.texture == 0)
                {
                  //Really button, we quit now
                  quit = true;
                }
              else if (mouseDownClick.texture == 1)
                {
                  //Cancel button, we go back to quit object
                  mouseDownClick.sbo->setVisibility(false);
                  sidebar.object("quit")->setVisibility(true);
                }
            }
          else if (mouseDownClick.sbo->id() == "whiteArmy" ||
                   mouseDownClick.sbo->id() == "blackArmy")
            {
              //Set all appearnaces to unselected except for this one
              for (size_t i = 0; i < mouseDownClick.sbo->size(); i++)
                {
                  mouseDownClick.sbo->setTexture(i, armyTex[i]);
                }
              mouseDownClick.sbo->setTexture(mouseDownClick.texture,
                                             armySelTex[mouseDownClick.texture]);

              if (mouseDownClick.sbo->id() == "whiteArmy")
                {
                  ng.setArmy(SideType::WHITE, toArmy(mouseDownClick.texture));
                }
              else //blackArmy
                {
                  ng.setArmy(SideType::BLACK, toArmy(mouseDownClick.texture));
                }
            }
          else if (mouseDownClick.sbo->id() == "start")
            {
              //There's a chance start won't work (armies not set)
              if (mouseDownClick.texture == 0 &&
                  ng.start() == GameReturnType::SUCCESS)
                {
                  //If it does, we need to set up the game objects
                  sidebar.object("whiteArmy")->setVisibility(false);
                  sidebar.object("blackArmy")->setVisibility(false);
                  mouseDownClick.sbo->setVisibility(false);

                  sidebar.object("state")->setVisibility(true);
                  sidebar.object("blackstones")->setVisibility(true);
                  sidebar.object("whitestones")->setVisibility(true);
                }
            }
          else if (mouseDownClick.sbo->id() == "skipking")
            {
              if (mouseDownClick.texture == 0)
                {
                  //Find a king
                  SideType kingside = SideType::WHITE;
                  if (ng.state() == GameStateType::BLACK_KINGMOVE)
                    {
                      kingside = SideType::BLACK;
                    }

                  //Move him to the special position in order to skip the turn
                  Position king = board.getKing(kingside)[0];
                  ng.move(Move(king, KINGMOVE_SKIP_POS, PieceType::TKG_WARRKING,
                               kingside));
                }
            }
        }

      //Perform any per-frame sidebar updates
      //Update sidebar status depending on state
      Sidebar::iterator statusObject = sidebar.object("state");
      statusObject->setTexture(1, statusTex[num(ng.state())]);
      if (ng.state() == GameStateType::WHITE_MOVE)
        {
          int armyIndex = num(ng.army(SideType::WHITE));
          statusObject->setTexture(0, armySelTex[armyIndex]);
          armyIndex = num(ng.army(SideType::BLACK));
          statusObject->setTexture(2, armyTex[armyIndex]);
        }
      else if (ng.state() == GameStateType::BLACK_MOVE)
        {
          int armyIndex = num(ng.army(SideType::WHITE));
          statusObject->setTexture(0, armyTex[armyIndex]);
          armyIndex = num(ng.army(SideType::BLACK));
          statusObject->setTexture(2, armySelTex[armyIndex]);
        }
      statusObject->respace(SpacingType::UNIFORM);

      //If we are in a king move, the sidebar skip button needs to be visible
      if ((ng.state() == GameStateType::WHITE_KINGMOVE && whiteControl) ||
          (ng.state() == GameStateType::BLACK_KINGMOVE && blackControl))
        {
          sidebar.object("skipking")->setVisibility(true);
        }
      else
        {
          sidebar.object("skipking")->setVisibility(false);
        }

      //Update number of stones
      for (size_t i = 0; i < ng.stones(SideType::WHITE); i++)
        {
          sidebar.object(17, "whitestones")->setTexture(i, stoneWhiteTex);
        }
      for (size_t i = ng.stones(SideType::WHITE); i < 6; i++)
        {
          sidebar.object(17, "whitestones")->setTexture(i, stoneNoneTex);
        }
      for (size_t i = 0; i < ng.stones(SideType::BLACK); i++)
        {
          sidebar.object(16, "blackstones")->setTexture(i, stoneBlackTex);
        }
      for (size_t i = ng.stones(SideType::BLACK); i < 6; i++)
        {
          sidebar.object(16, "blackstones")->setTexture(i, stoneNoneTex);
        }

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
      if ((ng.state() == GameStateType::BOTH_BID ||
           ng.state() == GameStateType::WHITE_BID) && whiteControl)
        {
          std::vector<std::string> choices;
          for (std::uint8_t i = 0; i < ng.stones(SideType::WHITE)+1 && i < 3; i++)
            {
			  std::stringstream ss;
			  ss << int(i);
              choices.push_back(ss.str());
            }
		  std::stringstream ss;
		  ss << "Select number of stones for white to bid (has " <<
		    int(ng.stones(SideType::WHITE)) << ").";
          int stones = dialogBox(ss.str(), choices, screen);
          ng.bid(SideType::WHITE, stones-1);
          
        }
      if ((ng.state() == GameStateType::BOTH_BID ||
           ng.state() == GameStateType::BLACK_BID) && blackControl)
        {
          std::vector<std::string> choices;
          for (std::uint8_t i = 0; i < ng.stones(SideType::BLACK)+1 && i < 3; i++)
            {
			  std::stringstream ss;
			  ss << int(i);
              choices.push_back(ss.str());
            }
		  std::stringstream ss;
		  ss << "Select number of stones for black to bid (has " <<
		    int(ng.stones(SideType::BLACK)) << ").";
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
      SDL_Rect boardDst = {0, 0, BOARD_WIDTH, BOARD_HEIGHT};
      SDL_RenderCopy(rend, boardTex, NULL, &boardDst);

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

      //Draw sidebar
      sidebar.render(rend, BOARD_WIDTH, 0);
      
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
      if (!ng.connected())
        {
          std::cerr << "Lost connection to other player!" << std::endl;
          quit = true;
        }

      //Cap FPS at 30
      int newtimer = SDL_GetTicks();
      if (newtimer - timer < 1000.0/30.)
        {
          SDL_Delay(1000./30. - (newtimer-timer));
        }
      timer = newtimer;
    }

  //Clean up
  SDL_DestroyTexture(boardTex);
  SDL_DestroyTexture(pieceTex);
  SDL_DestroyTexture(moveTex);
  SDL_DestroyTexture(quitTex);
  SDL_DestroyRenderer(rend);
  SDL_DestroyWindow(screen);
  IMG_Quit();
  SDL_Quit();
  return 0;
}
