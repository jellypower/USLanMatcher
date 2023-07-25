// Fill out your copyright notice in the Description page of Project Settings.


#include "USLANMatcherHostServerSubsystem.h"

#include "SocketSubsystem.h"
#include "IPAddress.h"
#include "Misc/FeedbackContext.h"

#include "USLANMatcherUtilPrivate.h"


void UUSLANMatcherHostServerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize Addr address cache
	AddrCache = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	JoinReqValidationPacket = MakeShared<JoinRequestPacket>();

	UE_LOG(LogUSLanMatcher, Log, TEXT("USLanMatcher Server Subsystem Ready!"));
}

void UUSLANMatcherHostServerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	if(GetbIsServerHosting())
		CloseServer();
}


SOCKET_CONSTRUCT_RESULT UUSLANMatcherHostServerSubsystem::HostServer(int32 DesiredPortNo)
{
	
	if (bIsServerHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("LAN Match host server is already running."));
		return SOCKET_CONSTRUCT_RESULT::ALREADY_RUNNING;
	}

	ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	FOutputDevice* Warn = (FOutputDevice*)GWarn;

	const TSharedRef<FInternetAddr> addr = SocketSubsystem->GetLocalBindAddr(*Warn);
	addr->SetPort(DesiredPortNo);

	USServerSock = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("USLANMAtcher_Server"));

	if (USServerSock == nullptr)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("SOCKET_CREATION_FAIL(SERVER)"));
		return SOCKET_CONSTRUCT_RESULT::SOCKET_CREATION_FAIL;
	}

	if (!USServerSock->Bind(*addr))
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("BIND_FAIL(SERVER)"));
		USServerSock->Close();
		return SOCKET_CONSTRUCT_RESULT::BIND_FAIL;
	}

	if (!USServerSock->SetNonBlocking())
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("NONBLOCK_NOT_SUPPORT(SERVER)"));
		USServerSock->Close();
		return SOCKET_CONSTRUCT_RESULT::NONBLOCK_NOT_SUPPORT;
	}


	GenRandomServerIDInfo();
	bIsServerHosting = true;

	UE_LOG(LogUSLanMatcher, Log, TEXT("Server hosting... (Port NO: %d)"), GetPortNo());
	return SOCKET_CONSTRUCT_RESULT::SUCCEDED;
}

void UUSLANMatcherHostServerSubsystem::CloseServer()
{
	if(!bIsServerHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Server is alread disabled"));
		return;
	}
	
	USServerSock->Close();
	bIsServerHosting = false;
	UE_LOG(LogUSLanMatcher, Log, TEXT("Close hosted server."));
}


FString UUSLANMatcherHostServerSubsystem::GetCurrentWorldIP(bool bWithPortNo) const
{
	bool canBind = false;
	TSharedRef<FInternetAddr> IP= 
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, canBind);

	const FString IPString = IP->ToString(false);

	if(bWithPortNo)
		return IPString + TEXT(":") + FString::FromInt(GetWorld()->URL.Port);
	else
		return IPString;
}

