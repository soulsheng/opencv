
#include "faceRecognizeST.h"

#include <fstream>

#define	AUTO_TRAIN		0



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


enum	TIMER_STEP
{
	TIMER_doWork_Cap,
	TIMER_faceDetect,
	TIMER_faceSearch,
	TIMER_faceTrain,
	TIMER_COUNT
};

#define	TIME_ENABLE		1

template <> 
SenseTimeSDK* Singleton<SenseTimeSDK>::ms_pSingleton = 0;

SenseTimeSDK& SenseTimeSDK::getSingleton( void )
{
	assert( ms_pSingleton );  
	return ( *ms_pSingleton );  
}

SenseTimeSDK* SenseTimeSDK::getSingletonPtr( void )
{
	return ms_pSingleton;
}

SenseTimeSDK::SenseTimeSDK()
{
	hIndex = NULL;
	bTrain = false;
	bInitialized = false;
	bReleased = false;

	pface=NULL;
	countFace=0;

	db_id = 0;
	bForceGray = false;
	nScoreLine = 0;
	fRatioThreshold = 0.25f;	// 0.25% = 0.0025


	timer.assign( TIMER_COUNT, NULL );
	for ( TimeItr itr=timer.begin(); itr != timer.end(); itr ++ )
		sdkCreateTimer( &(*itr) );

	initialize();
}

SenseTimeSDK::~SenseTimeSDK()
{
	for ( TimeItr itr=timer.begin(); itr != timer.end(); itr ++ )
		sdkDeleteTimer( &(*itr) );

	timer.clear();

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
	
	hDetect = mcv_facesdk_create_frontal_detector_instance_from_resource(/*true,*/1);
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

	bInitialized = true;


	if( !checkTrained() )
	{
		printf( "first time to train \n" );
		return false;
	}
	else
		printf( "success to initialize \n" );

	return true;
}

bool SenseTimeSDK::release()
{
	if (bReleased)
		return true;

	fflush(stdout);

	if(countFace) mcv_facesdk_release_frontal_result(pface,countFace);

	if(hIndex) mcv_verify_search_release_index(hIndex);
	if(hDetect) mcv_facesdk_destroy_frontal_instance(hDetect);
	if(vinst) mcv_verify_release_instance(vinst);

	for (int i=0;i<imageSamples.size();i++)
		delete imageSamples[i];
	imageSamples.clear();
	imageShow.clear();

	bReleased = true;

	return true;
}

bool SenseTimeSDK::predict( cv::Mat& imageFace, std::vector<int>& lableTop, bool bLabel, bool bForceGray, int n )
{
#if TIME_ENABLE
	sdkResetTimer( &timer[TIMER_faceSearch] );
	sdkStartTimer( &timer[TIMER_faceSearch]  );
#endif

	if ( false == bInitialized )
		initialize();

	if ( false == bTrain )
#if AUTO_TRAIN
		train( FILE_LIST_NAME );
#else
	{
		cout << "not yet trained, please train face samples first!" << endl;
		return false;
	}
#endif

	this->bForceGray = bForceGray;

	db_item item = getFeature( imageFace );

	if( item.idx == -1 )
		return false;

	mcv_face_search_result_t results[10];
	unsigned int result_cnt = 0;
	/* use a db_item as query */
	const struct db_item *query = &item;
	mcv_result_t ret = mcv_verify_search_face(vinst, 
		hIndex, query,
		results, n, &result_cnt);


	for ( int i = 0; i < result_cnt; i++ )
	{
		int idItem = results[i].item.idx;
		if( results[i].score<nScoreLine )
		{
			cout << "invalid id of item " << idItem << ", score = " << results[i].score << endl;
			continue;
		}
		else
		{
			cout << "match item " << idItem << ", score = " << results[i].score << endl;
		}

		int idLabel = findLabelByItemIdx( idItem );
		if( idLabel != -1)
			lableTop.push_back( idLabel );		
		else
			cout << "invalid id of item " << idItem << endl;
	}
	cout << "items.size = " << items.size() << endl;

#if TIME_ENABLE
	sdkStopTimer( &timer[TIMER_faceSearch] );
	cout << "time of face search is: " << sdkGetTimerValue( &timer[TIMER_faceSearch] ) << endl;
#endif

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

	fprintf(file, "%d \n", labelSamples.size() );
	for ( int i=0; i<labelSamples.size(); i++ )
	{
		fwrite( (void*)&labelSamples[i], sizeof(int), 1, file );
	}

	fclose( file );

	mcv_verify_save_db( hIndex, "db.dat" );

	return true;
}

