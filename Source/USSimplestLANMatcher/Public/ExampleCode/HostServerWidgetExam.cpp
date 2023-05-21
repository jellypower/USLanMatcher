// Fill out your copyright notice in the Description page of Project Settings.


#include "HostServerWidgetExam.h"
#include "../USLANMatcherHostServerSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UHostServerWidgetExam::NativeConstruct()
{
	Super::NativeConstruct();
	ServerSubsystem = GetGameInstance()->GetSubsystem<UUSLANMatcherHostServerSubsystem>();
}

void UHostServerWidgetExam::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	
	// server logic
	if (ServerSubsystem != nullptr && ServerSubsystem->GetbIsServerHosting())
		ServerSubsystem->ProcessRequests(ServerSubsystem->GetCurrentWorldIP());
}

void UHostServerWidgetExam::BeginDestroy()
{
	Super::BeginDestroy();
	if (ServerSubsystem != nullptr && ServerSubsystem->GetbIsServerHosting()) ServerSubsystem->CloseServer();
}

void UHostServerWidgetExam::OnHostButtonHit()
{

	FText txt = InPortNo->GetText();

	int32 portNo = txt.IsNumeric()
					? FCString::Atoi(*txt.ToString()) : DEFAULT_SERVER_PORT_NO;

	if (ServerSubsystem->HostServer(portNo) == SOCKET_CONSTRUCT_RESULT::SUCCEDED)
	{
		ServerState->SetText(FText::FromString(FString::Printf(TEXT("Server Hosted!"))));
		ServerID->SetText(FText::FromString(ServerSubsystem->GetServerID()));
		ServerIP->SetText(FText::FromString(FString::Printf(TEXT("%s:%d"),
			*ServerSubsystem->GetCurrentWorldIP(false) ,ServerSubsystem->GetPortNo())));
	}
}

void UHostServerWidgetExam::OnCloseButtonHit()
{
	ServerSubsystem->CloseServer();

	ServerState->SetText(FText::FromString("Not hosted"));
	ServerID->SetText(FText::FromString("ServerID"));
	ServerIP->SetText(FText::FromString(""));
}
