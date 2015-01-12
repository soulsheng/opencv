
#include "faceRecognizeST.h"

#include <fstream>



#define FILE_LIST_NAME	"at.txt"
#define FILE_RESULT_NAME	"out.txt"
#define FILE_DATABASE_ITEMS	"st_items.bin"
#define FILE_DATABASE_NAMES	"st_names.bin"

#define DATA_FILE_COUNT	5
char  dataFileList[][50]={
	"data/align_init.model",
	"data/align_transfer.bin",
	"data/model_verif.bin",
	"data/params.txt",
	"data/verify_nets.model"
	};

SenseTimeSDK::SenseTimeSDK()
{
	hIndex = NULL;
	bTrain = false;
	bInitialized = false;
	bReleased = false;

	pface=NULL;
	countFace=0;

	db_id = 0;

	initialize();
}

SenseTimeSDK::~SenseTimeSDK()
{
	release();
}

bool SenseTimeSDK::checkDataFile()
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

bool SenseTimeSDK::initialize()
{
	if( bInitialized )	
		return true;
	
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

	bInitialized = true;
	return true;
}

bool SenseTimeSDK::release()
{
	if (bReleased)
		return true;

	fflush(stdout);

	if(flist) fclose(flist);
	if(of) fclose(of);

	if(countFace) mcv_facesdk_release_multiview_result(pface,countFace);

	if(hIndex) mcv_verify_search_release_index(hIndex);
	if(hDetect) mcv_facesdk_destroy_multiview_instance(hDetect);
	if(vinst) mcv_verify_release_instance(vinst);

	imageSamples.clear();

	bReleased = true;

	return true;
}

