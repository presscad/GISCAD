#include "StdAfx.h"

#ifdef _INTERNAL_
	#include <time.h>
	#include  <io.h>
#endif
#include "atlconv.h"
#include "math.h"
#include "NSBaseValidator.h"
#include "..\..\Include\NSReportWriter.h"
#include "..\..\Include\NSValidator.h"
#include "..\..\Include\NSRelationLinker.h"
#include "NSAssetValidation.h"
#include "NSLayerTbl.h"
#include "NSTextStyleTbl.h"
#include "NSLineTypeTbl.h"
#include "NSDimStyleTbl.h"
#include "NSTitleBlockTbl.h"
#include "NSTextHeightTbl.h"
#include "NSLayerBlockValidator.h"
#include "NSExtraValidator.h"
#include "NSErrorCircle.h"
#include "NSErrorParser.h"
#include <ctime>
extern CNSErrorCircle gErrorCircle;
NSSTRING strDWG$ = _T(""); 
NSSTRING strdwg$ = _T(""); 
double gTolerance = 1;
int LinearConnOnLayerFlag = 1;
int CNSBaseValidator::m_nP1ErrorFlag;
int CNSBaseValidator::m_nP2ErrorFlag;
int CNSBaseValidator::m_nP3ErrorFlag;
int CNSBaseValidator::m_nP4ErrorFlag;
//CNSReportWriter ReportObj;
CNSBaseValidator::CNSBaseValidator(void)
{
	m_dSearchAreaFactor = 1.0;
	m_dSearchAreaRadius = 1.0;
	m_dSymbolFactor = 5.0;
	m_bAssetValidation = false;
	m_szStdMDBLocation = NULL;
	
	NewFeatCnt = 0;
	ModFeatCnt = 0;
	DelFeatCnt = 0;
}

CNSBaseValidator::~CNSBaseValidator(void)
{
    delete [] m_strDWGFilename;
    delete [] m_strStdDWGFilename;
    delete [] m_strReportPath;
	delete [] m_szMDBLocation;
	delete [] m_szStdMDBLocation;
}
/*
@brief		Copies the user selected DWGFilename in the member variable
@Param(in)	The DWGFilename that is selected by the user
*/
void CNSBaseValidator::setDWGFilePath(TCHAR* strDWGFileName)
{
	int nLen = (int)NSSTRLEN(strDWGFileName);
	m_strDWGFilename = new TCHAR[nLen+1];

    #ifndef _INTERNAL_
	    NSSTRCPY(m_strDWGFilename,nLen+1, strDWGFileName);
    #else
	    NSSTRCPY(m_strDWGFilename,nLen+1, strDWGFileName);
        //NSSTRCPY(m_strDWGFilename , strDWGFileName);
    #endif

	strDWG$ = strDWGFileName;

	int nSlash = strDWG$.find_last_of('\\');

	strdwg$ = strDWG$.substr(nSlash+1,strDWG$.length()-nSlash);

    //int nSlash = strDWG$.ReverseFind('\\');	
	//strDWG$ = strDWG$.Mid(nSlash + 1, (strDWG$.GetLength() - nSlash));

	

	//m_strDWGFilename = strDWGFileName;
//	MessageBox(NULL,m_strDWGFilename.c_str(),_T("m_strDWGFilename"),MB_OK);
}
/*
@brief		Copies the standard DWGFilename from config file in the member variable
@Param(in)	The DWGFilename whose path is present in the config file.
*/
void CNSBaseValidator::setStdDWGFilePath(TCHAR* strStdDWGFileName)
{
	int nLen = (int)NSSTRLEN(strStdDWGFileName);
	m_strStdDWGFilename = new TCHAR[nLen+1];
    #ifndef _INTERNAL_
	    NSSTRCPY(m_strStdDWGFilename,nLen+1, strStdDWGFileName);
    #else
	    NSSTRCPY(m_strStdDWGFilename,nLen+1, strStdDWGFileName);
        //NSSTRCPY(m_strStdDWGFilename , strStdDWGFileName);
    #endif

	//m_strStdDWGFilename = strStdDWGFileName;
	//NSSTRCPY(m_strStdDWGFilename , strStdDWGFileName.c_str());
	//MessageBox(NULL,m_strStdDWGFilename.c_str(),_T("m_strStdDWGFilename"),MB_OK);
}
/*
@brief		Copies the report path in the member variable.
@Param(in)	The report path where the report is going to be stored.
*/
void CNSBaseValidator::setReportFilePath(TCHAR* strReportPath)
{
	//m_strReportPath = strReportPath;
	int nLen = (int)NSSTRLEN(strReportPath);
	m_strReportPath = new TCHAR[_MAX_PATH + 1];
    #ifndef _INTERNAL_
	    NSSTRCPY(m_strReportPath ,nLen+1, strReportPath);
    #else
		NSSTRCPY(m_strReportPath ,nLen+1, strReportPath);
        //NSSTRCPY(m_strReportPath , strReportPath);
    #endif
}