bool SenseTimeSDK::load( std::string fileImageFetures )
{

	if ( false == bInitialized )
		initialize();

	items.clear();
#if 1
	FILE *file = fopen( fileImageFetures.c_str(), "rb" );

	int nSize = 0;

	fscanf(file, "%d \n", &nSize );
	for ( int i=0; i<nSize; i++ )
	{
		db_item item;
		fread( (void*)&item, sizeof(db_item), 1, file );
		items.push_back(item);
	}


	fscanf(file, "%d \n", &nSize );
	labelSamples.assign( nSize, -1 );
	for ( int i=0; i<labelSamples.size(); i++ )
	{
		fread( (void*)&labelSamples[i], sizeof(int), 1, file );
		//cout << "labelSamples[" << i << "]" << labelSamples[i] << endl;
	}

	fclose( file );

#endif
	
	if ( !items.empty() )
	{
		mcv_result_t ret = mcv_verify_search_build_index(vinst,
			&items[0], items.size(), &hIndex);

		assert(hIndex != 0 && ret == MCV_OK);
	}
	//mcv_verify_load_db( hIndex, "db.dat" );

	cout << "load items " << items.size() << endl;

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
		cout << "opening " << FILE_DATABASE_ITEMS << endl;
		return load( FILE_DATABASE_ITEMS );
	}
	else
	{
#if AUTO_TRAIN
		if ( !train( FILE_LIST_NAME ) )
			return false;

		return save( FILE_DATABASE_ITEMS );	
#else
		cout << "create " << FILE_DATABASE_ITEMS << endl;
		file = fopen( FILE_DATABASE_ITEMS, "wb" );
		int nSize = 0;
		fprintf(file, "%d \n", nSize );
		fprintf(file, "%d \n", nSize );
		fclose( file );

		load( FILE_DATABASE_ITEMS );

		return false;
#endif
	}
}


