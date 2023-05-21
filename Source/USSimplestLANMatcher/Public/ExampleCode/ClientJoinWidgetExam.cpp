// Fill out your copyright notice in the Description page of Project Settings.


#include "ClientJoinWidgetExam.h"
#include "../USLANMatcherClientSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/CheckBox.h"
#include "Engine.h"

void UClientJoinWidgetExam::NativeConstruct()
{
	Super::NativeConstruct();
	
	ClientSubsystem = GetGameInstance()->GetSubsystem<UUSLANMatcherClientSubsystem>();
	ClientSubsystem->HostResponseRecvSocket();
}

void UClientJoinWidgetExam::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);


	// client logic
	if (ClientSubsystem != nullptr)
	{
		
		JOIN_ACK_RECV_RESULT result = ClientSubsystem->TryRecvJoinAck(ServerIpAddrStr);
		if (result == JOIN_ACK_RECV_RESULT::SUCCEDED)
		{
			ServerFindState->SetText(FText::FromString(TEXT("Join Ack!")));
		}
		else if (result != JOIN_ACK_RECV_RESULT::NO_PACKET_TO_READ)
		{
			UE_LOG(LogTemp, Log, TEXT("%d"), result);
		}
	}
}

void UClientJoinWidgetExam::BeginDestroy()
{
	Super::BeginDestroy();
	if (ClientSubsystem != nullptr && ClientSubsystem->GetbIsClientHosting()) ClientSubsystem->CloseClientSocket();
}

void UClientJoinWidgetExam::OnFindButtonHit()
{
	bool bUseTargetIP = CBUseTargetIP->IsChecked();

	FString ServerID = InServerID->GetText().ToString();

	UE_LOG(LogTemp, Log, TEXT("Finding Server..."));

	JOIN_REQ_SEND_RESULT result;
	if(!bUseTargetIP)
	{
		result = ClientSubsystem->SendJoinReq(*ServerID);
	}
	else
	{
		const FString& TargetIP = InTargetIP->GetText().ToString();
		const int TargetPort = FCString::Atoi(*InTargetPort->GetText().ToString());
		result = ClientSubsystem->SendJoinReq(*ServerID, TargetPort, false, TargetIP);
	}

	
	FString resultMsg;
	FColor resultColor;
	
	switch(result)
	{
	case JOIN_REQ_SEND_RESULT::SUCCEDED:
		resultMsg = TEXT("SUCCEDED");
		resultColor = FColor::Green;
		break;
		
	case JOIN_REQ_SEND_RESULT::UNKNOWN_FAIL:
		resultMsg = TEXT("UNKNOWN_FAIL");
		resultColor = FColor::Red;
		break;
		
	case JOIN_REQ_SEND_RESULT::INVALID_ACCESS:
		resultMsg = TEXT("INVALID_ACCESS");
		resultColor = FColor::Red;
		break;
		
	case JOIN_REQ_SEND_RESULT::INVALID_IP:
		resultMsg = TEXT("INVALID_IP");
		resultColor = FColor::Red;
		break;
		
	case JOIN_REQ_SEND_RESULT::INVALID_SERVER_ID:
		resultMsg = TEXT("INVALID_SERVER_ID");
		resultColor = FColor::Yellow;
		break;
	}

	GEngine->AddOnScreenDebugMessage(10, 10.f, resultColor, resultMsg);
}