void CNSBaseValidator::setDatabase(CNSDWGMgr* pDWGMgr)
{
	localDWGObj = *pDWGMgr;
}
/*
@brief		Opens the drawing file
@Param(in)  The name of the file and the object of the class CNSDWGMgr
@Return		@Return		int				0-NS_SUCCESS, 1-NS_FAIL
*/
int CNSBaseValidator::openDrawing(NSSTRING strFileName, CNSDWGMgr &dwgMgr, bool bMakeCurrentActive)
{
	/*if(bMakeCurrentActive)
	{
		dwgMgr.setCurrentDatabaseActive();
		return NS_SUCCESS;
	}*/

	TCHAR szFormat[MAX_PATH + 1];
	if(NS_SUCCESS != dwgMgr.openDWG((TCHAR*)strFileName.c_str(), true))   //C:\GIDDocs\NS_DWG.dwg
	{
		NSSPRINTF(szFormat, _T("Error in opening drawing file:\'%s\'"), strFileName.c_str());
		WriteErrorRec(LOG_OTHER, szFormat);//WriteToLog(LOG_OTHER, szFormat);
		return NS_FAIL;
	}
	return NS_SUCCESS;
}

void CNSBaseValidator::setGraphicIndication(int nGraphicIndication)
{
	gErrorCircle.setGraphicIndication(nGraphicIndication);
}

/*
@brief		Calls the different function for validating different properties of dwg file
@Return		@Return		int				0-NS_SUCCESS, 1-NS_FAIL
*/
int CNSBaseValidator::validate(void)
{
	::CoInitialize(NULL);
	CNSReportWriter* pReportWriter = NULL;
	pReportWriter = CNSReportWriter::getReportWriterInstance();
	TCHAR szTime[_MAX_PATH + 1];
	time_t rawtime;
	tm * timeinfo;
	char buffer [80];
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
		
	strftime (buffer,80,"%d/%m/%Y",timeinfo);
	USES_CONVERSION;
	TCHAR* szDate = A2T(buffer);
		
	NSTIME(szTime);
	TCHAR szFormat[_MAX_PATH + 1];

	NSSTRING strDwgVersion = localDWGObj.getDWGVersionInfo(m_strDWGFilename);
	NSSPRINTF(szFormat, _T("AutoCAD File Format : %s"), strDwgVersion.c_str());
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);

	//NSSPRINTF(szFormat, _T("Validated Drawing: %s "), m_strDWGFilename.c_str());
	NSSPRINTF(szFormat, _T("Validated Drawing: %s "), m_strDWGFilename);
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);
	
	//NSSPRINTF(szFormat, _T("Standard Drawing: %s "), m_strStdDWGFilename.c_str());
	NSSPRINTF(szFormat, _T("Standard Drawing: %s "), m_strStdDWGFilename);	
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);

    //NSSPRINTF(szFormat, _T("Standard Drawing Version:1.0"));
    //WriteToLog(LOG_HEADER, szFormat);

	CNSValidator utilsValidator;
	utilsValidator.createDSN(m_szMDBLocation);
	CNSDWGMgr  stdDWGObj;
	
	checkReportPath();
