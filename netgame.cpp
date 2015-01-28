/*
  Copyright (c) 2014 Auston Sterling
  See license.txt for copying permission.
  
  -----NetGame Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  A class extending the Chess 2 Game to automatically synchronize between
  two players on separate clients.
*/

#include "netgame.hpp"

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>


#include <iostream>

namespace c2
{

  NetGame::NetGame(Board* b, std::string ip, std::string port) :
    _sockfd(-1),
    _killThread(false)
  {
    setBoard(b);
    if (ip.length() > 0)
      {
        connectStart(ip, port);
      }
  }

  bool NetGame::connectStart(std::string ip, std::string port)
  {
    //Set up addrinfo struct
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* results;
    int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &results);
    if (status != 0)
      {
        return false;
      }

    //Try each of the provided addresses
    addrinfo* p;
    for (p = results; p != nullptr; p = p->ai_next)
      {
        //Open a TCP socket
        _sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_sockfd < 0)
          {
            continue;
          }

        //Bind the address to the socket
        if (connect(_sockfd, p->ai_addr, p->ai_addrlen) < 0)
          {
            close(_sockfd);
            continue;
          }

        //If we made it here, we've got a good one
        break;
      }
    freeaddrinfo(results);
    if (p == nullptr)
      {
        perror("Failed to set up a socket: ");
        close(_sockfd);
        return false;
      }

    //Create a thread to handle the communication
    pthread_t tid;
    _killThread = false;
    if (pthread_create(&tid, NULL, &connect_thread, this) != 0)
      {
        close(_sockfd);
        return false;
      }
    return true;
  }

  bool NetGame::listenStart(std::string port)
  {
    //Set up addrinfo struct
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* results;
    int status = getaddrinfo(nullptr, port.c_str(), &hints, &results);
    if (status != 0)
      {
        return false;
      }
    
    //Try each of the provided addresses
    addrinfo* p;
    for (p = results; p != nullptr; p = p->ai_next)
      {
        //Open a TCP socket
        _sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_sockfd < 0)
          {
            continue;
          }

        int yes = 1;
        int opt = setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
                             &yes, sizeof(int));
        if (opt < 0)
          {
            continue;
          }

        //Bind the socket
        if (bind(_sockfd, p->ai_addr, p->ai_addrlen) != 0)
          {
            close(_sockfd);
            continue;
          }

        //If we made it here, we've got a good one
        break;
      }
    freeaddrinfo(results);
    if (p == nullptr)
      {
        return false;
      }

    //Start listening for a connection
    if (listen(_sockfd, 5) < 0) return false;
    bool connected = false;
    while (!connected)
      {
        sockaddr_storage peer;
        socklen_t size = sizeof(peer);
        int csock = accept(_sockfd, reinterpret_cast<sockaddr*>(&peer), &size);
        if(csock < 0)	//bad client, skip it
          continue;

        //We're good!
        close(_sockfd);
        _sockfd = csock;
        connected = true;
        pthread_t tid;
        _killThread = false;
        
        if (pthread_create(&tid, NULL, &connect_thread, this) != 0) return false;
      }
    return true;
  }

  void* connect_thread(void* self)
  {
    NetGame* ng = static_cast<NetGame*>(self);

    //We believe a connection has been established.
    //Start by sending a version message, then begin regular behavior
    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x07);
    om.push_back(NET_VERSION >> 8);
    om.push_back(0xFF & NET_VERSION);
    ng->_outMessage.push(om);

    //Set up fp set for select
    fd_set readfs;
    FD_ZERO(&readfs);
    FD_SET(ng->_sockfd, &readfs);
    timeval fstime;
    fstime.tv_sec = 0;
    fstime.tv_usec = 100000;

    //Main loop
    while (!ng->_killThread)
      {
        //Check for incoming messages
        fd_set updatedfs = readfs;
        select(ng->_sockfd+1, &updatedfs, nullptr, nullptr, &fstime);

        bool gotMessage = false;
        Message im(100, '\0');
        std::size_t rec = 0;
        if (FD_ISSET(ng->_sockfd, &updatedfs))
          {
            do
              {
                ssize_t err = read(ng->_sockfd, (&im[rec]), HEADER_SIZE-rec);
                if (err < 0)
                  {
                    perror("Error reading from socket: ");
                    return nullptr;
                  }
                rec += err;
              }
            while (rec < HEADER_SIZE && rec > 0);
          }

        //If we did get a header, get the rest and process it
        if (rec >= HEADER_SIZE)
          {
            gotMessage = true;
            
            //If the magic number is wrong, bail for now
            //NOTE: Make this more intelligent later!
            if (im[0] != MAGIC_NUM)
              {
                break;
              }

            //Handle the message depending on type
            //NOTE: Find a better way to resolve non-success returns
            switch (im[1])
              {
              case 0: //setArmy
                {
                  while (rec < 4)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 4-rec);
                    }
                  GameReturnType r = ng->_game.setArmy(static_cast<SideType>(im[2]),
                                                       static_cast<ArmyType>(im[3]));
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 1: //start
                {
                  GameReturnType r = ng->_game.start();
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 2: //move
                {
                  while (rec < 8)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 8-rec);
                    }
                  Move m(Position(im[2],im[3]),Position(im[4],im[5]),
                         static_cast<PieceType>(im[6]),
                         static_cast<SideType>(im[7]));
                  GameReturnType r = ng->_game.move(m);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 3: //startDuel
                {
                  while (rec < 3)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 3-rec);
                    }
                  GameReturnType r = ng->_game.startDuel(im[2]==0?false:true);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 4: //bid
                {
                  while (rec < 4)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 4-rec);
                    }
                  GameReturnType r = ng->_game.bid(static_cast<SideType>(im[2]),
                                                   im[3]);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 5: //promote
                {
                  while (rec < 3)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 3-rec);
                    }
                  GameReturnType r = ng->_game.promote(static_cast<PieceType>(im[2]));
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  break;
                }
                
              case 6: //state
                {
                  while (rec < 3)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 3-rec);
                    }
                  if (im[2] != num(ng->_game.state())) ng->_killThread = true;
                  break;
                }

              case 7: //version
                {
                  while (rec < 4)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 4-rec);
                    }
                  std::uint16_t otherVer = (im[2] << 8) + im[3];
                  if (NET_VERSION != otherVer) ng->_killThread = true;
                  break;
                }
                
              default: break;
              }
          }

        //If we received a message, don't send anything until we're sure the
        //incoming queue is clear
        if (gotMessage) continue;
        
        //Send all outgoing messages
        while (!ng->_outMessage.empty())
          {
            om = ng->_outMessage.front();
            ng->_outMessage.pop();
            std::size_t sent = 0;
            while (sent < om.size())
              {
                sent += write(ng->_sockfd, &om[sent], om.size() - sent);
              }
          }
      }
    return nullptr;
  }

  GameReturnType NetGame::setBoard(Board* b)
  {
    //This part isn't synchronized, but should be done before connecting
    return _game.setBoard(b);
  }

  GameReturnType NetGame::setArmy(SideType side, ArmyType army)
  {
    GameReturnType r = _game.setArmy(side, army);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x00);
    om.push_back(num(side));
    om.push_back(num(army));
    _outMessage.push(om);

    return r;
  }

  GameReturnType NetGame::start()
  {
    GameReturnType r = _game.start();
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x01);
    _outMessage.push(om);

    return r;
  }

  GameReturnType NetGame::move(const Move& m)
  {
    GameReturnType r = _game.move(m);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x02);
    om.push_back(m.start.x());
    om.push_back(m.start.y());
    om.push_back(m.end.x());
    om.push_back(m.end.y());
    om.push_back(num(m.type));
    om.push_back(num(m.side));
    _outMessage.push(om);

    return r;
  }

  GameReturnType NetGame::startDuel(bool d)
  {
    GameReturnType r = _game.startDuel(d);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x03);
    om.push_back(d?0x01:0x00);
    _outMessage.push(om);

    return r;
  }

  GameReturnType NetGame::bid(SideType side, std::uint8_t stones)
  {
    GameReturnType r = _game.bid(side, stones);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x04);
    om.push_back(num(side));
    om.push_back(stones);
    _outMessage.push(om);

    return r;
  }

  GameReturnType NetGame::promote(PieceType newType)
  {
    GameReturnType r = _game.promote(newType);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(0x03);
    om.push_back(num(newType));
    _outMessage.push(om);

    return r;
  }

} //Namespace
