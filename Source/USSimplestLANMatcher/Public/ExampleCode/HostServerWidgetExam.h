// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HostServerWidgetExam.generated.h"

/**
 * 
 */
UCLASS()
class USSIMPLESTLANMATCHER_API UHostServerWidgetExam : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	virtual void BeginDestroy() override;
	

	UFUNCTION(Category= "HostServerWidgetExam", BlueprintCallable)
	void OnHostButtonHit();
	
	UFUNCTION(Category= "HostServerWidgetExam", BlueprintCallable)
	void OnCloseButtonHit();



private:
	
	UPROPERTY(meta=(BindWidget)) 
	class UEditableTextBox* InPortNo;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ServerState;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ServerID;
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* ServerIP;

	UPROPERTY()
	class UUSLANMatcherHostServerSubsystem* ServerSubsystem;


};