#ifndef _INTERNAL_
	//Open current drawing file
	if(NS_SUCCESS != openDrawing(m_strDWGFilename, localDWGObj))
	{
		//Copy the name in the member variable
		m_strReportFileName = _T("ErrorReport.html");
		NSSTRING strReportPath = m_strReportPath;
		strReportPath.append(m_strReportFileName);

		//Create the report and write the different logs in it
		CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)strReportPath.c_str());
        delete pReportWriter;
	    pReportWriter = NULL;
	        ::CoUninitialize();
		return NS_FAIL;
	}
#endif
	//Open standard drawing file
	if(NS_SUCCESS != openDrawing(m_strStdDWGFilename, stdDWGObj))
	{
		//Copy the name in the member variable
		m_strReportFileName = _T("ErrorReport.html");
		NSSTRING strReportPath = m_strReportPath;
		strReportPath.append(m_strReportFileName);

		//Create the report and write the different logs in it
		CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)strReportPath.c_str());
        delete pReportWriter;
	    pReportWriter = NULL;
	        ::CoUninitialize();
        #ifndef _INTERNAL_
	        localDWGObj.closeDatabase();
        #endif
		return NS_FAIL;
	}
	
    int nResult = localDWGObj.copyBlockFromDrawing(stdDWGObj, _T("ERR_CIRCLE"));

	if(NS_SUCCESS != CNSDatabaseMgr::getInstance()->openDatabase(_T(""), _T(""), _T("DSN=IE_ToolBox")))
	{
		WriteErrorRec(LOG_OTHER, _T("Could not establish database connection."));

		m_strReportFileName = _T("ErrorReport.html");
		NSSTRING strReportPath = m_strReportPath;
		strReportPath.append(m_strReportFileName);

		CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)strReportPath.c_str());
	    delete pReportWriter;
	    pReportWriter = NULL;
	        ::CoUninitialize();
	    //Close the database
        #ifndef _INTERNAL_
	        localDWGObj.closeDatabase();
        #endif
	    stdDWGObj.closeDatabase();
		return NS_FAIL;
	}
	
	NSSTRING strVersion = localDWGObj.getVersionInfo(stdDWGObj);
	NSSPRINTF(szFormat, _T("Standard Drawing Version: %s"), strVersion.c_str());
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);
	//NSSTRING strVersion = localDWGObj.getVersionInfo();
	//strVersion = localDWGObj.getDWGVersionInfo(m_strDWGFilename);
	//NSSPRINTF(szFormat, _T("Standard Drawing Version:%s"), strVersion.c_str());
	//WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);

	//Erase previous error circles drawn in the local dwg
	localDWGObj.eraseGISErrorLayer();
	gErrorCircle.clearErrMap();
	gErrorCircle.setLocalDWGObject(&localDWGObj);

//**//SUBIR  - 10.02.2010 <<MAIL SEARCH STRING = "Invalid drawing unable to continue validation" "Extract ID">>

//Validate Title block
CNSTitleBlock titleblockTbl;
titleblockTbl.setLocalDWGObject(&localDWGObj);
titleblockTbl.setStdDWGObject(&stdDWGObj);

if(NS_SUCCESS != titleblockTbl.validate())
{

 NSSTRING strMsg = _T("Unable to validate ") + (NSSTRING)m_strDWGFilename + _T("\n");

 strMsg = strMsg + _T("Drawing may not be a valid GIS extract") + _T("\n");

 strMsg = strMsg + _T("Refer error report for details.");

 MessageBox(NULL, strMsg.c_str(), _T("Invalid drawing"), MB_OK);


	WriteErrorRec(LOG_OTHER, _T("Invalid drawing ... Possible Reasons are as below <BR><BR> 1) TitleBlock name does not end with '_TITLE' <BR> 2) ExportID is blank in TitleBlock <BR> 3) Attribute values are not same in all the TitleBlocks "));

	m_strReportFileName = _T("ErrorReport.html");
	NSSTRING strReportPath = m_strReportPath;
	strReportPath.append(m_strReportFileName);

	CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)strReportPath.c_str());
	delete pReportWriter;
	pReportWriter = NULL;
		::CoUninitialize();
	//Close the database
	#ifndef _INTERNAL_
		localDWGObj.closeDatabase();
	#endif
	stdDWGObj.closeDatabase();
	return NS_FAIL;
}
//**//SUBIR  - 10.02.2010 <<MAIL SEARCH STRING = "Invalid drawing unable to continue validation" "Extract ID">>


