#ifndef WIN_SERIAL_HEADER
#define WIN_SERIAL_HEADER

#include <Windows.h>

#include <list>
#include <string>

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

class CSerial
{

public:
	CSerial();
	~CSerial();

	bool Open(int nPort = 2, int nBaud = 9600 );
	bool Open(char* zzName, DWORD baud, BYTE bytesize, BYTE parity, BYTE stopbits);
	bool Close( void );

	int ReadData( void *, int );
	int SendData( const char *, int );

	bool IsOpened( void ){ return( m_bOpened ); }

protected:
	bool m_bOpened;
	bool WriteCommByte( unsigned char );

	HANDLE m_hComm;
	OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
};

std::list<std::string> GetPortnameList();

#endif
