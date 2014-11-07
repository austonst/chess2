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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>


#include <iostream>

namespace c2
{

  NetGame::NetGame(Board* b, std::string ip, std::uint16_t port) :
    _funcCount(0),
    _sockfd(-1),
    _killThread(false)
  {
    setBoard(b);
    if (ip.length() > 0)
      {
        connectStart(ip, port);
      }
  }

  bool NetGame::connectStart(std::string ip, std::uint16_t port)
  {
    //Open a TCP socket
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd < 0) return false;

    //Set up the address of the peer
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    inet_pton(AF_INET, &ip[0], &(address.sin_addr.s_addr));

    //Bind the address to the socket
    if (connect(_sockfd, (sockaddr*) &address, sizeof(address)) < 0) return false;

    //Create a thread to handle the communication
    pthread_t tid;
    _killThread = false;
    _address.resize(4,0);
    inet_pton(AF_INET, &ip[0], &_address[0]);
    if (pthread_create(&tid, NULL, &connect_thread, this) != 0) return false;
    return true;
  }

  bool NetGame::listenStart(std::uint16_t port)
  {
    //Open a TCP socket
    _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sockfd < 0) return false;
    
    //Set up our address
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    std::uint8_t* ipaddr = reinterpret_cast<std::uint8_t*>(&address.sin_addr);
    ipaddr[0] = 127;
    ipaddr[1] = 0;
    ipaddr[2] = 0;
    ipaddr[3] = 1;
    if (bind(_sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
      {
        return false;
      }

    //Start listening for a connection
    if (listen(_sockfd, 1) < 0) return false;
    bool connected = false;
    while (!connected)
      {
        sockaddr_in peer;
        socklen_t size = sizeof(peer);
        int csock = accept(_sockfd, reinterpret_cast<sockaddr*>(&peer), &size);
        if(csock < 0)	//bad client, skip it
          continue;

        //We're good!
        connected = true;
        pthread_t tid;
        _killThread = false;
        _address.resize(4,0);
        ipaddr = reinterpret_cast<std::uint8_t*>(&peer.sin_addr);
        _address[0] = ipaddr[0];
        _address[1] = ipaddr[1];
        _address[2] = ipaddr[2];
        _address[3] = ipaddr[3];
        if (pthread_create(&tid, NULL, &connect_thread, this) != 0) return false;
      }
    return true;
  }

  void* connect_thread(void* self)
  {
    NetGame* ng = static_cast<NetGame*>(self);
    ng->_funcCount = 0;

    //We believe a connection has been established.
    //Start by sending a version and state message, then begin regular behavior
    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(ng->_funcCount >> 8);
    om.push_back(0xFF & ng->_funcCount);
    om.push_back(0x08);
    om.push_back(NET_VERSION >> 8);
    om.push_back(0xFF & NET_VERSION);
    
    om.push_back(MAGIC_NUM);
    om.push_back(ng->_funcCount >> 8);
    om.push_back(0xFF & ng->_funcCount);
    om.push_back(0x06);
    om.push_back(num(ng->_game.state()));
    
    std::size_t sent = 0;
    while (sent < om.size())
      {
        sent += write(ng->_sockfd, &om[sent], om.size() - sent);
      }

    //Main loop
    while (!ng->_killThread)
      {
        //Check for incoming messages
        bool gotMessage = false;
        Message im(100, '\0');
        std::size_t rec = 0;
        do
          {
            rec += read(ng->_sockfd, (&im[rec]), 4-rec);
          }
        while (rec < 4 && rec > 0);

        //If we did get a header, get the rest and process it
        if (rec >= 4)
          {
            gotMessage = true;
            
            //If the magic number is wrong, bail for now
            //NOTE: Make this more intelligent later!
            if (im[0] != MAGIC_NUM)
              {
                break;
              }

            //Always compare function counters
            //NOTE: How common is this? Do we need to resolve desyncs?
            std::uint16_t otherFC = (im[1] << 8) + im[2];
            if (ng->_funcCount != otherFC)
              {
                break;
              }

            //Handle the message depending on type
            //NOTE: Find a better way to resolve non-success returns
            switch (im[3])
              {
              case 0: //setArmy
                {
                  while (rec < 6)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 6-rec);
                    }
                  GameReturnType r = ng->_game.setArmy(static_cast<SideType>(im[4]),
                                                       static_cast<ArmyType>(im[5]));
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 1: //start
                {
                  GameReturnType r = ng->_game.start();
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 2: //move
                {
                  while (rec < 10)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 10-rec);
                    }
                  Move m(Position(im[4],im[5]),Position(im[6],im[7]),
                         static_cast<PieceType>(im[8]),
                         static_cast<SideType>(im[9]));
                  GameReturnType r = ng->_game.move(m);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 3: //startDuel
                {
                  while (rec < 5)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 5-rec);
                    }
                  GameReturnType r = ng->_game.startDuel(im[4]==0?false:true);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 4: //bid
                {
                  while (rec < 6)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 6-rec);
                    }
                  GameReturnType r = ng->_game.bid(static_cast<SideType>(im[4]),
                                                   im[5]);
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 5: //promote
                {
                  while (rec < 5)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 5-rec);
                    }
                  GameReturnType r = ng->_game.promote(static_cast<PieceType>(im[4]));
                  if (r != GameReturnType::SUCCESS) ng->_killThread = true;
                  ng->_funcCount++;
                  break;
                }
                
              case 6: //state
                {
                  while (rec < 5)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 5-rec);
                    }
                  if (im[4] != num(ng->_game.state())) ng->_killThread = true;
                  break;
                }
                
              case 7: //ack
                {
                  while (rec < 5)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 5-rec);
                    }
                  //Right now, we do nothing with acks, but they could be
                  //useful in the future for resolving desyncs
                  break;
                }

              case 8: //version
                {
                  while (rec < 6)
                    {
                      rec += read(ng->_sockfd, (&im[rec]), 6-rec);
                    }
                  std::uint16_t otherVer = (im[4] << 8) + im[5];
                  if (NET_VERSION != otherVer) ng->_killThread = true;
                  break;
                }
                
              default: break;
              }

            //Message has been processed, send ack for anything but ack
            if (im[3] != 0x07)
              {
                om.clear();
                om.push_back(MAGIC_NUM);
                om.push_back(ng->_funcCount >> 8);
                om.push_back(0xFF & ng->_funcCount);
                om.push_back(0x07);
                om.push_back(im[3]);
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
            sent = 0;
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
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x00);
    om.push_back(num(side));
    om.push_back(num(army));
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

  GameReturnType NetGame::start()
  {
    GameReturnType r = _game.start();
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x01);
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

  GameReturnType NetGame::move(const Move& m)
  {
    GameReturnType r = _game.move(m);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x02);
    om.push_back(m.start.x());
    om.push_back(m.start.y());
    om.push_back(m.end.x());
    om.push_back(m.end.y());
    om.push_back(num(m.type));
    om.push_back(num(m.side));
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

  GameReturnType NetGame::startDuel(bool d)
  {
    GameReturnType r = _game.startDuel(d);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x03);
    om.push_back(d?0x01:0x00);
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

  GameReturnType NetGame::bid(SideType side, std::uint8_t stones)
  {
    GameReturnType r = _game.bid(side, stones);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x04);
    om.push_back(num(side));
    om.push_back(stones);
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

  GameReturnType NetGame::promote(PieceType newType)
  {
    GameReturnType r = _game.promote(newType);
    if (r != GameReturnType::SUCCESS) return r;

    Message om;
    om.push_back(MAGIC_NUM);
    om.push_back(_funcCount >> 8);
    om.push_back(0xFF & _funcCount);
    om.push_back(0x03);
    om.push_back(num(newType));
    _outMessage.push(om);

    _funcCount++;
    return r;
  }

} //Namespace