//if(m_bGisValidation == true)
//{
	CNSExtraValidator extraValidations;
	extraValidations.setLocalDWGObject(&localDWGObj);
	extraValidations.setStdDWGObject(&stdDWGObj);
	extraValidations.validate(m_bCadValidation,m_bGisValidation);
//}

if(m_bCadValidation == true)
{
	//Validate Layers properties
	CNSLayerTbl layerTbl;
	layerTbl.setLocalDWGObject(&localDWGObj);
	layerTbl.setStdDWGObject(&stdDWGObj);
	layerTbl.validate();
}

if(m_bCadValidation == true)
{
	//Validate TextStyles properties             
	CNSTextStyleTbl textstyleTbl;                
	textstyleTbl.setLocalDWGObject(&localDWGObj);
	textstyleTbl.setStdDWGObject(&stdDWGObj);    
	textstyleTbl.validate();     
}

if(m_bCadValidation == true)
{
	//Validate LineType	properties                           
	CNSLineTypeTbl linetypeTbl;                  
	linetypeTbl.setLocalDWGObject(&localDWGObj); 
	linetypeTbl.setStdDWGObject(&stdDWGObj);     
	linetypeTbl.validate();  
}

if(m_bCadValidation == true)
{
	//Validate DimStyle properties 
	CNSDimStyleTbl dimstyleTbl;
	dimstyleTbl.setLocalDWGObject(&localDWGObj);
	dimstyleTbl.setStdDWGObject(&stdDWGObj);
	dimstyleTbl.validate();
}
	


if(m_bCadValidation == true)
{
	//Validate Text Height
	
	CNSTextHeightTbl textHtTbl;
	textHtTbl.setLocalDWGObject(&localDWGObj);
	textHtTbl.setStdDWGObject(&stdDWGObj);
	textHtTbl.validate();
}

if(m_bGisValidation == true)
{
	//Validate layer block features	
	CNSLayerBlockValidator layerBlkValidator;
	layerBlkValidator.setLocalDWGObject(&localDWGObj);
	layerBlkValidator.setStdDWGObject(&stdDWGObj);
	layerBlkValidator.setSearchAreaFactor(m_dSearchAreaFactor);
	layerBlkValidator.validate(NewFeatCnt,ModFeatCnt,DelFeatCnt);
}

if(m_bGisValidation == true && m_nGisFlag == 1)
{
	CNSRelationLinker objLinker;
	objLinker.setLocalDWGObject(&localDWGObj);
	objLinker.link(m_nRelFlag, m_dSearchAreaFactor, m_dSearchAreaRadius, m_dSymbolFactor, LinearConnOnLayerFlag);
}

//*******************************Commented on 4-Feb-2014, 2013 conversion, since no asset validation is performed************************************
	//if(m_bAssetValidation == true)
	//{
	//	CNSAssetValidation objAssetval;
	//	objAssetval.setLocalDWGObject(&localDWGObj);
	//	objAssetval.setUserCredentials(m_UserCredentials);
	//	int nResult = objAssetval.validate();
	//	if(nResult == NS_FAIL)
	//	{
	//		//WriteIDToLog(LOG_GIS,  _T("Failed in AssetValidation"), ERR_GIS_312);

	//		WriteErrorRec(LOG_GIS, ERR_GIS_312, _T("Failed in AssetValidation")); 
	//	}
	//		
	//}
