// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "USLANMatcherUtilPublic.h"


DECLARE_LOG_CATEGORY_EXTERN(LogUSLanMatcher, Log, All);





#define JOIN_REQ_PACKET_NAME TEXT("USSimplestJoinReqPacket")
#define JOIN_REQ_PACKET_VERSION TEXT("0.0.0")
struct JoinRequestPacket
{
	JoinRequestPacket(FString ServerID)	:
	PacketNameHash(TextKeyUtil::HashString(JOIN_REQ_PACKET_NAME)),
	PacketVersionValidateHash(TextKeyUtil::HashString(JOIN_REQ_PACKET_VERSION)),
	ServerIDHash(TextKeyUtil::HashString(ServerID))
	{ }

	JoinRequestPacket() :
	PacketNameHash(TextKeyUtil::HashString(JOIN_REQ_PACKET_NAME)),
	PacketVersionValidateHash(TextKeyUtil::HashString(JOIN_REQ_PACKET_VERSION)),
	ServerIDHash(0)
	{ }

 
	// Packet Data
	uint32 PacketNameHash;
	uint32 PacketVersionValidateHash;
	uint32 ServerIDHash;
 
};

#define JOIN_ACK_PACKET_NAME TEXT("USSimplestJoinAckPacket")
#define JOIN_ACK_PACKET_VERSION TEXT("0.0.0")
struct JoinAckPacket
{
	JoinAckPacket(const FString& ServerIPName) :
	PacketNameHash(TextKeyUtil::HashString(JOIN_ACK_PACKET_NAME)),
	PacketVersionValidateHash(TextKeyUtil::HashString(JOIN_ACK_PACKET_VERSION))
	{
		memset(GameServerIPName, 0, SERVER_IP_NAME_LEN);
		TCString<TCHAR>::Strncpy(GameServerIPName, *ServerIPName, SERVER_IP_NAME_LEN);
	}

	JoinAckPacket() :
	PacketNameHash(TextKeyUtil::HashString(JOIN_ACK_PACKET_NAME)),
	PacketVersionValidateHash(TextKeyUtil::HashString(JOIN_ACK_PACKET_VERSION))
	{
		memset(GameServerIPName, 0, SERVER_IP_NAME_LEN);
	}

 
	// Packet Data
	uint32 PacketNameHash;
	uint32 PacketVersionValidateHash;
	TCHAR GameServerIPName[SERVER_IP_NAME_LEN];
 
};

/**
 * @return return true if ServerID fit to format. else return false. 
 */
inline bool CheckServerIDFitFormat(const TCHAR* ServerID)
{
	
	if(TCString<TCHAR>::Strlen(ServerID) != SERVER_ID_LEN) return false;
	
	for(int i=0;i<SERVER_ID_LEN;i++)
	{
		if('A' > ServerID[i] || 'Z' < ServerID[i])
		{
			return false;
		}
	}

	return true;
}