JOIN_REQ_RECV_RESULT UUSLANMatcherHostServerSubsystem::TryReceiveJoinReq(JoinRequesterInfo& outSenderInfo)
{
	if (!bIsServerHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Client must be hosted before Recieve packet."));
		return JOIN_REQ_RECV_RESULT::INVALID_ACCESS;
	}

	JoinRequestPacket ReceivedPacket;
	constexpr int8 BufferSize = sizeof(JoinRequestPacket);
	int32 BytesRead;

	memset(&outSenderInfo, 0, sizeof(FInternetAddr));

	bool IsReceiveSucceded = USServerSock->RecvFrom(
		(uint8*)(&ReceivedPacket), BufferSize, BytesRead, *AddrCache, ESocketReceiveFlags::None);

	if (IsReceiveSucceded)
	{
		if (ReceivedPacket.PacketNameHash != JoinReqValidationPacket->PacketNameHash)
		{
			UE_LOG(LogUSLanMatcher, Warning,
			       TEXT("(IVALID_PACKET)PacketNameHash: %u, PacketVersionHash: %u, ServerIDHash: %u")
			       , ReceivedPacket.PacketNameHash, ReceivedPacket.PacketVersionValidateHash,
			       ReceivedPacket.ServerIDHash);
			return JOIN_REQ_RECV_RESULT::INVALID_PACKET;
		}

		if (ReceivedPacket.PacketVersionValidateHash != JoinReqValidationPacket->PacketVersionValidateHash)
		{
			UE_LOG(LogUSLanMatcher, Warning,
			       TEXT("(INVALID_PACKET_VER)PacketNameHash: %u, PacketVersionHash: %u, ServerIDHash: %u")
			       , ReceivedPacket.PacketNameHash, ReceivedPacket.PacketVersionValidateHash,
			       ReceivedPacket.ServerIDHash);
			return JOIN_REQ_RECV_RESULT::INVALID_PACKET_VER;
		}

		if (ReceivedPacket.ServerIDHash != JoinReqValidationPacket->ServerIDHash)
		{
			UE_LOG(LogUSLanMatcher, Warning, TEXT("(INVALID_SERVER_ID)ServerIDHash: %u")
			       , ReceivedPacket.ServerIDHash);
			return JOIN_REQ_RECV_RESULT::INVALID_SERVER_ID;
		}


		AddrCache->GetIp(outSenderInfo.IP);
		outSenderInfo.PortNo = AddrCache->GetPort();
		return JOIN_REQ_RECV_RESULT::SUCCEDED;
	}

	if (BytesRead == 0)
	{
		memset(&ReceivedPacket, 0, BufferSize);
		return JOIN_REQ_RECV_RESULT::NO_PACKET_TO_READ;
	}

	memset(&ReceivedPacket, 0, BufferSize);
	return JOIN_REQ_RECV_RESULT::UNKNOWN_FAIL;
}

JOIN_ACK_SEND_RESULT UUSLANMatcherHostServerSubsystem::SendJoinAck(const JoinRequesterInfo& inDestInfo,
                                                                   const FString& inGameServerIP)
{
	if (!bIsServerHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Server must be hosted before Recieve packet."));
		return JOIN_ACK_SEND_RESULT::INVALID_ACCESS;
	}

	AddrCache->SetIp(inDestInfo.IP);
	AddrCache->SetPort(inDestInfo.PortNo);


	UE_LOG(LogTemp, Log, TEXT("%s"), *inGameServerIP);

	JoinAckPacket AckPacket(inGameServerIP);
	constexpr int32 BufferSize = sizeof(JoinAckPacket);
	int32 BytesSent;


	if (USServerSock->SendTo((uint8*)&AckPacket, BufferSize, BytesSent, *AddrCache))
	{
		return JOIN_ACK_SEND_RESULT::SUCCEDED;
	}

	UE_LOG(LogUSLanMatcher, Warning, TEXT("Packet send fail with unknown reason."));
	return JOIN_ACK_SEND_RESULT::UNKNOWN_FAIL;
}

void UUSLANMatcherHostServerSubsystem::ProcessRequests(const FString& InGameServerURL)
{
	if(!bIsServerHosting)
	{
		UE_LOG(LogUSLanMatcher, Warning, TEXT("Server must be hosted before Recieve packet."));
		return;
	}
	
	JOIN_REQ_RECV_RESULT result;
	JoinRequesterInfo sender;
	
	do
	{
		result = TryReceiveJoinReq(sender);

		if(result == JOIN_REQ_RECV_RESULT::SUCCEDED)
		{
			SendJoinAck(sender, InGameServerURL);
		}
	}
	while (result != JOIN_REQ_RECV_RESULT::NO_PACKET_TO_READ);
}


void UUSLANMatcherHostServerSubsystem::GenRandomServerIDInfo()
{
	for (int i = 0; i < SERVER_ID_LEN; i++) // Generate random server id
	{
		ServerID[i] = FMath::RandRange('A', 'Z');
	}
	ServerIDHash = TextKeyUtil::HashString(FString(ServerID));

	JoinReqValidationPacket->ServerIDHash = ServerIDHash;

	UE_LOG(LogUSLanMatcher, Warning, TEXT("(Validation Packet Info)"
		       "PacketNameHash: %u, PacketVersionHash: %u, ServerIDHash: %u")
	       , JoinReqValidationPacket->PacketNameHash, JoinReqValidationPacket->PacketVersionValidateHash,
	       JoinReqValidationPacket->ServerIDHash);
}