//*******************************Commented on 4-Feb-2014, 2013 conversion, since no asset validation is performed************************************	

	int nGISLogs = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_GIS);
	int nCADLogs = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_CAD);

	int nGISLogs_P1 = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_GIS,_T("P1"));
	int nGISLogs_P2 = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_GIS,_T("P2"));
	int nCADLogs_P1 = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_CAD,_T("P1"));
	int nCADLogs_P2 = CNSReportWriter::getReportWriterInstance()->getNumberOfLogs(LOG_CAD,_T("P2"));

	int nGISLogs_P1nP2 = nGISLogs_P1 + nGISLogs_P2 ;
	int nCADLogs_P1nP2 = nCADLogs_P1 + nCADLogs_P2 ;

	std::map<NSSTRING,NSSTRING> mAttributes;
	if(nGISLogs_P1nP2 > 0 )
	{
		NSSPRINTF(szFormat, _T("GIS Compliance: No"));
		mAttributes.insert(STRING_MAP::value_type(_T("GIS_SUCCESS"),_T("0")));		
	}
	else
	{
		NSSPRINTF(szFormat, _T("GIS Compliance: Yes"));
		mAttributes.insert(STRING_MAP::value_type(_T("GIS_SUCCESS"),_T("1")));		
	}
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);

	if(nCADLogs > 0 )
	{
		NSSPRINTF(szFormat, _T("IE CAD Standards Compliance: No"));
		mAttributes.insert(STRING_MAP::value_type(_T("CAD_SUCCESS"),_T("0")));	
	}
	else
	{
		NSSPRINTF(szFormat, _T("IE CAD Standards Compliance: Yes"));
		mAttributes.insert(STRING_MAP::value_type(_T("CAD_SUCCESS"),_T("1")));		
	}
	
	WriteErrorRec(LOG_HEADER, szFormat);//WriteToLog(LOG_HEADER, szFormat);

	localDWGObj.setTitleBlockAttributes(mAttributes);

#ifdef _INTERNAL_
	//localDWGObj.saveActiveDWGAs2004();
	
#else
	localDWGObj.saveCurrentDWG((TCHAR*)m_strDWGFilename, false, strDwgVersion);
#endif
	//CNSValidator validator;
//	__time64_t time = validator.getFileLastModified((TCHAR*)m_strStdDWGFilename.c_str());

//	_wasctime_s(szFormat, _MAX_PATH, &time);
	

	//localDWGObj.saveCurrentDWG((TCHAR*)m_strDWGFilename, false, strDwgVersion);

	NSSPRINTF(szFormat, _T("Validation date: %s %s "), szDate ,szTime);
    
	WriteErrorRec(LOG_HEADER,szFormat);//WriteToLog(LOG_HEADER, szFormat);


	TCHAR szUserName[MAX_COMPUTERNAME_LENGTH + 1];  
    DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;  
	if( !GetUserName(szUserName, &dwBufferSize))
	{
		NSSTRCPY(szUserName, titleblockTbl.getUserName().c_str());
	}

	NSSPRINTF(szFormat, _T("Validated by: %s"), szUserName);
	
	WriteErrorRec(LOG_HEADER,szFormat);//WriteToLog(LOG_HEADER, szFormat);


	NSSPRINTF(szFormat, _T("Total Errors: %i"), nGISLogs + nCADLogs);
	
	WriteErrorRec(LOG_HEADER,szFormat);//WriteToLog(LOG_HEADER, szFormat);


	TCHAR szNewLine[_MAX_PATH+1];
	TCHAR szModLine[_MAX_PATH+1];
	TCHAR szDelLine[_MAX_PATH+1];
	#ifndef _INTERNAL_
		NSSPRINTF(szNewLine , _MAX_PATH+1,_T("Count of New Features       : %d\r\n"), NewFeatCnt);
		NSSPRINTF(szModLine , _MAX_PATH+1,_T("Count of Modified Features  : %d\r\n"), ModFeatCnt);
		NSSPRINTF(szDelLine , _MAX_PATH+1,_T("Count of Deleted Features   : %d\r\n"), DelFeatCnt);
	#else
		NSSPRINTF(szNewLine , _T("Count of New Features       :%d\r\n"), NewFeatCnt);
		NSSPRINTF(szModLine , _T("Count of Modified Features  :%d\r\n"), ModFeatCnt);
		NSSPRINTF(szDelLine , _T("Count of Deleted Features   :%d\r\n"), DelFeatCnt);
	#endif


	WriteErrorRec(LOG_HEADER,szNewLine);
	WriteErrorRec(LOG_HEADER,szModLine);
	WriteErrorRec(LOG_HEADER,szDelLine);

	//-- Show Validation Rule error messages status to Validation Report --
	NSSTRING errorRules;
	if(m_nP1ErrorFlag) 
	{
		errorRules.append(_T("P1 "));
	}
	if(m_nP2ErrorFlag)
	{
		errorRules.append(_T("P2 "));
	}
	if(m_nP3ErrorFlag)
	{
		errorRules.append(_T("P3 "));
	}
	if(m_nP4ErrorFlag)
	{
		errorRules.append(_T("P4"));
	}
	TCHAR szErrorRules[_MAX_PATH+1];
	NSSPRINTF(szErrorRules , _MAX_PATH+1,_T("Selected Error Messages: %s\r\n"), errorRules.c_str());
	WriteErrorRec(LOG_HEADER,szErrorRules);
	


	
	//m_strReportPath.append(titleblockTbl.getReportName().c_str());
	//NSSTRCPY(m_strReportPath titleblockTbl.getReportName().c_str());
	NSSTRING szReportFile = m_strReportPath;
