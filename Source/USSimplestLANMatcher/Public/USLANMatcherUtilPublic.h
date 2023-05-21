// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


constexpr int32 DEFAULT_SERVER_PORT_NO = 8889;

constexpr uint8 SERVER_ID_LEN = 6;

constexpr uint8 SERVER_IP_NAME_LEN = 24;
struct JoinAckInfo
{
	TCHAR GameServerDomainToJoin[SERVER_IP_NAME_LEN];
	uint32 ServerIP;
	int32 PortNo;
};

struct JoinRequesterInfo
{
	uint32 IP;
	int32 PortNo;
};


enum class SOCKET_CONSTRUCT_RESULT
{
	SUCCEDED = 0,
	SOCKET_CREATION_FAIL,
	BIND_FAIL,
	NONBLOCK_NOT_SUPPORT,
	BROADCAST_NOT_SUPPORT,
	ALREADY_RUNNING
};