/**
 * @file src/cxx/ip/src/maxRectInMask.cc
 * @author <a href="mailto:Laurent.El-Shafey@idiap.ch">Laurent El Shafey</a> 
 *
 */

#include "ip/maxRectInMask.h"

namespace ip = Torch::ip;
namespace ipd = Torch::ip::detail;

bool
ipd::isTrue( const blitz::Array<bool,2>& src, int y0, int x0, int y1, int x1)
{
  for(int j=y0; j<=y1; ++j)
    for(int i=x0; i<=x1; ++i)
      if(!src(j,i))
        return false;
  return true;
}

const blitz::TinyVector<int,4> 
ip::maxRectInMask( const blitz::Array<bool,2>& src)
{
  const int height = src.extent(0);
  const int width = src.extent(1);
  blitz::TinyVector<int,4> cur_sol = 0;
  int cur_max_area = 0;

  // Find largest rectangle visiting all the possible combinations
  for(int y0=0; y0<height; ++y0)
    for(int x0=0; x0<width; ++x0)
      for(int y1=y0; y1<height; ++y1)
        for(int x1=x0; x1<width; ++x1)
        {
          if(ipd::isTrue(src,y0,x0,y1,x1))
          {
            int area = (y1-y0+1)*(x1-x0+1);
            if( area > cur_max_area )
            {
              cur_max_area = area;
              cur_sol(0) = y0;
              cur_sol(1) = x0;
              cur_sol(2) = y1-y0+1;
              cur_sol(3) = x1-x0+1;
            }
          }
        } 

  return cur_sol;
}
