/*
  -----SidebarObject Class Header-----
  Auston Sterling
  austonst@gmail.com

  An object to be included in a Sidebar. An object consists of multiple
  SDL_Textures laid out horizontally across the sidebar. Objects can handle
  being clicked on.

  This class takes no ownership over its SDL_Texture*s. They must be provided
  from an external source and freed by an external source.

  As this class is designed to be used in a std::set within the corresponding
  Sidebar class, most variables are marked mutable. A "const" sidebar object
  is one whose weight and ID cannot change. All other values do not affect
  sorting and can be changed.

  Spacing is determined by a spacing type and a number of pixels to put between
  squished textures. A spacing type can squish textures to one side or towards
  the center, in which case the images will be spaced by the given number of
  pixels. If the spacing type is uniform, the inter-texture spacing is
  automatically determined to "justify" the images.
*/

#ifndef _sidebarobject_hpp_
#define _sidebarobject_hpp_

#include <SDL.h>
#include <SDL_image.h>
#include <vector>
#include <cstdint>
#include <string>

//Defines the ways in which textures can be horizontally spaced
enum class SpacingType : std::uint8_t
{
  SQUISH_LEFT,
    SQUISH_CENTER,
    SQUISH_RIGHT,
    UNIFORM
    };

//Defines the ways in which textures can be vertically aligned
enum class VertAlignType : std::uint8_t
{
  FLUSH_UP,
    FLUSH_DOWN,
    CENTER
    };

struct SidebarObjectClickResponse
{
  //The index of the texture clicked on. If none, this will be -1
  int texture;

  //The coordinates in the texture's coordinate space of the click
  //If no texture clicked, these don't matter, but will still be 0
  int texX, texY;
};
  
class SidebarObject
{
public:
  //Constructors
  //Default, has no images
  SidebarObject();

  //Sets up the given textures
  SidebarObject(const std::vector<SDL_Texture*>& image, int sidebarWidth,
                SpacingType space = SpacingType::SQUISH_CENTER,
                int interspace = 0,
                VertAlignType align = VertAlignType::FLUSH_UP);

  //Sets up an empty sidebar object which can be later filled and spaced
  SidebarObject(std::size_t n, int sidebarWidth);

  //Getters and setters
  //Changing a texture without respacing may cause the spacing to be incorrect
  //if the width changes. Only SQUISH_LEFT is guaranteed to stay correct.
  void setTexture(std::size_t i, SDL_Texture* t) const;
  SDL_Texture* texture(std::size_t i) const {return image_[i];}
  void setMaxWidth(int w) const {width_ = w;}
  int maxWidth() const {return width_;}
  int height() const {return height_;}
  void setVertAlign(VertAlignType vat) const {align_ = vat;}
  VertAlignType vertAlign() const {return align_;}
  //Changing the weight or ID of an object already in a sidebar will break!
  //There are NO checks on this; so don't be stupid!
  void prepareForInsert(int w, const std::string& id);
  int weight() const {return weight_;}
  std::string id() const {return id_;}
  bool visible() const {return visible_;}
  void setVisibility(bool vis) const {visible_ = vis;}

  //Higher level functions
  //Changes the number of images, which requires a respacing
  void resizeAndRespace(std::size_t n,
                        SpacingType space = SpacingType::SQUISH_CENTER,
                        int interspace = 0) const;

  //Sets the spacing between objects
  void respace(SpacingType space = SpacingType::SQUISH_CENTER,
               int interspace = 0) const;

  //Draws the object to the given SDL_Renderer given the top left coords
  void render(SDL_Renderer* rend, int x, int y) const;

  //Given coordinates of a click in object space, returns a struct detailing
  //what that click hit
  SidebarObjectClickResponse click(int x, int y) const;
    
private:
  //The images to lay out horizontally
  mutable std::vector<SDL_Texture*> image_;

  //The spacing between images. This is the same size as image_ and says
  //how many pixels are left of the corresponding image.
  mutable std::vector<int> space_;

  //The maximum width of the object, used to calulate spacing
  mutable int width_;

  //The height of the tallest texture. This is updated as needed
  mutable int height_;
  void computeHeight() const;

  //The vertical aligning style to be used when rendering
  mutable VertAlignType align_;

  //These are values related to the object's placement within the sidebar
  //The "weight" of the object to determine its vertical sidebar position
  int weight_;

  //An identifying string
  std::string id_;

  //A flag determining if the object is visible
  mutable bool visible_;
};

class sboLighterThan
{
public:
  bool operator()(const SidebarObject& sbo1, const SidebarObject& sbo2) const;
};

#endif