//	szReportFile.append(_T("\\"));
	m_strReportFileName = titleblockTbl.getReportName();
	szReportFile.append(m_strReportFileName.c_str());
	//Create the report and write the different logs in it
	//CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)m_strReportPath.c_str());

	NSSTRING strCSSpath = _T("format.css");
	if(m_szStdMDBLocation != NULL )
	{
		strCSSpath = m_szStdMDBLocation;
		int nSlash = strCSSpath.find_last_of('\\');
		strCSSpath = strCSSpath.substr(0,nSlash);	
		strCSSpath.append(_T("\\format.css\""));
	}

	//strdwg$ = strDWG$.substr(nSlash+1,strDWG$.length()-nSlash);
	CNSReportWriter::getReportWriterInstance()->writeReport(REPORT_HTML, (TCHAR*)szReportFile.c_str(), (TCHAR*)strCSSpath.c_str());


	#ifdef _INTERNAL_ // forcefully given unicode _T() 15/11/2011
		NSSTRING szLOG;
		szLOG.append(_T("{\\L\\C2;Header Information}\n"));
		szLOG.append(CNSReportWriter::getReportWriterInstance()->getLogAtEx(LOG_HEADER));
		szLOG.append(_T("{\\L\\C2;CAD Errors}\n"));
		szLOG.append(CNSReportWriter::getReportWriterInstance()->getLogAtEx(LOG_CAD));
		szLOG.append(_T("{\\L\\C2;GIS Errors}\n"));
		szLOG.append(CNSReportWriter::getReportWriterInstance()->getLogAtEx(LOG_GIS));
		localDWGObj.WriteToLoglayout(szLOG);
	#endif
	
	//delete pReportWriter;
	pReportWriter->release();
    pReportWriter = NULL;
    CNSDatabaseMgr::getInstance()->closeDatabase();
	::CoUninitialize();

#ifndef _INTERNAL_
	localDWGObj.ReleaseHostDwg();
#endif
	//Close the database
	//localDWGObj.ReleaseHostDwg();
	stdDWGObj.ReleaseHostDwg();
	//Close the database
#ifndef _INTERNAL_
	localDWGObj.closeDatabase();
#endif
	//localDWGObj.closeDatabase();
	stdDWGObj.closeDatabase();
	return 0;
}

