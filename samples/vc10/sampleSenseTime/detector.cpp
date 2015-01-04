#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>

#include "imagehelper.hpp"

using namespace std;
using namespace sdktest;

/// \lib facesdk
/// \api mcv_facesdk_multiview_detector
/// \desc 能够检测出不同姿态方位的人脸在图中的位置 

int main(int argc, char *argv[])
{
	assert(argc == 3);
	ImageHelper *helper = new ImageHelperBackend();

	// init handle
	mcv_handle_t hDetect = mcv_facesdk_create_multiview_detector_instance_from_resource(true, 2);

	Image *img=helper->LoadGrayImage(argv[1]);
	Image *img_color = helper->LoadRGBAImage(argv[1]);
	if(!img || !img_color){
		fprintf(stderr, "fail to read %s\n", argv[1]);
		return -1;
	}

	// detect
	PMCV_FACERECT pface=NULL;
	unsigned int count;
	__TIC__();
	mcv_facesdk_multiview_detector(hDetect,img->data,img->cols,img->rows,img->cols,&pface,&count);
	__TOC__();

	// draw result
	for ( int i=0;i<count;i++){
		helper->DrawRect(img_color,pface[i].Rect.top,pface[i].Rect.left,pface[i].Rect.right,pface[i].Rect.bottom, 3);
		printf("face %d: %d %d %d %d\n", i, pface[i].Rect.top, pface[i].Rect.left, pface[i].Rect.right, pface[i].Rect.bottom);
	}

	// release the memory of result
	mcv_facesdk_release_multiview_result(pface,count);

	// destroy handle
    mcv_facesdk_destroy_multiview_instance(hDetect);
        
	helper->SaveImage(argv[2],img_color);
	helper->FreeImage(img);
	helper->FreeImage(img_color);
	delete helper;
	return 0;
}

