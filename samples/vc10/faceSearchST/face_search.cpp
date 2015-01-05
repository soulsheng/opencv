#include <iostream>
#include <cassert>
#include <mcv_facesdk.h>
#include <mcv_faceverify.h>

#include <vector>
#include <string>
#include "imagehelper.hpp"

using namespace std;
using namespace sdktest;

#define FILE_LIST_NAME	"at.txt"
#define FILE_RESULT_NAME	"out.txt"

#define DATA_FILE_COUNT	5
char  dataFileList[][50]={
	"data/align_init.model",
	"data/align_transfer.bin",
	"data/model_verif.bin",
	"data/params.txt",
	"data/verify_nets.model"
	};


ImageHelper *helper;
FILE *flist, *of;
mcv_handle_t hDetect;
mcv_handle_t vinst;
vector<db_item> items;
vector<string> names;
mcv_handle_t hIndex = NULL;
bool bTrain = false;

bool checkDataFile()
{
	FILE* file = NULL;
	for (int i=0;i<DATA_FILE_COUNT;i++)
	{	
		file = fopen( dataFileList[i], "r" );
		if ( NULL ==file )
		{
			printf( "failed to open file %s \n", dataFileList[i] );
			return false;
		}
		fclose(file);
	}
	return true;
}

bool initialize()
{
	helper = new ImageHelperBackend();
	hDetect = mcv_facesdk_create_multiview_detector_instance_from_resource(true, 1);
	if ( NULL == hDetect)
	{
		printf( "failed to create detector \n" );
		return false;
	}

	if ( !checkDataFile() )
		return false;

	vinst = mcv_create_verify_instance();
	if ( NULL == vinst)
	{
		printf( "failed to create verify \n" );
		return false;
	}


	flist = fopen(FILE_LIST_NAME, "r");
	if ( NULL == flist )
	{
		printf( "failed to open file %s \n", FILE_LIST_NAME );
		return false;
	}

	of = fopen( FILE_RESULT_NAME, "w" );
	if ( NULL == of)
	{
		printf( "failed to open file %s \n", FILE_RESULT_NAME );
		return false;
	}

}

bool release()
{
	fflush(stdout);

	fclose(flist);
	fclose(of);

	mcv_verify_search_release_index(hIndex);
	mcv_facesdk_destroy_multiview_instance(hDetect);
	mcv_verify_release_instance(vinst);

	delete helper;
	return true;
}

bool train()
{
	if ( bTrain )
		return true;

	char line[1024];
	int db_id = 0;
	/* generate feature database */
	while(fgets(line, 1024, flist)){
		size_t len = strlen(line);
		if(line[len-1] == '\n')
			line[len-1] = 0;
		fprintf(stderr, "Training %s\n", line);
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
			_snprintf(buf,1024, "%s:%d", line, i);
			names.push_back(buf);
		}

		mcv_facesdk_release_multiview_result(pface, fcount);
		helper->FreeImage(img_gray);
		helper->FreeImage(img_color);
	}

	if(items.size() == 0){
		fprintf(stderr, "No faces\n");
	}

	mcv_result_t ret = mcv_verify_search_build_index(vinst,
		&items[0], items.size(), &hIndex);

	assert(hIndex != 0 && ret == MCV_OK);

	bTrain = true;

	return true;
}


bool predict()
{

	if ( false == bTrain )
		train();

	mcv_face_search_result_t results[10];
	unsigned int result_cnt = 0;
	/* use a db_item as query */
	const struct db_item *query = &items[0];
	mcv_result_t ret = mcv_verify_search_face(vinst, 
			hIndex, query,
			results, 10, &result_cnt);
	assert(ret == MCV_OK);
	fprintf( of, "%s\t%s\t%s\t\t\t\t\t%s\t\t%s\n", "i", "idx", "names", 
		"score", "rank_score");
	for(unsigned int i = 0; i < result_cnt; i++){
		int idx = results[i].item->idx;
		fprintf( of, "%d\t%d\t%s\t%f\t%f\n", i, idx, names[idx].c_str(), 
			results[i].score, results[i].rank_score);
	}

	printf("done!search %d results and save to file %s \n", result_cnt, FILE_RESULT_NAME );		

	return true;
}

int main(int argc, char const *argv[])
{

	if ( !initialize() )
		return false;

	/* train */
	if ( !train() )
		return false;

	train();
	train();

	/* query */
	if ( !predict() )
		return false;

	predict();
	predict();

	release();

	system( "pause" );
	return 0;
}
