// Fill out your copyright notice in the Description page of Project Settings.


#include "USLANMatcherClientSubsystem.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Misc/FeedbackContext.h"

#include "USLANMatcherUtilPrivate.h"


void UUSLANMatcherClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Instancing variable to recv Server IP info
	{
		ServerAddrStorage = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	}

	// init broadcast Addr
	{
		JoinReqAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		JoinReqAddr->SetBroadcastAddress();
		int32 portNo = DEFAULT_SERVER_PORT_NO;
		JoinReqAddr->SetPort(portNo);
	}

	JoinAckValidationPacket = MakeShared<JoinAckPacket>();

	UE_LOG(LogUSLanMatcher, Log, TEXT("USLanMatcher Client Subsystem Ready!"));
}

SOCKET_CONSTRUCT_RESULT UUSLANMatcherClientSubsystem::HostResponseRecvSocket(int32 DesiredPortNo)
{
	if(bIsClientHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("LAN Match client is already running."));
		return SOCKET_CONSTRUCT_RESULT::ALREADY_RUNNING;
	}
	
	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	FOutputDevice* Warn = (FOutputDevice*)GWarn;
	const TSharedRef<FInternetAddr> ClientAddr = SocketSubsystem->GetLocalBindAddr(*Warn);
	ClientAddr->SetPort(DesiredPortNo);

	USJoinAckRecvSock = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("USLANMAtcher_Client"));

	if (USJoinAckRecvSock == nullptr)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("SOCKET_CREATION_FAIL(CLIENT)"));
		return SOCKET_CONSTRUCT_RESULT::SOCKET_CREATION_FAIL;
	}

	if (!USJoinAckRecvSock->Bind(*ClientAddr))
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("BIND_FAIL(CLIENT)"));
		USJoinAckRecvSock->Close();
		return SOCKET_CONSTRUCT_RESULT::BIND_FAIL;
	}

	if (!USJoinAckRecvSock->SetNonBlocking())
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("NONBLOCK_NOT_SUPPORT(CLIENT)"));
		USJoinAckRecvSock->Close();
		return SOCKET_CONSTRUCT_RESULT::NONBLOCK_NOT_SUPPORT;
	}

	if (!USJoinAckRecvSock->SetBroadcast())
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("BROADCAST_NOT_SUPPORT(CLIENT)"));
		USJoinAckRecvSock->Close();
		return SOCKET_CONSTRUCT_RESULT::BROADCAST_NOT_SUPPORT;
	}

	bIsClientHosting = true;
	
	UE_LOG(LogUSLanMatcher, Log, TEXT("Client Waiting for Invite... (Port NO: %d)"), GetPortNo());
	return SOCKET_CONSTRUCT_RESULT::SUCCEDED;
}

void UUSLANMatcherClientSubsystem::CloseClientSocket()
{
	if(!bIsClientHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Server is alread disabled"));
		return;
	}
	
	USJoinAckRecvSock->Close();
	bIsClientHosting = false;
	UE_LOG(LogUSLanMatcher, Log, TEXT("Close hosted client."));

}

JOIN_REQ_SEND_RESULT UUSLANMatcherClientSubsystem::SendJoinReq(const TCHAR* ServerID, int32 ServerPortNo,
					bool bIsBroadcast, FString IPAddr)
{
	if(!bIsClientHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("HostResponseRecvSocket must be hosted before Recieve packet."));
		return JOIN_REQ_SEND_RESULT::INVALID_ACCESS;
	}
	
	if(!CheckServerIDFitFormat(ServerID))
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("ServerID is Invalid"));
		return JOIN_REQ_SEND_RESULT::INVALID_SERVER_ID;
	}

	if(bIsBroadcast)
	{
		JoinReqAddr->SetBroadcastAddress();
	}
	else
	{
		bool IsIPValid = true;
		JoinReqAddr->SetIp(*IPAddr, IsIPValid);
		if(!IsIPValid) return JOIN_REQ_SEND_RESULT::INVALID_IP;
	}
	
	JoinReqAddr->SetPort(ServerPortNo);

	JoinRequestPacket RequestPacket(ServerID);
	constexpr int32 BufferSize = sizeof(JoinRequestPacket);
	int32 BytesSent;

	
	if(USJoinAckRecvSock->SendTo((uint8*)&RequestPacket, BufferSize, BytesSent, *JoinReqAddr))
	{
		return JOIN_REQ_SEND_RESULT::SUCCEDED;
	}

	UE_LOG(LogUSLanMatcher, Warning, TEXT("Packet send fail with unknown reason."));
	return JOIN_REQ_SEND_RESULT::UNKNOWN_FAIL;
}


JOIN_ACK_RECV_RESULT UUSLANMatcherClientSubsystem::TryRecvJoinAck(FString& outString)
{
	if(!bIsClientHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Client must be hosted before Recieve packet."));
		return JOIN_ACK_RECV_RESULT::INVALID_ACCESS;
	}

	JoinAckPacket RecvedPacket;
	constexpr int8 BufferSize = sizeof(JoinAckPacket);
	int32 BytesRead;


	bool IsReceiveSucceded = USJoinAckRecvSock->RecvFrom(
		(uint8*)(&RecvedPacket), BufferSize, BytesRead, *ServerAddrStorage, ESocketReceiveFlags::None);
	
	if (IsReceiveSucceded)
	{
		if(RecvedPacket.PacketNameHash != JoinAckValidationPacket->PacketNameHash)
		{
			UE_LOG(LogUSLanMatcher, Warning, TEXT("(UUSLANMatcherClientSubsystem::TryRecvJoinAck)PacketNameHash: %u, PacketVersionHash: %u, GameServerIP: %s") 
				, RecvedPacket.PacketNameHash, RecvedPacket.PacketVersionValidateHash, RecvedPacket.GameServerIPName);
			return JOIN_ACK_RECV_RESULT::IVALID_PACKET;
		}
		
		if(RecvedPacket.PacketVersionValidateHash != JoinAckValidationPacket->PacketVersionValidateHash)
		{
			UE_LOG(LogUSLanMatcher, Warning, TEXT("(UUSLANMatcherClientSubsystem::TryRecvJoinAck)PacketNameHash: %u, PacketVersionHash: %u, GameServerIP: %s")
				, RecvedPacket.PacketNameHash, RecvedPacket.PacketVersionValidateHash, RecvedPacket.GameServerIPName);
			return JOIN_ACK_RECV_RESULT::IVALID_PACKET_VER;
		}

		// 자기자신의 패킷을 받았을 때 처리도 해주기

		outString = RecvedPacket.GameServerIPName;
		return JOIN_ACK_RECV_RESULT::SUCCEDED;
		
	}

	if(BytesRead == 0)
	{
		return JOIN_ACK_RECV_RESULT::NO_PACKET_TO_READ;
	}

	return JOIN_ACK_RECV_RESULT::UNKNOWN_FAIL;
	
}