void CNSBaseValidator::getReportFilePath(NSSTRING &temp)
{
	TCHAR szReportData[_MAX_PATH+1];
	#ifndef _INTERNAL_
		NSSPRINTF(szReportData , _MAX_PATH+1,_T("Validated Drawing:\'%s\'\r\n"), m_strDWGFilename);
	#else
		NSSPRINTF(szReportData , _T("Validated Drawing:\'%s\'\r\n"), m_strDWGFilename);
	#endif


	temp.append(_T("------------------------------------------------------------------------------------------------------------------\r\n"));
	temp.append(szReportData);
	temp.append(_T("Report Location:\r\n"));
	temp.append(_T("file:"));
	//Replace the double slash in the path with single slash as URL's fails to open them in IE
 	NSSTRING strReportString = getTokenizedPath(m_strReportPath);
	temp.append(strReportString);
//	temp.append(_T("\"));
	temp.append(m_strReportFileName);
	temp.append(_T("\r\n"));
	temp.append(_T("------------------------------------------------------------------------------------------------------------------\r\n"));

	TCHAR szNewLine[_MAX_PATH+1];
	TCHAR szModLine[_MAX_PATH+1];
	TCHAR szDelLine[_MAX_PATH+1];
	#ifndef _INTERNAL_
		NSSPRINTF(szNewLine , _MAX_PATH+1,_T("Count of New Features       : %d\r\n"), NewFeatCnt);
		NSSPRINTF(szModLine , _MAX_PATH+1,_T("Count of Modified Features  : %d\r\n"), ModFeatCnt);
		NSSPRINTF(szDelLine , _MAX_PATH+1,_T("Count of Deleted Features   : %d\r\n"), DelFeatCnt);
	#else
		NSSPRINTF(szNewLine , _T("Count of New Feature		:%d\r\n"), NewFeatCnt);
		NSSPRINTF(szModLine , _T("Count of Modified Feature :%d\r\n"), ModFeatCnt);
		NSSPRINTF(szDelLine , _T("Count of Deleted Feature  :%d\r\n"), DelFeatCnt);
	#endif
	temp.append(szNewLine);
	temp.append(szModLine);
	temp.append(szDelLine);
	//return ((TCHAR*)strReportFullpath.c_str());
}

void CNSBaseValidator::setRelationshipFlag(int nRelFlag)
{
	m_nRelFlag = nRelFlag;
}

/*
@brief			Set connectivity flag. 
@Param(in)		nGisFlag:- Flag true=1 / False=0.
@Param(out)		
*/

void CNSBaseValidator::setConnctivityFlag(int nGisFlag)
{
	m_nGisFlag = nGisFlag;
}

void CNSBaseValidator::setSearchAreaFactor(double dSearchAreaFactor, double dSymbolFactor)
{
	m_dSearchAreaFactor = dSearchAreaFactor;
	m_dSymbolFactor = dSymbolFactor;
}

void CNSBaseValidator::setPrecision(int nPrecision)
{
	//gTolerance = 1.0 / pow(10.0 ,nPrecision);
}

void CNSBaseValidator::setSearchAreaRadius(double dSearchAreaRadius)
{
	m_dSearchAreaRadius = dSearchAreaRadius;
}

void CNSBaseValidator::setTolerance(double dTolerance)
{
	if(dTolerance == 0.0) dTolerance = 1.0;
	gTolerance = dTolerance;
}
void CNSBaseValidator::setLinearConnOnLayerFlag(int iFlag)
{	
	LinearConnOnLayerFlag = iFlag;
}

/*
@brief		Check for validity of report path
*/
void CNSBaseValidator::checkReportPath()
{
	//If report path is not proper in config file.
	int nretValue = NSACCESS(m_strReportPath, 0);
	TCHAR szFormat[_MAX_PATH +1];
	TCHAR app_path[_MAX_PATH + 1];
	if(nretValue == -1)
	{
		NSSPRINTF(szFormat, _T("Invalid report path \'%s\'. Report will be created in application path."),m_strReportPath);
		WriteErrorRec(LOG_OTHER, szFormat);//WriteToLog(LOG_CAD, szFormat);
		//m_strReportPath = _T("");
		//Get the application path and copy the report path in the member variable
	#ifndef _INTERNAL_
		GetModuleFileNameW(NULL, app_path, MAX_PATH + 1);
	#else
		GetModuleFileName(NULL, app_path, MAX_PATH + 1);
	#endif
		//NSSTRCPY(m_strReportPath, MAX_PATH + 1, app_path);
        NSSTRING strTempReportPath(app_path);
        size_t nIndex = strTempReportPath.find_last_of(_T("\\"));
        strTempReportPath = strTempReportPath.substr(0, nIndex + 1);
		#ifndef _INTERNAL_
			NSSTRCPY(m_strReportPath, _MAX_PATH + 1, strTempReportPath.c_str());
		#else
			NSSTRCPY(m_strReportPath, _MAX_PATH + 1, strTempReportPath.c_str());
			//NSSTRCPY(m_strReportPath, strTempReportPath.c_str());
		#endif
	}
	else
	{
        NSSTRING strApp(m_strReportPath);
        int nValue = (int)strApp.find_last_of(_T("\\"));
        if(nValue != strApp.length() - 1)
        {
            strApp.append(_T("\\"));
			#ifndef _INTERNAL_
				NSSTRCPY(m_strReportPath , MAX_PATH + 1,(TCHAR*)strApp.c_str());
			#else
				NSSTRCPY(m_strReportPath , MAX_PATH + 1,(TCHAR*)strApp.c_str());
				//NSSTRCPY(m_strReportPath , (TCHAR*)strApp.c_str());
			#endif
        }
	}
}