bool SenseTimeSDK::faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, vector<cv::Mat>& matimg)
{
#if TIME_ENABLE
	sdkResetTimer( &timer[TIMER_faceDetect] );
	sdkStartTimer( &timer[TIMER_faceDetect]  );
#endif

	cv::Mat gray;
	cvtColor( imgIn, gray, CV_BGR2GRAY );
	cv::Mat *img = &gray;

	// detect
	mcv_facesdk_frontal_detector(hDetect,img->data,img->cols,img->rows,img->cols,&pface,&countFace);

	// draw result
	for ( int i=0;i<countFace;i++){

		float areaFace = (pface[i].Rect.right - pface[i].Rect.left) * 
			(pface[i].Rect.bottom - pface[i].Rect.top) ;
		
		float areaImage = imgIn.rows * imgIn.cols ;
		
		if( areaFace/areaImage < fRatioThreshold*0.01 )
			continue;

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

#if TIME_ENABLE
	sdkStopTimer( &timer[TIMER_faceDetect] );
	cout << "time of face detection is: " << sdkGetTimerValue( &timer[TIMER_faceDetect] ) << endl;
#endif

	return true;
}

bool SenseTimeSDK::prepareSamples( std::string filelist, vector<cv::Mat>& samples, vector<int>& labels,
	int	nIDMember, int nCountEach )
{
	cout << "prepareSamples" << endl;

	cv::Mat imgIn;
	std::ostringstream	pathEach;

	for ( int j = 0; j < nCountEach; j++ )
	{
		int id = (nIDMember-1) * 30 + j;

		pathEach.str("");
		pathEach << filelist << id << ".bmp";

		cout << pathEach.str() << endl;

		imgIn = cv::imread( pathEach.str() );

		samples.push_back( imgIn );
		labels.push_back( nIDMember );
	}


	return true;
}

bool SenseTimeSDK::train( vector<cv::Mat>& samples, vector<int>& labels, bool bForce )
{
#if TIME_ENABLE
	sdkResetTimer( &timer[TIMER_faceTrain] );
	sdkStartTimer( &timer[TIMER_faceTrain]  );
#endif

	cout << "bTrain = " << bTrain << endl;

	if (bTrain && !bForce)
		return true;

	labelSamples.clear();

	items.clear();

	labelSamples.assign( labels.size(), 0 );


	for ( int i = 0; i < samples.size(); i++ )
	{

		labelSamples[i] = labels[i];

		cout << "labelSamples[" << i << "] = " << labelSamples[i] << endl;

		fprintf(stderr, "Training %d\n", i);

		db_item item = getFeature( samples[i] );

		if( item.idx != -1 )
		{
			items.push_back(item);

			cout << "new item.idx = " << item.idx << endl;
		}
		else
			cout << "failed to getFeature " << i << endl;
	}

	if(items.size() == 0){
		fprintf(stderr, "No faces\n");
	}
	else
		cout << "items.size = " << items.size() << endl;

	if( ! items.empty() )
	{
		mcv_result_t ret = mcv_verify_search_build_index(vinst,
			&items[0], items.size(), &hIndex);

		assert(hIndex != 0 && ret == MCV_OK);
	}

	bTrain = true;

	save( FILE_DATABASE_ITEMS );

#if TIME_ENABLE
	sdkStopTimer( &timer[TIMER_faceTrain] );
	cout << "time to train " << samples.size() << " faces is: " << sdkGetTimerValue( &timer[TIMER_faceTrain] ) << endl;
#endif

	return true;
}

db_item SenseTimeSDK::getFeature( cv::Mat& imageIn )
{
	cv::Mat gray, imgInBGRA;

	//cout << "image cols = " << imageIn.cols << ", rows = " << imageIn.rows << endl;

	if ( imageIn.type() == CV_8UC3 )
	{
		cvtColor( imageIn, gray, CV_BGR2GRAY );
	}
	else
		gray = imageIn;

	//cout << "image cols = " << gray.cols << ", rows = " << gray.rows << endl;

	if ( bForceGray )
	{
		cvtColor( gray, imgInBGRA, CV_GRAY2BGRA );
	} 
	else
	{
		cvtColor( imageIn, imgInBGRA, CV_BGR2BGRA );
	}

	db_item item;
	item.idx = -1;

	//if(fcount)
	{
		mcv_rect_t rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = imageIn.cols;
		rect.bottom = imageIn.rows;
		memset(&item, 0, sizeof(item));
		mcv_result_t ret = mcv_verify_search_get_feature(vinst, imgInBGRA.data, imgInBGRA.cols,
			imgInBGRA.rows,  rect, &item);
		assert(ret == MCV_OK);
		item.idx = items.size();
	}
	

	return item;
}

cv::Mat* SenseTimeSDK::getImage( int nID, bool bLabel )
{
	cv::Mat* pImg = NULL;

	cout << "calling getImage " << nID << endl;

	if( bLabel )
	{
		if ( nID < 0 || nID > imageShow.size() )
		{
			cout << "nID " << nID << " not in (0, " << imageShow.size()-1 << endl;
			return NULL;
		}
		pImg = imageShow[ nID ];
		cout << "getImage " << nID << " of " << imageShow.size() << endl;
	}
	
	return	pImg;
}

bool SenseTimeSDK::trainAdd( vector<cv::Mat>& samples, vector<int>& labels )
{
#if TIME_ENABLE
	sdkResetTimer( &timer[TIMER_faceTrain] );
	sdkStartTimer( &timer[TIMER_faceTrain]  );
#endif

	cv::Mat* pImgIn;

	int nSizeOld = items.size();
	for ( int i = 0; i < samples.size(); i++ )
	{

		labelSamples.push_back( labels[i] );

		cout << "labels[" << i+nSizeOld << "] = " << labels[i] << endl;

		fprintf(stderr, "Training %d\n", i+nSizeOld);

		db_item item = getFeature( samples[i] );

		if( item.idx != -1 )
		{
			items.push_back(item);
			cout << "new item.idx = " << item.idx << endl;
		}
	}

	if(items.size() == 0){
		fprintf(stderr, "No faces\n");
	}
	else
		cout << "items.size = " << items.size() << endl;

	if( ! items.empty() )
	{
		mcv_result_t ret = mcv_verify_search_build_index(vinst,
			&items[0], items.size(), &hIndex);

		assert(hIndex != 0 && ret == MCV_OK);
	}

	bTrain = true;

	save( FILE_DATABASE_ITEMS );

#if TIME_ENABLE
	sdkStopTimer( &timer[TIMER_faceTrain] );
	cout << "time of face detection is: " << sdkGetTimerValue( &timer[TIMER_faceTrain] ) << endl;
#endif

	return true;
}

int SenseTimeSDK::findLabelByItemIdx( int idx )
{
	bool bFound = false;
	
	int i=0;
	for (i=0;i<items.size();i++)
	{
		if( items[i].idx == idx )
		{
			bFound = true;
			break;
		}
	}

	if( bFound)
		return labelSamples[i];
	else
		return -1;
}

void SenseTimeSDK::setScoreLine( int score )
{
	nScoreLine = score;
}

void SenseTimeSDK::setRatioThreshold( float ratio )
{
	fRatioThreshold = ratio;
}
