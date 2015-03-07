/*
  -----SidebarObject Class Implementation-----
  Auston Sterling
  austonst@gmail.com

  Implementation of the SidebarObject class.
*/

#include "sidebarobject.hpp"

SidebarObject::SidebarObject() :
  width_(0),
  height_(0),
  align_(VertAlignType::FLUSH_UP),
  weight_(0),
  visible_(true)
{}

SidebarObject::SidebarObject(const std::vector<SDL_Texture*>& image,
                             int sidebarWidth, SpacingType space,
                             int interspace, VertAlignType align) :
  image_(image),
  width_(sidebarWidth),
  align_(align),
  weight_(0),
  visible_(true)
{
  respace(space, interspace);
  computeHeight();
}

SidebarObject::SidebarObject(std::size_t n, int sidebarWidth) :
  width_(sidebarWidth),
  height_(0),
  align_(VertAlignType::FLUSH_UP),
  weight_(0),
  visible_(true)
{
  image_.resize(n, nullptr);
  space_.resize(n, 0);
}

void SidebarObject::setTexture(std::size_t i, SDL_Texture* t) const
{
  //Update value
  if (i >= image_.size()) return;
  image_[i] = t;

  //Check for max height increase
  int texW = 0, texH = 0;
  if (t != nullptr)
    {
      SDL_QueryTexture(t, NULL, NULL, &texW, &texH);
    }
  
  if (texH > height_)
    {
      height_ = texH;
    }
  else
    {
      computeHeight();
    }
}

void SidebarObject::prepareForInsert(int w, const std::string& id)
{
  weight_ = w;
  id_ = id;
}

void SidebarObject::resizeAndRespace(std::size_t n, SpacingType space,
                                     int interspace) const
{
  image_.resize(n, nullptr);
  respace(space, interspace);
}

void SidebarObject::respace(SpacingType space, int interspace) const
{
  //In most cases, most spaces are interspace
  space_ = std::vector<int>(image_.size(), interspace);
  if (space_.empty()) return;

  //Find the total width of all textures
  int texWidth = 0;
  for (SDL_Texture* im : image_)
    {
      int texW = 0, texH = 0;
      if (im != nullptr)
        {
          SDL_QueryTexture(im, NULL, NULL, &texW, &texH);
        }
      texWidth += texW;
    }

  //Find the total width of the spaces between (but not before) textures
  int spaceWidth = interspace * (space_.size()-1);

  //And finally, the extra space that we need to place properly
  int extraSpace = width_ - (texWidth + spaceWidth);
    
  switch(space)
    {
    case SpacingType::SQUISH_LEFT:
      //Align the leftmost texture with the side, all others are okay
      space_[0] = 0;
      break;

    case SpacingType::SQUISH_CENTER:
      //The starting point is after half of the extra space
      space_[0] = extraSpace/2;
      break;
        
    case SpacingType::SQUISH_RIGHT:
      //Align the leftmost texture as far over as possible, all others are okay
      space_[0] = extraSpace;
      break;

    case SpacingType::UNIFORM:
      //Align the leftmost texture with the side, allocate extra space evenly
      //When space cannot be allocated perfectly evenly, add one more pixel
      //to some of the spaces so the numbers come out right
      space_[0] = 0;
      int newspace = interspace + extraSpace/(space_.size()-1);
      std::size_t addOneMoreUntil = (extraSpace % (space_.size()-1)) + 1;
      for (std::size_t i = 1; i < addOneMoreUntil; i++)
        {
          space_[i] = newspace + 1;
        }
      for (std::size_t i = addOneMoreUntil; i < space_.size(); i++)
        {
          space_[i] = newspace;
        }
      break;
    }
}

void SidebarObject::render(SDL_Renderer* rend, int x, int y) const
{
  int offX = 0;
  for (std::size_t i = 0; i < image_.size(); i++)
    {
      //Get texture dimensions
      int texW = 0, texH = 0;
      if (image_[i] != nullptr)
        {
          SDL_QueryTexture(image_[i], NULL, NULL, &texW, &texH);
        }
        
      //Set up rectangles
      offX += space_[i];
      SDL_Rect srcRec = {0, 0, texW, texH};
      SDL_Rect destRec = {x+offX, y, texW, texH};

      //Align vertically
      if (align_ == VertAlignType::FLUSH_DOWN)
        {
          destRec.y += height_ - texH;
        }
      else if (align_ == VertAlignType::CENTER)
        {
          destRec.y += (height_ - texH)/2;
        }

      //Render
      SDL_RenderCopy(rend, image_[i], &srcRec, &destRec);

      //Step down to the next one
      offX += texW;
    }
}

SidebarObjectClickResponse SidebarObject::click(int x, int y) const
{
  //Construct return struct
  SidebarObjectClickResponse sbocr;
  sbocr.texture = -1;
  sbocr.texX = sbocr.texY = 0;

  //Check bounds
  if (x < 0 || x >= width_ || y < 0 || y >= height_)
    {
      return sbocr;
    }
    
  int offX = 0;
  for (std::size_t i = 0; i < image_.size(); i++)
    {
      //Get texture dimensions
      int texW = 0, texH = 0;
      if (image_[i] != nullptr)
        {
          SDL_QueryTexture(image_[i], NULL, NULL, &texW, &texH);
        }
        
      //Check if it it the spacing on the left side
      if ((offX += space_[i]) > x)
        {
          return sbocr;
        }

      //Check if it hit the texture itself
      if ((offX += texW) > x)
        {
          if (align_ == VertAlignType::FLUSH_UP && y < texH)
            {
              sbocr.texY = y;
            }
          else if (align_ == VertAlignType::FLUSH_DOWN && y > height_ - texH)
            {
              sbocr.texY = y - (height_ - texH);
            }
          else if (align_ == VertAlignType::CENTER &&
                   y > (height_ - texH)/2 &&
                   y < height_ - (height_ - texH)/2)
            {
              sbocr.texY = y - (height_ - texH)/2;
            }
          else
            {
              //Click missed it vertically
              return sbocr;
            }

          //We have a hit, finish up
          sbocr.texture = i;
          sbocr.texX = x - (offX - texW);
          return sbocr;
        }
    }

  //No hits found, return the "miss" struct we set up
  return sbocr;
}

void SidebarObject::computeHeight() const
{
  height_ = 0;
  for (SDL_Texture* im : image_)
    {
      int texW = 0, texH = 0;
      if (im != nullptr)
        {
          SDL_QueryTexture(im, NULL, NULL, &texW, &texH);
        }
      height_ = std::max(height_, texH);
    }
}

bool sboLighterThan::operator()(const SidebarObject& sbo1,
                                const SidebarObject& sbo2) const
{
  if (sbo1.weight() == sbo2.weight())
    {
      return sbo1.id() < sbo2.id();
    }
  return sbo1.weight() < sbo2.weight();
}