bool SenseTimeSDK::train( vector<cv::Mat>&	imageSamples )
{

	if ( false == bInitialized )
		initialize();

	if ( bTrain )
		return true;
	


	/* generate feature database */
	for( int i = 0; i< names.size(); i++ )
	{
		
		fprintf(stderr, "Training %s\n", names[i]);

		db_item item = getFeature( imageSamples[i] );

		if( item.idx != -1 )
			items.push_back(item);

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


bool SenseTimeSDK::predict()
{

	if ( false == bInitialized )
		initialize();

	if ( false == bTrain )
		train( FILE_LIST_NAME );

	mcv_face_search_result_t results[10];
	unsigned int result_cnt = 0;
	/* use a db_item as query */
	const struct db_item *query = &items[0];
	mcv_result_t ret = mcv_verify_search_face(vinst, 
			hIndex, query,
			results, 10, &result_cnt);
	assert(ret == MCV_OK);
	fprintf( of, "%s\t%s\t%s\t\t\t\t%s\t\t%s\n", "i", "idx", "names", 
		"score", "rank_score");
	for(unsigned int i = 0; i < result_cnt; i++){
		int idx = results[i].item->idx;
		if( idx < items.size() )
			fprintf( of, "%d\t%d\t%s\t%f\t%f\n", i, idx, names[idx].c_str(), 
				results[i].score, results[i].rank_score);
	}

	printf("done!search %d results and save to file %s \n", result_cnt, FILE_RESULT_NAME );		

	return true;
}

bool SenseTimeSDK::predict( cv::Mat& imageFace, std::vector<int>& lableTop, int n )
{

	if ( false == bInitialized )
		initialize();

	if ( false == bTrain )
		train( FILE_LIST_NAME );

	db_item item = getFeature( imageFace );

	if( item.idx == -1 )
		return false;

	mcv_face_search_result_t results[10];
	unsigned int result_cnt = 0;
	/* use a db_item as query */
	const struct db_item *query = &item;
	mcv_result_t ret = mcv_verify_search_face(vinst, 
		hIndex, query,
		results, 10, &result_cnt);

	if( result_cnt < n )
		n = result_cnt;

	for ( int i = 0; i < n; i++ )
	{
		int idItem = results[i].item->idx;
		int idLabel = labelSamples[ idItem ];
		lableTop.push_back( idLabel );
	}

	return true;
}

bool SenseTimeSDK::save( std::string fileImageFetures )
{
	FILE *file = fopen( fileImageFetures.c_str(), "wb" );

	fprintf(file, "%d \n", items.size() );
	for ( int i=0; i<items.size(); i++ )
	{
		fwrite( (void*)&items[i], sizeof(db_item), 1, file );
	}

	fclose( file );
#if 0
	file = fopen( fileNames.c_str(), "wb" );

	fprintf(file, "%d \n", names.size() );
	for ( int i=0; i<names.size(); i++ )
	{
		std::string& name = names[i];
		fprintf(file, "%d:", name.size() );
		fwrite( (void*)name.c_str(), name.size(), 1, file );
		fprintf(file, "\n" );
	}

	fclose( file );
#endif
	return true;
}

bool SenseTimeSDK::load( std::string fileImageFetures )
{

	if ( false == bInitialized )
		initialize();

	items.clear();
	names.clear();

	FILE *file = fopen( fileImageFetures.c_str(), "rb" );

	int nSize = 0;

	fscanf(file, "%d \n", &nSize );
	for ( int i=0; i<nSize; i++ )
	{
		db_item item;
		fread( (void*)&item, sizeof(db_item), 1, file );
		items.push_back(item);
	}

	fclose( file );
#if 0
	file = fopen( fileNames.c_str(), "rb" );

	fscanf(file, "%d \n", &nSize );
	for ( int i=0; i<nSize; i++ )
	{
		int nLength = 0;
		fscanf(file, "%d:", &nLength );

		char buf[1024];
		fread( buf, nLength, 1, file );
		buf[nLength] = '\0';

		names.push_back(buf);
	}

	fclose( file );
#endif

	prepareSamples( FILE_LIST_NAME );

	mcv_result_t ret = mcv_verify_search_build_index(vinst,
		&items[0], items.size(), &hIndex);

	assert(hIndex != 0 && ret == MCV_OK);

	bTrain = true;

	return true;
}

bool SenseTimeSDK::checkTrained()
{
	if (bTrain)
		return true;

	FILE *file = fopen( FILE_DATABASE_ITEMS, "rb" );
	if ( file )
	{
		fclose( file );
		return load( FILE_DATABASE_ITEMS );
	}
	else
	{
		if ( !train( FILE_LIST_NAME ) )
			return false;

		return save( FILE_DATABASE_ITEMS );	
	}
}


bool SenseTimeSDK::faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, vector<cv::Mat>& matimg)
{
	cv::Mat gray;
	cvtColor( imgIn, gray, CV_BGR2GRAY );
	cv::Mat *img = &gray;

	// detect
	mcv_facesdk_multiview_detector(hDetect,img->data,img->cols,img->rows,img->cols,&pface,&countFace);

	// draw result
	for ( int i=0;i<countFace;i++){
		rectangle( imgOut, 
			cvPoint( pface[i].Rect.left, pface[i].Rect.top ),
			cvPoint( pface[i].Rect.right, pface[i].Rect.bottom ) ,
			CV_RGB(0,0,255), 3, 8, 0);

		cv::Mat imgROI( imgIn, cv::Rect(
			pface[i].Rect.left, pface[i].Rect.top ,
			pface[i].Rect.right - pface[i].Rect.left, pface[i].Rect.bottom - pface[i].Rect.top ));
		//printf("width = %d \n", pface[i].Rect.right - pface[i].Rect.left);
		matimg.push_back(imgROI);

	}

	return true;
}

bool SenseTimeSDK::prepareSamples( std::string filelist )
{
	imageSamples.clear();
	names.clear();
	labelSamples.clear();

	cv::Mat imgIn;
	
	std::ifstream file(filelist.c_str(), ifstream::in);
	if (!file) {
		printf( "cannot to open %s \n", filelist.c_str() );
	}

	string line, path, classlabel;

	while( getline(file, line) )
	{
		stringstream liness(line);
		getline(liness, path, ';');

		liness >> classlabel;

		imgIn = cv::imread( path );

		imageSamples.push_back( imgIn );
		names.push_back( path );

		labelSamples.push_back( atoi( classlabel.c_str() ) );
	}
	file.close();


	return true;
}

bool SenseTimeSDK::train( std::string filelist )
{
	prepareSamples( filelist );

	return train( imageSamples );
}

db_item SenseTimeSDK::getFeature( cv::Mat& imageIn )
{
	cv::Mat gray, imgInBGRA;

	cvtColor( imageIn, gray, CV_BGR2GRAY );
	cvtColor( imageIn, imgInBGRA, CV_BGR2BGRA );

	PMCV_FACERECT pface=NULL;
	unsigned int fcount = 0;
	mcv_facesdk_multiview_detector(hDetect, gray.data, gray.cols, gray.rows, 
		gray.cols,&pface,&fcount);

	db_item item;
	item.idx = -1;

	if(fcount){
		memset(&item, 0, sizeof(item));
		mcv_result_t ret = mcv_verify_search_get_feature(vinst, imgInBGRA.data, imgInBGRA.cols,
			imgInBGRA.rows,  pface[0].Rect, &item);
		assert(ret == MCV_OK);
		item.idx = db_id++;
	}

	mcv_facesdk_release_multiview_result(pface, fcount);

	return item;
}
