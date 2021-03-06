#ifndef __SFTPCONTROLSOCKET_H__
#define __SFTPCONTROLSOCKET_H__

#include "ControlSocket.h"

enum class sftpEvent {
	Unknown = -1,
	Reply = 0,
	Done,
	Error,
	Verbose,
	Info,
	Status,
	Recv,
	Send,
	Listentry,
	AskHostkey,
	AskHostkeyChanged,
	AskHostkeyBetteralg,
	AskPassword,
	Transfer,
	RequestPreamble,
	RequestInstruction,
	UsedQuotaRecv,
	UsedQuotaSend,
	KexAlgorithm,
	KexHash,
	KexCurve,
	CipherClientToServer,
	CipherServerToClient,
	MacClientToServer,
	MacServerToClient,
	Hostkey,

	count
};

namespace fz {
class process;
}

class CSftpInputThread;

struct sftp_message;
class CSftpControlSocket final : public CControlSocket, public CRateLimiterObject
{
public:
	CSftpControlSocket(CFileZillaEnginePrivate & engine);
	virtual ~CSftpControlSocket();

	virtual int Connect(const CServer &server);

	virtual int List(CServerPath path = CServerPath(), std::wstring const& subDir = std::wstring(), int flags = 0);
	virtual int Delete(const CServerPath& path, std::deque<std::wstring>&& files);
	virtual int RemoveDir(CServerPath const& path = CServerPath(), std::wstring const& subDir = std::wstring());
	virtual int Mkdir(const CServerPath& path);
	virtual int Rename(const CRenameCommand& command);
	virtual int Chmod(const CChmodCommand& command);
	virtual void Cancel();

	virtual bool Connected() { return m_pInputThread != 0; }

	virtual bool SetAsyncRequestReply(CAsyncRequestNotification *pNotification);

protected:
	// Replaces filename"with"quotes with
	// "filename""with""quotes"
	std::wstring QuoteFilename(std::wstring const& filename);

	virtual int DoClose(int nErrorCode = FZ_REPLY_DISCONNECTED);

	virtual int ResetOperation(int nErrorCode);
	virtual int SendNextCommand();
	virtual int ParseSubcommandResult(int prevResult);

	void ProcessReply(int result, std::wstring const& reply);

	int ConnectParseResponse(bool successful, std::wstring const& reply);
	int ConnectSend();

	virtual int FileTransfer(std::wstring const& localFile, CServerPath const& remotePath,
							 std::wstring const& remoteFile, bool download,
							 CFileTransferCommand::t_transferSettings const& transferSettings);
	int FileTransferSubcommandResult(int prevResult);
	int FileTransferSend();
	int FileTransferParseResponse(int result, std::wstring const& reply);

	int ListSubcommandResult(int prevResult);
	int ListSend();
	int ListParseResponse(bool successful, std::wstring const& reply);
	int ListParseEntry(std::wstring && entry, std::wstring const& stime, std::wstring && name);
	int ListCheckTimezoneDetection();

	int ChangeDir(CServerPath path = CServerPath(), std::wstring subDir = std::wstring(), bool link_discovery = false);
	int ChangeDirParseResponse(bool successful, std::wstring const& reply);
	int ChangeDirSubcommandResult(int prevResult);
	int ChangeDirSend();

	int MkdirParseResponse(bool successful, std::wstring const& reply);
	int MkdirSend();

	int DeleteParseResponse(bool successful, std::wstring const& reply);
	int DeleteSend();

	int RemoveDirParseResponse(bool successful, std::wstring const& reply);

	int ChmodParseResponse(bool successful, std::wstring const& reply);
	int ChmodSubcommandResult(int prevResult);
	int ChmodSend();

	int RenameParseResponse(bool successful, std::wstring const& reply);
	int RenameSubcommandResult(int prevResult);
	int RenameSend();

	bool SendCommand(std::wstring const& cmd, std::wstring const& show = std::wstring());
	bool AddToStream(std::wstring const& cmd, bool force_utf8 = false);

	virtual void OnRateAvailable(CRateLimiter::rate_direction direction);
	void OnQuotaRequest(CRateLimiter::rate_direction direction);

	// see src/putty/wildcard.c
	std::wstring WildcardEscape(std::wstring const& file);

	fz::process* m_pProcess{};
	CSftpInputThread* m_pInputThread{};

	virtual void operator()(fz::event_base const& ev);
	void OnSftpEvent(sftp_message const& message);
	void OnTerminate(std::wstring const& error);

	std::wstring m_requestPreamble;
	std::wstring m_requestInstruction;

	CSftpEncryptionNotification m_sftpEncryptionDetails;
};

#endif //__SFTPCONTROLSOCKET_H__
