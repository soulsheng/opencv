#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>
#include <mcv_faceverify.h>

#include <vector>
#include <string>
#include "imagehelper.hpp"

using namespace std;
using namespace sdktest;

int main(int argc, char const *argv[])
{
	assert(argc == 2);
	ImageHelper *helper = new ImageHelperBackend();
	mcv_handle_t hDetect = mcv_facesdk_create_multiview_detector_instance_from_resource(true, 1);
	assert(hDetect != 0);

	mcv_handle_t vinst = mcv_create_verify_instance();
	assert(vinst != 0);

	char line[1024];
	FILE *flist = fopen(argv[1], "r");
	assert(flist != 0);

	vector<db_item> items;
	vector<string> names;

	int db_id = 0;
	/* generate feature database */
	while(fgets(line, 1024, flist)){
		size_t len = strlen(line);
		if(line[len-1] == '\n')
			line[len-1] = 0;
		fprintf(stderr, "Reading %s\n", line);
		Image *img_color=helper->LoadRGBAImage(line);
		Image *img_gray=helper->LoadGrayImage(line);
		if(!img_color || !img_gray){
			fprintf(stderr, "Fail to read %s\n", line);
			continue;
		}

		PMCV_FACERECT pface=NULL;
		unsigned int fcount = 0;
		mcv_facesdk_multiview_detector(hDetect, img_gray->data, img_gray->cols, img_gray->rows, 
			img_gray->cols,&pface,&fcount);


		for(unsigned int i = 0; i < fcount; i++){
			db_item item;
			memset(&item, 0, sizeof(item));
			mcv_result_t ret = mcv_verify_search_get_feature(vinst, img_color->data, img_color->cols,
				img_color->rows,  pface[i].Rect, &item);
			assert(ret == MCV_OK);
			item.idx = db_id++;
			items.push_back(item);
			char buf[1024];
			snprintf(buf,1024, "%s:%d", line, i);
			names.push_back(buf);
		}

		mcv_facesdk_release_multiview_result(pface, fcount);
		helper->FreeImage(img_gray);
		helper->FreeImage(img_color);
	}

	fclose(flist);
	if(items.size() == 0){
		fprintf(stderr, "No faces\n");
		exit(1);
	}

	/* query */
	mcv_handle_t hIndex = NULL;
	mcv_result_t ret = mcv_verify_search_build_index(vinst,
		&items[0], items.size(), &hIndex);

	assert(hIndex != 0 && ret == MCV_OK);

	if(items.size() > 0){
		mcv_face_search_result_t results[10];
		unsigned int result_cnt = 0;
		/* use a db_item as query */
		const struct db_item *query = &items[0];
		ret = mcv_verify_search_face(vinst, &items[0], 
				hIndex, query,
				results, 10, &result_cnt);
		assert(ret == MCV_OK);
		for(unsigned int i = 0; i < result_cnt; i++){
			int idx = results[i].item->idx;
			printf("%d\t%d\t%s\t%f\t%f\n", i, idx, names[idx].c_str(), 
				results[i].score, results[i].rank_score);
		}
		fflush(stdout);
	}

	mcv_verify_search_release_index(hIndex);
	mcv_facesdk_destroy_multiview_instance(hDetect);
	mcv_verify_release_instance(vinst);

	delete helper;
	return 0;
}
