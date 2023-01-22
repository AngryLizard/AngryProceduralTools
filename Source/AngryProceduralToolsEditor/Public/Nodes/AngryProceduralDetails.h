// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class AProceduralActor;

class FAngryProceduralDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

	/** Create static mesh from procedural component */
	UStaticMesh* CreateStaticMesh(AProceduralActor* ProceduralActor);

	/** Handle clicking the convert button */
	FReply ClickedOnConvertToStaticMesh();

	/** Handle clicking the replace button */
	FReply ClickedOnReplaceToStaticMesh();

	/** Is the convert button enabled */
	bool StaticMeshEnabled() const;

	/** Util to get the ProcMeshComp we want to convert */
	AProceduralActor* GetFirstSelectedProceduralActor() const;

	/** Cached array of selected objects */
	TArray< TWeakObjectPtr<UObject> > SelectedObjectsList;
};
