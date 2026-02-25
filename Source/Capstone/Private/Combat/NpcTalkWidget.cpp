#include "Combat/NpcTalkWidget.h"

#include "Components/TextBlock.h"

void UNpcTalkWidget::SetNpcName(const FText& InName)
{
	if (NameText)
	{
		NameText->SetText(InName);
	}
}

void UNpcTalkWidget::SetLine(const FText& InLine)
{
	if (LineText)
	{
		LineText->SetText(InLine);
	}
}

void UNpcTalkWidget::Clear()
{
	if (NameText)
	{
		NameText->SetText(FText::GetEmpty());
	}
	if (LineText)
	{
		LineText->SetText(FText::GetEmpty());
	}
}