void CNSBaseValidator::setMDBLocation(TCHAR* szMDBLocation, bool bStandardMDB)
{
	int nLen = (int)NSSTRLEN(szMDBLocation);
	if(!bStandardMDB)
	{
		m_szMDBLocation = new TCHAR[_MAX_PATH + 1];
		#ifndef _INTERNAL_
			NSSTRCPY(m_szMDBLocation ,nLen+1, szMDBLocation);
		#else
			NSSTRCPY(m_szMDBLocation ,nLen+1, szMDBLocation);
			//NSSTRCPY(m_szMDBLocation , szMDBLocation);
		#endif
	}
	else
	{
		m_szStdMDBLocation = new TCHAR[_MAX_PATH + 1];
		#ifndef _INTERNAL_
			NSSTRCPY(m_szStdMDBLocation, nLen+1, szMDBLocation);
		#else
		    NSSTRCPY(m_szStdMDBLocation, nLen+1, szMDBLocation);
			//NSSTRCPY(m_szStdMDBLocation, szMDBLocation);
		#endif

	}
}

NSSTRING CNSBaseValidator::getTokenizedPath(TCHAR* pszInput)
{
	// Establish string and get the first token:
	TCHAR *pchToken = NULL;
	TCHAR *pchTempTok = NULL;
	TCHAR chSeps[] = _T("\\");
	NSSTRING strOutString;

    int nLen = (int)NSSTRLEN(pszInput);
	TCHAR* pTempPath = new TCHAR[nLen+1];

    #ifndef _INTERNAL_
	    NSSTRCPY(pTempPath,nLen+1, pszInput);
    #else
		NSSTRCPY(pTempPath,nLen+1, pszInput);
        //NSSTRCPY(pTempPath , pszInput);
    #endif

	#ifdef _INTERNAL_
		//pchToken = NSSTRTOK(pTempPath, chSeps); // C4996
		pchToken = NSSTRTOK(pTempPath, chSeps , &pchTempTok); // C4996
	#else
		pchToken = NSSTRTOK(pTempPath, chSeps , &pchTempTok); // C4996
	#endif

	// Note: strtok is deprecated; consider using strtok_s instead
	while( pchToken != NULL )
	{
		strOutString.append(pchToken);
		strOutString.append(_T("\\"));
		// Get next token: 
	#ifdef _INTERNAL_
		//pchToken = NSSTRTOK( NULL, chSeps); // C4996
		pchToken = NSSTRTOK( NULL, chSeps ,  &pchTempTok); // C4996
	#else
		pchToken = NSSTRTOK( NULL, chSeps ,  &pchTempTok); // C4996
	#endif
	}
    delete []pTempPath;
	return strOutString;
}

void CNSBaseValidator::setUserCredentials(CNSUserCredentials userCredentials)
{
	m_UserCredentials = userCredentials;
}

void CNSBaseValidator::setAssetValidationFlag(bool bFlag)
{
	m_bAssetValidation = bFlag;
}

void CNSBaseValidator::setCadValidationFlag(bool bFlag)
{
	m_bCadValidation = bFlag;
}

void CNSBaseValidator::setGisValidationFlag(bool bFlag)
{
	m_bGisValidation = bFlag;
}

/*
@brief			Set Validation Rule Error Meassages checkBox Status. 
@Param(in)		
@Param(out)		
*/

void CNSBaseValidator::setValidationRulesErrorFlag(int nFlagP1,int nFlagP2,int nFlagP3,int nFlagP4)
{
	m_nP1ErrorFlag = nFlagP1;
	m_nP2ErrorFlag = nFlagP2;
	m_nP3ErrorFlag = nFlagP3;
	m_nP4ErrorFlag = nFlagP4;

}
