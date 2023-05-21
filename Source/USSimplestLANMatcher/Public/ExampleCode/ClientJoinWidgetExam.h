// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ClientJoinWidgetExam.generated.h"

/**
 * 
 */
UCLASS()
class USSIMPLESTLANMATCHER_API UClientJoinWidgetExam : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	virtual void BeginDestroy() override;
	
	UFUNCTION(Category= "ClientJoinWidgetExam", BlueprintCallable)
	void OnFindButtonHit();
	
	UPROPERTY(Category= "ClientJoinWidgetExam", BlueprintReadOnly);
	FString ServerIpAddrStr;

private:
	UPROPERTY( meta=(BindWidget))
	class UEditableTextBox* InServerID;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ServerFindState;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ServerIP;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category= "ClientJoinWidgetExam")
	class UCheckBox* CBUseTargetIP;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category= "ClientJoinWidgetExam")
	class UEditableTextBox* InTargetIP;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget), Category= "ClientJoinWidgetExam")
	class UEditableTextBox* InTargetPort;

	UPROPERTY()
	class UUSLANMatcherClientSubsystem* ClientSubsystem;


};
