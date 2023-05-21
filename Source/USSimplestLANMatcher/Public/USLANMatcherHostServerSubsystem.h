// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Sockets.h"
#include "USLANMatcherUtilPublic.h"

#include "USLANMatcherHostServerSubsystem.generated.h"


enum class JOIN_REQ_RECV_RESULT : uint8
{
	SUCCEDED = 0,
	NO_PACKET_TO_READ = 1,
	INVALID_PACKET,
	INVALID_PACKET_VER,
	INVALID_SERVER_ID,
	UNKNOWN_FAIL,

	/** It can occur when you try get join request through unhosted server*/
	INVALID_ACCESS
};

enum class JOIN_ACK_SEND_RESULT
{
	SUCCEDED,
	UNKNOWN_FAIL,

	/** It can occur when you try get join request through unhosted server*/
	INVALID_ACCESS
};


/**
 * 
 */
UCLASS()
class USSIMPLESTLANMATCHER_API UUSLANMatcherHostServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/**
	 * Host socket to receive response from server.
	 * @param DesiredPortNo	Default value is DEFAULT_SERVER_PORT_NO
	 * @returns SOCKET_CONSTRUCT_RESULT
	 */
	SOCKET_CONSTRUCT_RESULT HostServer(int32 DesiredPortNo = DEFAULT_SERVER_PORT_NO);

	void CloseServer();


	inline int32 GetPortNo() const { return bIsServerHosting ? USServerSock->GetPortNo() : 0; }
	inline FString GetServerID() const { return bIsServerHosting ? ServerID : ""; }
	inline uint32 GetServerIDHash() const { return bIsServerHosting ? ServerIDHash : 0; }
	inline int8 GetbIsServerHosting() { return bIsServerHosting; }
	FString GetCurrentWorldIP(bool bWithPortNo = true) const;

	/**
	 * @param outSenderInfo	If successful, return Game ServerAddr.
	*/
	JOIN_REQ_RECV_RESULT TryReceiveJoinReq(JoinRequesterInfo& outSenderInfo);

	JOIN_ACK_SEND_RESULT SendJoinAck(const JoinRequesterInfo& inDestInfo, const FString& inGameServerIP);

	/**
	 * @param InGameServerURL the ServerIP for client to join when client get acknowledged.
	 */
	void ProcessRequests(const FString& InGameServerURL);

private:
	int8 bIsServerHosting = false;

	char ServerID[SERVER_ID_LEN];
	uint32 ServerIDHash;
	TSharedPtr<struct JoinRequestPacket> JoinReqValidationPacket;
	void GenRandomServerIDInfo();

	FSocket* USServerSock;
	TSharedPtr<FInternetAddr> AddrCache;
};
