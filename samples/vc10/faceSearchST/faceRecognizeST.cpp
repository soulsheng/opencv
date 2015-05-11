
#include "faceRecognizeST.h"

#include <fstream>

#define	AUTO_TRAIN		0



#define FILE_RESULT_NAME	"out.txt"
#define FILE_DATABASE_ITEMS	"st_items.bin"
#define FILE_DATABASE_NAMES	"st_names.bin"

#define DATA_FILE_COUNT	4
char  dataFileList[][50]={
	"data/attribute.pack",
	"data/model_verif.pack",
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

SenseTimeSDK::SenseTimeSDK(int nInstanceCount)
{
	hIndex = NULL;
	bTrain = false;
	bInitialized = false;
	bReleased = false;

	db_id = 0;
	bForceGray = false;
	nScoreLine = 0;
	fRatioThreshold = 0.25f;	// 0.25% = 0.0025
	s = 2.0f;
	npoint = 21;
	angleThreshold = 15;

	points = new mcv_pointf_t[npoint];

	m_nInstance.assign( nInstanceCount, -1 );
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

	delete[]	points;

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
	
	for (int i=0; i< m_nInstance.size(); i++)
	{
		mcv_handle_t hDetect = mcv_facesdk_create_frontal_detector_instance_from_resource(2);
		if ( NULL == hDetect)
		{
			printf( "failed to create detector \n" );
			return false;
		}
		m_nInstance[i] = hDetect;
	}

	if ( !checkDataFile() )
		return false;

	vinst = mcv_create_verify_instance();
	if ( NULL == vinst)
	{
		printf( "failed to create verify \n" );
		return false;
	}

	hAlignmentor = mcv_facesdk_create_LRAlignmentor_instance_from_resource(npoint);

	bInitialized = true;


	if( !checkTrained() )
	{
		printf( "failed to train \n" );
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

	if(hIndex) mcv_verify_search_release_index(hIndex);
	for (int i=0; i< m_nInstance.size(); i++)
		if( m_nInstance[i] ) mcv_facesdk_destroy_frontal_instance( m_nInstance[i] );

	if(vinst) mcv_verify_release_instance(vinst);
	if(hAlignmentor) mcv_facesdk_destroy_LRAlignmentor_instance(hAlignmentor);

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
		cout<<nScoreLine<<endl;
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
		cout<<idLabel<<endl;
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

	cout << "load p 1" << endl;

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
		cout << "cannot open " << FILE_DATABASE_ITEMS << endl;
		return false;
#endif
	}
}

void SenseTimeSDK::scale( mcv_rect_t& rect, float s, cv::Mat& imgIn )
{
	float left, right, top, bottom;
	float xCenter, yCenter, width, height;

	left  = rect.left ; 
	right = rect.right ;
	top  = rect.top ;
	bottom = rect.bottom ;

	width	= right - left;
	height	= bottom - top;
	xCenter = left	+ width*0.5f ;
	yCenter = top	+ height*0.5f ;

	width	*= s;
	height	*= s;

	left	= xCenter - width*0.5f ;
	right	= xCenter + width*0.5f ;
	top		= yCenter - height*0.5f ;
	bottom	= yCenter + height*0.5f ;

	if( left < 0 ) left = 0;
	if( right > imgIn.cols ) right = imgIn.cols;
	if( top < 0 ) top = 0;
	if( bottom > imgIn.rows ) bottom = imgIn.rows;

	cout << "image(col, row)" << imgIn.cols << ", " << imgIn.rows << endl;

	cout << "Rect(l,r,t,b) = " 
		<< left	<< ", " << right << ", " 
		<< top	<< ", " << bottom << endl;

	rect.left = int(left + 0.5f);
	rect.right = int(right + 0.5f);
	rect.top = int(top + 0.5f);
	rect.bottom = int(bottom + 0.5f);

}

bool SenseTimeSDK::faceDetect(cv::Mat& imgIn, cv::Mat& imgOut, vector<cv::Mat>& matimg, int nInstance)
{
#if TIME_ENABLE
	sdkResetTimer( &timer[TIMER_faceDetect] );
	sdkStartTimer( &timer[TIMER_faceDetect]  );
#endif

	cv::Mat gray;
	cvtColor( imgIn, gray, CV_BGR2GRAY );
	cv::Mat *img = &gray;

	PMCV_FACERECT pface=NULL ;
	unsigned int countFace=0 ;

	// detect
	mcv_facesdk_frontal_detector(m_nInstance[nInstance],img->data,img->cols,img->rows,img->cols,&pface,&countFace);

	float left, right, top, bottom;

	// draw result
	for ( int i=0;i<countFace;i++){

		scale( pface[i].Rect, s, imgIn );

		left  = pface[i].Rect.left ; 
		right = pface[i].Rect.right ;
		top  = pface[i].Rect.top ;
		bottom = pface[i].Rect.bottom ;

		float areaFace = (right - left) * (bottom - top) ;

		float areaImage = imgIn.rows * imgIn.cols ;
		
		if( areaFace/areaImage < fRatioThreshold*0.01 )
			continue;

		if ( !testFrontal(imgIn, pface[i].Rect ))
			continue;

		rectangle( imgOut, 
			cvPoint( left, top ),
			cvPoint( right, bottom ) ,
			CV_RGB(0,0,255), 3, 8, 0);

		cv::Mat imgROI( imgIn, cv::Rect(left, top ,right - left, bottom - top ));
		//printf("width = %d \n", pface[i].Rect.right - pface[i].Rect.left);
		matimg.push_back(imgROI);

	}

	if(countFace) mcv_facesdk_release_frontal_result(pface,countFace);

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

bool SenseTimeSDK::prepareSamples( std::string fileList, vector<cv::Mat>& samples, vector<int>& labels )
{
	string filePath, fileName;
	vector<string> fileNames;
	ifstream	inputFile(fileList);
	inputFile >> filePath;
	
	while( !inputFile.eof() )
	{
		inputFile >> fileName;
		fileNames.push_back( filePath + fileName);
	}

	for (int i = 0;i<fileNames.size();i++)
	{
		cv::Mat imgIn = cv::imread( fileNames[i] );
		samples.push_back( imgIn );
		labels.push_back( i );
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

#if 0
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

#endif

	cvtColor( imageIn, imgInBGRA, CV_BGR2BGRA );


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

bool SenseTimeSDK::testFrontal( cv::Mat& mat, mcv_rect_t& rect )
{
	
	mcv_facesdk_LRAlign(
		hAlignmentor,
		mat.data,
		mat.cols,
		mat.rows,
		mat.step,
		rect,
		MCV_StrictFrontal,
		npoint,
		points
		);

	float angleYaw, anglePitch, angleRoll, dist;
	mcv_facesdk_get_pose(
		points, npoint, &angleYaw, &anglePitch, &angleRoll, &dist );

	if (  abs(angleYaw) < angleThreshold 
		&&abs(anglePitch) < angleThreshold 
		&&abs(angleRoll) < angleThreshold )
		return true;
	else
		return false;
	
}

void SenseTimeSDK::setAngleMax( float angle )
{
	angleThreshold  = angle;
}
