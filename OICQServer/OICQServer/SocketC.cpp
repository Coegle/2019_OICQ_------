
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
		Length of username	1 byte
		Username			<var>

		*** control packet format ***
		Type				1 byte
*/


#include "pch.h"

#include "SocketC.h"
#include "OICQServer.h"
#include "OICQServerDlg.h"
#include <fstream>
#include "User_Mesg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



SocketC::SocketC(COICQServerDlg *dlg)
{
	pdlg = dlg;
}

SocketC::~SocketC()
{

}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(SocketC, CSocket)
	//{{AFX_MSG_MAP(SocketC)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////



//�κ����ݵ���ʱ�����ô˺���	
void SocketC::OnReceive(int errcode)
{
	wchar_t data[PACKET_SIZE] = { 0 };
	wchar_t name[USER_SIZE] = { 0 };
	wchar_t pass[PASS_SIZE] = { 0 };
	unsigned int retvalue;
	User tmp;
	int len1, len2;
	std::map<CString, User>::iterator it_registed_user;
	std::map<CString, SocketC*>::iterator it_msgsock;
	std::multimap<CString, Mesg>::iterator it_unsend_msg;
	Mesg mesg;

	retvalue = this->Receive(data, length);
	if (retvalue == SOCKET_ERROR)return;

	switch (data[0]) {
	case TYPE_LOGIN://						��¼
		len1 = data[1];
		len2 = data[2];

		memcpy(name, &data[3], len1 * 2);
		memcpy(pass, &data[3 + len1], len2 * 2);

		tmp.password = pass;
		tmp.username = name;

		it_registed_user = theApp.Registed_User.find(tmp.username);
		if (it_registed_user != theApp.Registed_User.end() && it_registed_user->second.password == tmp.password) { //��¼�ɹ�

			//��ͻ��˷��͵�¼�ɹ���Ϣ
			this->SendControlMessage(TYPE_SUCCEED);

			//�ڽ����и����û�������Ϣ
			pdlg->list_online.InsertString(theApp.online, tmp.username);
			theApp.online++;

			//�ں�̨�����û�������Ϣ
			theApp.Registed_User[tmp.username].alive = true;
			
			it_msgsock = theApp.msgsock.find(name);
			if (it_msgsock != theApp.msgsock.end()) {
				it_msgsock->second->SendControlMessage(TYPE_EXIT);
				theApp.msgsock.erase(it_msgsock);
			}
			//������ͨ���׽���ӳ���б���
			theApp.msgsock[tmp.username] = this;
			this->name = tmp.username;

			//���䷢�ͻ�����Ϣ
			it_unsend_msg = theApp.Unsend_Mesg.find(tmp.username);
			while (it_unsend_msg != theApp.Unsend_Mesg.end())
			{
				this->SendTextData(it_unsend_msg->second);
				theApp.Unsend_Mesg.erase(it_unsend_msg);
				it_unsend_msg = theApp.Unsend_Mesg.find(tmp.username);
			}

			//�ڽ�����ɾ��������Ϣ
			int cnt = pdlg->list_msg.GetItemCount();
			for (int i = 0; i < cnt; i++) {
				if (pdlg->list_msg.GetItemText(i, 0) == tmp.username) {
					pdlg->list_msg.DeleteItem(i);
					i--;
				}
			}

			//�ڽ�������������¼�
			CString str_eve = CString("���ߣ�") + name;
			pdlg->list_eve.InsertString(0, str_eve);
			
			//�����������û���������֪ͨ
			for (it_msgsock = theApp.msgsock.begin(); it_msgsock != theApp.msgsock.end(); it_msgsock++) {
				if(it_msgsock->first != name) it_msgsock->second->SendNotification(TYPE_ONLINE, name);
			}

			//����û����������û�״̬
			for (auto it_msgsock = theApp.Registed_User.begin(); it_msgsock != theApp.Registed_User.end(); it_msgsock++) {
				if (it_msgsock->first != name) {
					this->SendNotification((it_msgsock->second.alive ? TYPE_ONLINE : TYPE_OFFLINE), it_msgsock->first);
				}
			}
		}
		else {
			this->SendControlMessage(TYPE_FAILED);

			CString str_eve = CString("��¼ʧ�ܣ�") + name;
			pdlg->list_eve.InsertString(0, str_eve);
		}
		break;

	case TYPE_REGIST://						ע��
		len1 = data[1];
		len2 = data[2];

		memcpy(name, &data[3], len1 * 2);
		memcpy(pass, &data[3 + len1], len2 * 2);

		tmp.username = name;
		tmp.password = pass;

		it_registed_user = theApp.Registed_User.find(tmp.username);
		if (it_registed_user == theApp.Registed_User.end()) {		//ע��ɹ�

			//��ͻ��˷���ע��ɹ���Ϣ
			this->SendControlMessage(TYPE_SUCCEED);

			//������ͨ���׽���ӳ���б���
			theApp.msgsock[tmp.username] = this;
			this->name = tmp.username;

			//�ڽ����и����û�������Ϣ
			pdlg->list_online.InsertString(theApp.online, tmp.username);
			theApp.online++;

			//�ں�̨�����û�������Ϣ
			theApp.Registed_User[tmp.username] = tmp;
			theApp.Registed_User[tmp.username].alive = true;

			//�ڽ��������ע���¼�
			CString str_eve = CString("ע��ɹ���") + tmp.username;
			pdlg->list_eve.InsertString(0, str_eve);
			theApp.msgsock[tmp.username] = this;
			this->name = tmp.username;

			//���û���Ϣд�������ļ�
			std::ofstream writer(theApp.PATH, std::ios::binary | std::ios::app);
			writer.write((char*)&name, USER_SIZE * 2);
			writer.write((char*)&pass, PASS_SIZE * 2);
			writer.close();

			//�����������û���������֪ͨ
			for (it_msgsock = theApp.msgsock.begin(); it_msgsock != theApp.msgsock.end(); it_msgsock++) {
				if (it_msgsock->first != name) it_msgsock->second->SendNotification(TYPE_ONLINE, name);
			}

			//����û����������û�״̬
			for (it_registed_user = theApp.Registed_User.begin(); it_registed_user != theApp.Registed_User.end(); it_registed_user++) {
				if (it_registed_user->first != name) {
					this->SendNotification((it_registed_user->second.alive ? TYPE_ONLINE : TYPE_OFFLINE), it_registed_user->first);
				}
			}

		}
		else {
			CString str_eve = CString("ע��ʧ�ܣ�") + tmp.username;
			pdlg->list_eve.InsertString(0, str_eve);

			this->SendControlMessage(TYPE_FAILED);
		}

		break;

	case TYPE_TEXT://						��Ϣ
		memcpy(&mesg, &data[1], sizeof(mesg));

		CString str_eve = CString("������Ϣ��") + mesg.from + CString(" --> ") + mesg.to;
		pdlg->list_eve.InsertString(0, str_eve);

		if (theApp.msgsock.find(mesg.to) == theApp.msgsock.end()) {		//������

			theApp.Unsend_Mesg.insert(std::make_pair(mesg.to, mesg));

			int nRow = pdlg->list_msg.InsertItem(0, mesg.to);
			pdlg->list_msg.SetItemText(nRow, 1, mesg.from);
			pdlg->list_msg.SetItemText(nRow, 2, mesg.content); 
		}
		else {															//����
			theApp.msgsock[mesg.to]->SendTextData(mesg);
		}
		break;

	}
}

