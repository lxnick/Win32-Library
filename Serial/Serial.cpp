#include "stdafx.h"
#include "Serial.h"

CSerial::CSerial()
{
	memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
 	memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );
	m_hComm = NULL;
	m_bOpened = FALSE;
}

CSerial::~CSerial()
{
	Close();
}

bool CSerial::Open( int nPort, int nBaud )
{
	if( m_bOpened ) 
		return true;

	char szPort[15];
	wsprintf( szPort, "COM%d", nPort );

	return Open(szPort, nBaud, 8, ONESTOPBIT, NOPARITY);
}

bool CSerial::Open(char* szName, DWORD nBaud, BYTE bytesize, BYTE parity, BYTE stopbits)
{
	if (m_bOpened) 
		return true;

	char szPort[15];
	wsprintf(szPort, "\\\\.\\%s", szName);

	m_hComm = CreateFile(szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (m_hComm == NULL)
		return false;

	memset(&m_OverlappedRead, 0, sizeof(OVERLAPPED));
	memset(&m_OverlappedWrite, 0, sizeof(OVERLAPPED));

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
	CommTimeOuts.ReadTotalTimeoutConstant = 0;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
	CommTimeOuts.WriteTotalTimeoutConstant = 5000;
	SetCommTimeouts(m_hComm, &CommTimeOuts);

	m_OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	DCB dcb;
	dcb.DCBlength = sizeof(DCB);
	GetCommState(m_hComm, &dcb);
	dcb.BaudRate = nBaud;
	dcb.ByteSize = bytesize;
	unsigned char ucSet;
	ucSet = (unsigned char)((FC_RTSCTS & FC_DTRDSR) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_RTSCTS) != 0);
	ucSet = (unsigned char)((FC_RTSCTS & FC_XONXOFF) != 0);

	if (!SetCommState(m_hComm, &dcb) ||
		!SetupComm(m_hComm, 10000, 10000) ||
		m_OverlappedRead.hEvent == NULL ||
		m_OverlappedWrite.hEvent == NULL) 
	{
		DWORD dwError = GetLastError();
		if (m_OverlappedRead.hEvent != NULL) 
			CloseHandle(m_OverlappedRead.hEvent);
		if (m_OverlappedWrite.hEvent != NULL) 
			CloseHandle(m_OverlappedWrite.hEvent);
		CloseHandle(m_hComm);
		return(FALSE);
	}

	m_bOpened = true;

	return(m_bOpened);
}

bool CSerial::Close( void )
{
	if( !m_bOpened || m_hComm == NULL ) 
		return( TRUE );
	if( m_OverlappedRead.hEvent != NULL ) 
		CloseHandle( m_OverlappedRead.hEvent );
	if( m_OverlappedWrite.hEvent != NULL ) 
		CloseHandle( m_OverlappedWrite.hEvent );
	CloseHandle(m_hComm);
	m_bOpened = true;
	m_hComm = NULL;

	return true;
}

bool CSerial::WriteCommByte( unsigned char ucByte )
{
	BOOL bWriteStat;
	DWORD dwBytesWritten;

	bWriteStat = WriteFile(m_hComm, (LPSTR) &ucByte, 1, &dwBytesWritten, &m_OverlappedWrite );
	if( !bWriteStat && ( GetLastError() == ERROR_IO_PENDING ) )
	{
		if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) 
			dwBytesWritten = 0;
		else
		{
			GetOverlappedResult(m_hComm, &m_OverlappedWrite, &dwBytesWritten, FALSE );
			m_OverlappedWrite.Offset += dwBytesWritten;
		}
	}

	return  true;
}

int CSerial::SendData( const char *buffer, int size )
{
	if( !m_bOpened || m_hComm == NULL ) 
		return( 0 );

	DWORD dwBytesWritten = 0;
	int i;
	for( i=0; i<size; i++ )
	{
		WriteCommByte( buffer[i] );
		dwBytesWritten++;
	}

	return( (int) dwBytesWritten );
}


int CSerial::ReadData( void *buffer, int limit )
{
	if( !m_bOpened || m_hComm == NULL ) 
		return( 0 );

	BOOL bReadStatus;
	DWORD dwBytesRead, dwErrorFlags;
	COMSTAT ComStat;

	ClearCommError(m_hComm, &dwErrorFlags, &ComStat );
	if( !ComStat.cbInQue ) return( 0 );

	dwBytesRead = (DWORD) ComStat.cbInQue;
	if( limit < (int) dwBytesRead ) dwBytesRead = (DWORD) limit;

	bReadStatus = ReadFile(m_hComm, buffer, dwBytesRead, &dwBytesRead, &m_OverlappedRead );
	if( !bReadStatus )
	{
		if( GetLastError() == ERROR_IO_PENDING )
		{
			WaitForSingleObject( m_OverlappedRead.hEvent, 2000 );
			return( (int) dwBytesRead );
		}
		return( 0 );
	}

	return( (int) dwBytesRead );
}




bool TryOpenComm(char* pszPort)
{
	char szPortName[256];
	snprintf(szPortName, sizeof(szPortName), "\\\\.\\%s", pszPort);

	HANDLE hComm = CreateFile(szPortName,		// Name of the Port to be Opened
		GENERIC_READ | GENERIC_WRITE,	// Read/Write Access
		0,								// No Sharing, ports cant be shared
		NULL,							// No Security
		OPEN_EXISTING,					// Open existing port only
		0,								// Non Overlapped I/O
		NULL);							// Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE)
		return FALSE;

	CloseHandle(hComm);
	return TRUE;
}


std::list<std::string> GetPortnameList()
{
	char name[15];
	std::list<std::string> name_list;

	for (int i = 1; i < 256; ++i)
	{
		snprintf(name, sizeof(name), "COM%d", i);

		if (TryOpenComm(name))
			name_list.push_back(name);
	}

	return name_list;
}

