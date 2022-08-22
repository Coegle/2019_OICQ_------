#pragma once
#include <string>

/*		***  Text packet format ***
		Type				1 byte
		Message Class		<var>

		*** Login/Regist packet format ***
		Type				1 byte
		Length of Username	1 byte
		Length of Password	1 byte
		Username			<var>
		Password			<var>

		*** state packet format ***
		Type				1 byte
		State				1 byte
		Length of username	1 byte
		Username			<var>

		*** control packet format ***
		Type				1 byte
*/

#include <afxsock.h>
#include "CLoginDlg.h"
#include "ClistDlg.h"
#include "resource.h"
class SocketC :
public CSocket
{

public:
	SocketC();

	virtual ~SocketC();

	void CloseSocket();

	//����һ����Ϣ
	void OnReceive(int errcode);

	//�����ı���Ϣ
	void SendTextData(Mesg mesg);

	//���Ϳ�����Ϣ
	void SendControlMessage(CString username, CString pass, int type);		

	const static unsigned int length = PACKET_SIZE * 2;

	CLoginDlg* logindlg;
	
	CListDlg* listdlg;
}; 

