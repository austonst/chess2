/*
  -----Sidebar Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  Implementation of the Sidebar class.
*/

#include "sidebar.hpp"

Sidebar::Sidebar() :
  width_(0),
  height_(0),
  spacing_(0)
{
  bgColor_ = {0,0,0,255};
}

Sidebar::Sidebar(int w, int h, SDL_Color bg, int spacing) :
  width_(w),
  height_(h),
  spacing_(spacing)
{
  bgColor_ = bg;
}

void Sidebar::insertObject(const SidebarObject& sbo)
{
  object_.insert(sbo);
}

void Sidebar::insertObject(SidebarObject& sbo, int weight,
                           const std::string& id)
{
  sbo.prepareForInsert(weight, id);
  insertObject(sbo);
}

Sidebar::iterator Sidebar::createObject(int weight, const std::string& id)
{
  SidebarObject sbo;
  sbo.setMaxWidth(width_);
  sbo.prepareForInsert(weight, id);
  insertObject(sbo);
  return object_.find(sbo);
}

void Sidebar::deleteObject(int w, const std::string& id)
{
  SidebarObject sboSearch;
  sboSearch.prepareForInsert(w, id);
  object_.erase(sboSearch);
}

Sidebar::iterator Sidebar::object(int w, const std::string& id)
{
  SidebarObject sboSearch;
  sboSearch.prepareForInsert(w, id);
  return object_.find(sboSearch);
}

Sidebar::iterator Sidebar::object(const std::string& id)
{
  //Linear time search since objects are sorted primarily by weight
  for (iterator i = object_.begin(); i != object_.end(); i++)
    {
      if (i->id() == id) return i;
    }

  //None found, end is our "not in here" iterator
  return object_.end();
}

bool Sidebar::isValid(const_iterator it) const
{
  if (it == object_.end() || object_.find(*it) == object_.end())
    {
      return false;
    }
  return true;
}

void Sidebar::render(SDL_Renderer* rend, int x, int y) const
{
  //Fill background
  SDL_SetRenderDrawColor(rend, bgColor_.r, bgColor_.g, bgColor_.b, bgColor_.a);
  SDL_Rect bgRect;
  bgRect.x = x;
  bgRect.y = y;
  bgRect.w = width_;
  bgRect.h = height_;
  SDL_RenderFillRect(rend, &bgRect);

  //Render each floating object
  iterator fwd;
  int yStep = y;
  for (fwd = object_.begin();
       fwd != object_.end() && fwd->weight() < SIDEBAR_SINK_CUTOFF;
       fwd++)
    {
      //Do not render if the object is invisible
      if (!fwd->visible()) continue;
      
      //Perform the rendering
      fwd->render(rend, x, yStep);

      //Update the y value of the next object
      yStep += spacing_ + fwd->height();
    }

  //Render each sinking object, going from the bottom up until we reach
  //the forward iterator
  sboLighterThan sboComp;
  yStep = y + height_;
  for (std::set<SidebarObject, sboLighterThan>::reverse_iterator back = object_.rbegin();
       fwd != object_.end() && back != object_.rend() && !sboComp(*back, *fwd);
       back++)
    {
      //Do not render if the object is invisible
      if (!back->visible()) continue;
      
      //Need the top left corner of the object
      yStep -= back->height();

      //Render it
      back->render(rend, x, yStep);

      //Insert spacing
      yStep -= spacing_;
    }
}

SidebarClickResponse Sidebar::click(int x, int y) const
{
  //Construct the return struct, since we always need to return one
  SidebarClickResponse sbcr;
  sbcr.sbo = object_.end();
  sbcr.texture = -1;
  sbcr.texX = sbcr.texY = 0;
    
  //Check bounds on x and y
  if (x < 0 || x >= width_ || y < 0 || y >= height_)
    {
      return sbcr;
    }
    
  //Step through the floating objects and check for an object hit
  iterator fwd;
  int yStep;
  for (fwd = object_.begin(), yStep = 0;
       fwd != object_.end() && fwd->weight() < SIDEBAR_SINK_CUTOFF;
       fwd++)
    {
      //If the object is invisible, it doesn't show itself or its spacing
      if (!fwd->visible()) continue;
      
      //yStep is currently at the top of this object; see if we hit it
      if ((yStep += fwd->height()) > y)
        {
          //Get details from the object
          SidebarObjectClickResponse sbocr = fwd->click(x, yStep - y);

          //Build up the struct and finish up
          sbcr.sbo = fwd;
          sbcr.texture = sbocr.texture;
          sbcr.texX = sbocr.texX;
          sbcr.texY = sbocr.texY;
          return sbcr;
        }

      //See if we hit the spacing between objects
      if ((yStep += spacing_) > y)
        {
          //Miss, finish up
          sbcr.sbo = fwd;
          return sbcr;
        }
    }

  //Step through each sinking object, going from the bottom up until we reach
  //the forward iterator
  sboLighterThan sboComp;
  yStep = height_;
  for (std::set<SidebarObject, sboLighterThan>::reverse_iterator back = object_.rbegin();
       fwd != object_.end() && back != object_.rend() && !sboComp(*back, *fwd);
       back++)
    {
      //If the object is invisible, it doesn't show itself or its spacing
      if (!back->visible()) continue;
      
      //See if we jump over y when moving up
      if ((yStep -= back->height()) < y)
        {
          //Get details from the object
          SidebarObjectClickResponse sbocr = back->click(x, y - yStep);

          //Build up the struct and finish up
          //Reverse iterators are funky and their "base" pointer is actually
          //one earlier than you would think. This corrects that issue.
          back++;
          sbcr.sbo = back.base();
          sbcr.texture = sbocr.texture;
          sbcr.texX = sbocr.texX;
          sbcr.texY = sbocr.texY;
          return sbcr;
        }

      //See if we hit the spacing
      if ((yStep -= spacing_) < y)
        {
          //Miss, finish up
          return sbcr;
        }
    }

  //It must be in the spacing inbetween the floaters and sinkers
  return sbcr;
}
