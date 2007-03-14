#include <proto/intuition.h>
#include <proto/muimaster.h>

ULONG XGET(APTR obj, ULONG attr)
{
	ULONG val;
	GetAttr(attr, obj, &val);
	return val;
}


APTR ImageButton(CONST_STRPTR text, CONST_STRPTR image)
{
	APTR obj, img;

	img = MUI_NewObject("Dtpic.mui", MUIA_Dtpic_Name, image, TAG_DONE);

	if (text)
	{
		obj = VGroup,
			Child, HCenter(img),
			Child, TextObject, text, MUIA_Text_PreParse, "\033c", End,
		End;
	}
	else
	{
		obj = img;
	}

	return obj;
}
