#include "pch.h"
#include "SocketC.h"
#include "ClistDlg.h"
#include "OICQ.h"
#include "Mesg.h"

SocketC::SocketC()
{
}

SocketC::~SocketC()
{
	SocketC::Close();
}

//		�����ı�����
void SocketC::SendTextData(Mesg mesg)
{
	wchar_t packet[PACKET_SIZE] = { 0 };

	packet[0] = TYPE_TEXT;
	memcpy(&packet[1], &mesg, sizeof(mesg)); 

	Send(packet, sizeof(packet));
}

//		���Ϳ��Ʊ������ݣ���¼/ע��
void SocketC::SendControlMessage(CString username,CString pass, int type)
{
	wchar_t packet[PACKET_SIZE] = { 0 };
	int n1 = username.GetLength();
	int n2 = pass.GetLength();

	packet[0] = type;
	packet[1] = n1;
	packet[2] = n2;

	memcpy(&packet[3], username, n1*2);
	memcpy(&packet[3 + n1], pass, n2*2);

	Send(packet, sizeof(packet));
}

void SocketC::CloseSocket()
{
	SocketC::Close();
}

//�κ����ݵ���ʱ�����ô˺���	
void SocketC::OnReceive(int errcode)
{
	wchar_t tmpname[USER_SIZE] = { 0 };
	wchar_t data[PACKET_SIZE] = { 0 };
	unsigned int retvalue;
	int len1, len2;
	int index1, index2;
	std::map<CString, CChatDlg*>::iterator it_dlgs;
	Mesg mesg; 
	int offline, online, state;

	retvalue = this->Receive(data, length);
	if (retvalue == SOCKET_ERROR)return;

	switch (data[0]) {

	case TYPE_SUCCEED:				//��¼�ɹ�
		logindlg->OnOK();
		break;

	case TYPE_FAILED:				//��¼ʧ��
		logindlg->GetDlgItem(IDC_STATIC)->SetWindowTextW(_T("�˺Ż��������,��¼ʧ�ܣ�"));
		CSocket::Close();
		break;

	case TYPE_TEXT:					//�յ����˷��͵���Ϣ
		memcpy(&mesg, &data[1], sizeof(mesg));

		offline = listdlg->m_list.FindString(0, mesg.from + CString(" [����]"));
		online = listdlg->m_list.FindString(0, mesg.from + CString(" [����]"));
		state = offline > online ? 0 : 1;
		
		it_dlgs = theApp.dlgs.find(mesg.from);
		if (it_dlgs != theApp.dlgs.end()) {
			it_dlgs->second->state = state;
			it_dlgs->second->ShowWindow(SW_SHOWNORMAL);
		}
		else {
			theApp.dlgs[CString(mesg.from)] = new CChatDlg(CString(mesg.from));
			it_dlgs = theApp.dlgs.find(mesg.from);
			theApp.dlgs[CString(mesg.from)]->Create(IDD_CHAT_DIALOG);
			it_dlgs->second->state = state;
			theApp.dlgs[CString(mesg.from)]->ShowWindow(SW_SHOWNORMAL);
			
		}
		it_dlgs->second->show(mesg, mesg.from);
		break;

	case TYPE_ONLINE:				//�յ���������
		len1 = data[1];

		memcpy(tmpname, &data[2], len1 * 2);

		//�ڽ����и����û�״̬
		index1 = listdlg->m_list.FindString(0, tmpname + CString(" [����]"));
		index2 = listdlg->m_list.FindString(0, tmpname + CString(" [����]"));

		if (index1 != -1) 
		{
			listdlg->m_list.DeleteString(index1);
		}
		if (index2 != -1)
		{
			listdlg->m_list.DeleteString(index2);
		}

		listdlg->m_list.InsertString(0, tmpname + CString(" [����]"));

		//�ں�̨�и����û�״̬
		//����Ի����Ѿ���ʾ��������״̬Ҳ��Ҫ����
		it_dlgs = theApp.dlgs.find(tmpname);
		if (it_dlgs != theApp.dlgs.end()) {
			it_dlgs->second->state = 1;
			if (it_dlgs->second->IsWindowVisible()) {
				it_dlgs->second->ChangeState();
			}
		}

		break;

	case TYPE_OFFLINE:					//�յ���������
		len1 = data[1];

		memcpy(tmpname, &data[2], len1 * 2);
		
		//�ڽ����и����û�״̬
		index1 = listdlg->m_list.FindString(0, tmpname + CString(" [����]"));
		index2 = listdlg->m_list.FindString(0, tmpname + CString(" [����]"));

		if (index1 != -1)
		{
			listdlg->m_list.DeleteString(index1);
		}
		if (index2 != -1)
		{
			listdlg->m_list.DeleteString(index2);
		}

		listdlg->m_list.AddString(tmpname + CString(" [����]"));

		//�ں�̨�и����û�״̬
		//����Ի����Ѿ���ʾ��������״̬Ҳ��Ҫ����
		it_dlgs = theApp.dlgs.find(tmpname);
		if (it_dlgs != theApp.dlgs.end()) {
			it_dlgs->second->state = 0;
			if (it_dlgs->second->IsWindowVisible()) {
				it_dlgs->second->ChangeState();
			}
		}
		break;
	case TYPE_EXIT:						//ǿ���˳�
		AfxMessageBox(_T("�����˺��ڱ�ĵط���¼"));
		listdlg->PostMessageW(WM_QUIT, 0, 0); 
		break;
	}
}