//		���Ϳ��Ʊ������ݣ���¼/ע��ɹ�/ʧ��
void SocketC::SendControlMessage(int type)
{
	wchar_t packet[PACKET_SIZE] = { 0 };
	packet[0] = type;

	Send(packet, sizeof(packet));

	if (type == TYPE_FAILED) {
		CSocket::Close();
	}

}

//		�����ı�����
void SocketC::SendTextData(Mesg mesg)
{
	wchar_t packet[PACKET_SIZE] = { 0 };

	packet[0] = TYPE_TEXT;
	memcpy(&packet[1], &mesg, sizeof(mesg)); 

	Send(packet, sizeof(packet));
}

//		����֪ͨ������/����
void SocketC::SendNotification(int type, CString name)
{
	wchar_t packet[PACKET_SIZE] = { 0 };
	int len = name.GetLength();

	packet[0] = type;
	packet[1] = len;
	memcpy(&packet[2], name, len * 2);

	Send(packet, sizeof(packet));
}

//		�ͻ����˳�
void SocketC::OnClose(int nErrorCode)
{
	CString str_eve = CString("���ߣ�") + name;
	pdlg->list_eve.InsertString(0, str_eve);

	//delete theApp.msgsock[name];
	theApp.msgsock.erase(this->name);

	pdlg->list_online.DeleteString(pdlg->list_online.FindString(0, this->name));

	std::map<CString, SocketC*>::iterator it;
	theApp.Registed_User[name].alive = false;

	for (it = theApp.msgsock.begin(); it != theApp.msgsock.end(); it++) {
		it->second->SendNotification(TYPE_OFFLINE, this->name);
	}

	theApp.online--;
	
	CSocket::OnClose(nErrorCode);
}


