#include "FrontalFace64x80.h"

namespace Torch {

FrontalFace64x80::FrontalFace64x80(int postype_) : FaceModel(postype_)
{
	D_EYES = 33.0;
	Y_UPPER = 16.0;
	Y_LOWER = 63.0;

	model_width = 64.0;
	model_height = 80.0;
}

bool FrontalFace64x80::ldm2bbx()
{
	const bool verbose = getBOption("verbose");

	double c0x, c0y, EEx, EEy;

	sPoint2D l_eye = getLeye();
	sPoint2D r_eye = getReye();

	EEx = r_eye.x - l_eye.x;
	EEy = r_eye.y - l_eye.y;

	// The face is non oriented
	if((int) r_eye.y != (int) l_eye.y)
	   	warning("FrontalFace64x80: y coordinates of eyes should be equal (%d == %d)", (int) r_eye.y, (int) l_eye.y);

	c0x = l_eye.x + EEx / 2.0;
	c0y = l_eye.y + EEy / 2.0;

	if(verbose)
		print("FrontalFace64x80: c0x=%g c0y=%g\n", c0x, c0y);

	double ratio = EEx / D_EYES;

	bbx_width = (int) (ratio * model_width + 0.5);
	if(bbx_width % 2 == 0) bbx_width += 1;

	bbx_height = (int) (ratio * (Y_UPPER + Y_LOWER) + 0.5);
	if(bbx_height % 2 == 0) bbx_height += 1;

	int upperleft_x;
	int upperleft_y;

	/* The upperleft x coordinate of the face is given
	   by the middle of the face minus the half size of the face */
	upperleft_x = (int) (c0x - (ratio * model_width / 2.0));

	/* The upperleft y coordinate of the face is given
	   by the constants #en_gn# and #tr_gn# */
	upperleft_y = (int) (c0y - (ratio * Y_UPPER));

	if(verbose)
		print("FrontalFace64x80: x=%d y=%d w=%d h=%d\n", upperleft_x, upperleft_y, bbx_width, bbx_height);

	// Filling bbx
	bbx_points[0].x = upperleft_x;
	bbx_points[0].y = upperleft_y;

	bbx_points[1].x = upperleft_x + bbx_width;
	bbx_points[1].y = upperleft_y;

	bbx_points[2].x = upperleft_x + bbx_width;
	bbx_points[2].y = upperleft_y + bbx_height;

	bbx_points[3].x = upperleft_x;
	bbx_points[3].y = upperleft_y + bbx_height;

	bbx_points[4] = bbx_points[0];

	bbx_center.x = (bbx_points[0].x + bbx_points[1].x) / 2;
	bbx_center.y = (bbx_points[0].y + bbx_points[2].y) / 2;

	return true;
}

bool FrontalFace64x80::bbx2ldm()
{
	warning("FrontalFace64x80::bbx2ldm() not implemented.");
	return false;
}

bool FrontalFace64x80::bbx2ldm(int n_bbx_points_, sPoint2D *bbx_points_, int n_ldm_points_, sPoint2D *ldm_points_)
{
	warning("FrontalFace64x80::bbx2ldm(...) not implemented.");
	return false;
}

FrontalFace64x80::~FrontalFace64x80()
{
}

}
