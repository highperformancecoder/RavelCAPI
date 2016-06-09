// dialog.h - Construct dialog boxes
// Copyright (c) 2006-2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once

namespace xll {
	template<class X>
	class XDialog {
		enum { 
			ItemNumber,
			HorizontalPosition,
			VerticalPosition,
			ItemWidth,
			ItemHeight,
			Text,
			InitialValue
		};
		XOPER<X> ref_, res_;
	public:
		typedef typename traits<X>::xcstr xcstr;
		typedef typename traits<X>::xword xword;
		enum DialogItem {
			DefaultOKButton = 1,
			CancelButton = 2,
			OKButton = 3,
			DefaultCancelButton = 4,
			StaticText = 5,
			TextEditBox = 6,
			IntegerEditBox = 7,
			NumberEditBox = 8,
			FormulaEditBox = 9,
			ReferenceEditBox = 10,
			OptionButtonGroup = 11,
			OptionButton = 12,
			CheckBox = 13,
			GroupBox = 14,
			ListBox = 15,
			LinkedListBox = 16,
			Icons = 17,
			LinkedFileListBox = 18,
			LinkedDriveAndDirectoryBox = 19,
			DirectoryTextBox = 20,
			DropDownListBox = 21,
			DropDownCombinationBox = 22,
			PictureButton = 23,
			HelpButton = 24
		};
		XDialog(xcstr name, int x, int y, int w, int h, int trigger = 0)
			: ref_(1, 7)
		{
			ref_(0, Text) = name;
			ref_(0, HorizontalPosition) = x;
			ref_(0, VerticalPosition) = y;
			ref_(0, ItemWidth) = w;
			ref_(0, ItemHeight) = h;
			if (trigger)
				ref_(0, InitialValue) = trigger;
		}
		// Returns 1-based position of item.
		xword Add(int item, int x = 0, int y = 0, int w = 0, int h = 0)
		{
			xword r = ref_.rows();
			ref_.resize(r + 1, 7);
			ref_(r, ItemNumber) = item;
			if (x) 
				ref_(r, HorizontalPosition) = x;
			if (y) 
				ref_(r, VerticalPosition) = y;
			if (w) 
				ref_(r, ItemWidth) = w;
			if (h) 
				ref_(r, ItemHeight) = h;

			return r;
		}
		xword Add(int item, xcstr text, int x = 0, int y = 0, int w = 0, int h = 0)
		{
			xword r = Add(item, x, y, w, h);
			ref_(r, Text) = text;

			return r;
		}
		void Set(xword i, const XOPER<X>& xValue)
		{
			ref_(i, 6) = xValue;
		}
		XOPER<X>& Get(xword i)
		{
			return res_.xltype == xltypeMulti ? res_(i, 6) : res_;
		}
		// Returns what was selected, row, or boolean success/failure.
		XOPER<X>& Show(void)
		{
			res_ = Excel<X>(xlfDialogBox, ref_);

			return res_[0] == XNil<X>() ? res_[6] : res_[0];
		}
	};

	typedef XDialog<XLOPER> Dialog;
	typedef XDialog<XLOPER12> Dialog12;
	typedef X_(Dialog) DialogX;

} // namespace xll