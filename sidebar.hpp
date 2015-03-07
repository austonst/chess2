/*
  -----Sidebar Class Header-----
  Auston Sterling
  austonst@gmail.com

  A renderable SDL sidebar for Chess 2. This is a modular system consisting of
  objects laid out vertically.

  Sidebar objects are copied when given to the sidebar, but can be later accessed
  and modified if needed.

  Each sidebar object has a "weight" and an ID which uniquely identify the object
  and determine the top-to-bottom order; heavier objects sink down and ligter
  objects float up.

  An object of weight < 1000 will rise to the top; an object of weight >=1000 will
  sink to the bottom.

  Clicks are handled by returning a struct containing an iterator to the
  clicked object, the index of the clicked texture, and the coordinates of the
  click in the *texture's* coordinate space.
*/

#ifndef _sidebar_hpp_
#define _sidebar_hpp_

#include "sidebarobject.hpp"

#include <set>

const int SIDEBAR_SINK_CUTOFF = 1000;

//Forward declaration -- make the compiler happy :)
struct SidebarClickResponse;
  
class Sidebar
{
public:

  typedef std::set<SidebarObject, sboLighterThan>::iterator iterator;
  typedef std::set<SidebarObject, sboLighterThan>::const_iterator const_iterator;
    
  //Constructors
  Sidebar();
  Sidebar(int w, int h, SDL_Color bg, int spacing = 0);

  //Accessors and mutators for simple variables
  void setWidth(int w) {width_ = w;}
  int width() const {return width_;}
  void setHeight(int h) {height_ = h;}
  int height() const {return height_;}
  void setBGColor(SDL_Color bg) {bgColor_ = bg;}
  SDL_Color bgColor() const {return bgColor_;}

  //Object management
  //Inserts the object assuming it's been set up with weight and id already
  void insertObject(const SidebarObject& sbo);
    
  //Sets the weight and ID of an object, then inserts it
  void insertObject(SidebarObject& sbo, int weight, const std::string& id);

  //Creates an empty object with the given weight and id to be built up later
  //Returns an interator to it in case you want to get started on it
  //The object is informed of the width of the sidebar
  iterator createObject(int weight, const std::string& id);

  //Deletes the object with the given weight and ID
  void deleteObject(int w, const std::string& id);

  //Retrieves an iterator to an object given the weight and ID
  iterator object(int w, const std::string& id);

  //Retrieves an iterator to one of the possibly many objects with the given ID
  iterator object(const std::string& id);

  //Other functions
  //Checks if an iterator is valid and can be dereferenced
  bool isValid(const_iterator it) const;
    
  //Draws the object to the given SDL_Renderer given the top left coords
  void render(SDL_Renderer* rend, int x, int y) const;

  //Given the coordinates of a click in sidebar space, returns more detailed
  //information about what was clicked on
  SidebarClickResponse click(int x, int y) const;
    
private:
  //The sidebar objects, sorted by weight
  std::set<SidebarObject, sboLighterThan> object_;

  //The dimensions of the sidebar
  int width_, height_;

  //The background color
  SDL_Color bgColor_;

  //The amount of vertical spacing to put between each object
  int spacing_;
};

struct SidebarClickResponse
{
  //The object clicked on. If they didn't hit any, isValid() will be false
  Sidebar::iterator sbo;

  //The index of the texture clicked on. If none, this will be -1
  int texture;

  //The coordinates in the texture's coordinate space of the click
  //If no texture clicked, these don't matter, but will still be 0
  int texX, texY;
};

#endif
