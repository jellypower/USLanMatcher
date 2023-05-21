// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Sockets.h"
#include "USLANMatcherUtilPublic.h"

#include "USLANMatcherClientSubsystem.generated.h"

constexpr int32 DEFAULT_CLIENT_PORT_NO = 0;


enum class JOIN_REQ_SEND_RESULT
{
	SUCCEDED,
	INVALID_IP,
	INVALID_SERVER_ID,
	UNKNOWN_FAIL,
	
	/** It can occur when you try get join request through unhosted server*/
	INVALID_ACCESS 
};

enum class JOIN_ACK_RECV_RESULT : uint8
{
	SUCCEDED = 0,
	NO_PACKET_TO_READ = 1,
	IVALID_PACKET,
	IVALID_PACKET_VER,
	UNKNOWN_FAIL,

	/** It can occur when you try get join request through unhosted server*/
	INVALID_ACCESS 
};

/**
 * 
 */
UCLASS()
class USSIMPLESTLANMATCHER_API UUSLANMatcherClientSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Host socket to receive response from server.
	 * @param DesiredPortNo	Default value is 0 (random Port No)
	 * @returns SOCKET_CONSTRUCT_RESULT
	 */
	SOCKET_CONSTRUCT_RESULT HostResponseRecvSocket(int32 DesiredPortNo = DEFAULT_CLIENT_PORT_NO);
	void CloseClientSocket();

	inline int32 GetPortNo() const { return bIsClientHosting ? USJoinAckRecvSock->GetPortNo() : 0; }
	inline int8 GetbIsClientHosting() const { return bIsClientHosting; }
	
	/**
	 * send socket requesting to join server. (Default action is broadcast)
	 * @param ServerID		ServerID of Matching server. 
	 * @param ServerPortNo	Session server port number. (Default is DEFAULT_SERVER_PORT_NO) 
	 * @param bIsBroadcast	If true, broadcast request. In false, send join request to TargetIPAddr
	 * @param TargetIPAddr	IP Addr to send Join request. bIsBroadcast must be false. Not include Port No.
	 */
	JOIN_REQ_SEND_RESULT SendJoinReq(const TCHAR* ServerID, int32 ServerPortNo = DEFAULT_SERVER_PORT_NO
	                                , bool bIsBroadcast = true, FString TargetIPAddr = TEXT(""));

	/**
	 * @param outGameServerAddr	If successful, return Game ServerAddr.
	 */
	JOIN_ACK_RECV_RESULT TryRecvJoinAck(FString& outGameServerAddr);
	
private:
	
	int8 bIsClientHosting = false;

	TSharedPtr<struct JoinAckPacket> JoinAckValidationPacket;
	
	FSocket* USJoinAckRecvSock;
	TSharedPtr<class FInternetAddr> JoinReqAddr;
	TSharedPtr<FInternetAddr> ServerAddrStorage;
	
};
