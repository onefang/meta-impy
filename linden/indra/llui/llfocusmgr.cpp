/** 
 * @file llfocusmgr.cpp
 * @brief LLFocusMgr base class
 *
 * Copyright (c) 2002-2007, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlife.com/developers/opensource/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at http://secondlife.com/developers/opensource/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 */

#include "linden_common.h"

#include "llfocusmgr.h"
#include "lluictrl.h"
#include "v4color.h"

const F32 FOCUS_FADE_TIME = 0.3f;

LLFocusMgr gFocusMgr;

LLFocusMgr::LLFocusMgr()
	:
	mLockedView( NULL ),
	mKeyboardLockedFocusLostCallback( NULL ),
	mMouseCaptor( NULL ),
	mKeyboardFocus( NULL ),
	mDefaultKeyboardFocus( NULL ),
	mKeyboardFocusLostCallback( NULL ),
	mTopCtrl( NULL ),
	mFocusWeight(0.f),
	mAppHasFocus(TRUE)   // Macs don't seem to notify us that we've gotten focus, so default to true
	#ifdef _DEBUG
		, mMouseCaptorName("none")
		, mKeyboardFocusName("none")
		, mTopCtrlName("none")
	#endif
{
}

LLFocusMgr::~LLFocusMgr()
{
	mFocusHistory.clear();
}

void LLFocusMgr::releaseFocusIfNeeded( LLView* view )
{
	if( childHasMouseCapture( view ) )
	{
		setMouseCapture( NULL );
	}

	if( childHasKeyboardFocus( view ))
	{
		if (view == mLockedView)
		{
			mLockedView = NULL;
			mKeyboardLockedFocusLostCallback = NULL;
			setKeyboardFocus( NULL, NULL );
		}
		else
		{
			setKeyboardFocus( mLockedView, mKeyboardLockedFocusLostCallback );
		}
	}

	if( childIsTopCtrl( view ) )
	{
		setTopCtrl( NULL );
	}
}


void LLFocusMgr::setKeyboardFocus(LLUICtrl* new_focus, FocusLostCallback on_focus_lost, BOOL lock)
{
	if (mLockedView && 
		(new_focus == NULL || 
			(new_focus != mLockedView && !new_focus->hasAncestor(mLockedView))))
	{
		// don't allow focus to go to anything that is not the locked focus
		// or one of its descendants
		return;
	}
	FocusLostCallback old_callback = mKeyboardFocusLostCallback;
	mKeyboardFocusLostCallback = on_focus_lost;

	//llinfos << "Keyboard focus handled by " << (new_focus ? new_focus->getName() : "nothing") << llendl;

	if( new_focus != mKeyboardFocus )
	{
		LLUICtrl* old_focus = mKeyboardFocus;
		mKeyboardFocus = new_focus;

		// clear out any existing flash
		if (new_focus)
		{
			mFocusWeight = 0.f;
		}
		mFocusTimer.reset();

		if( old_callback )
		{
			old_callback( old_focus );
		}

		#ifdef _DEBUG
			mKeyboardFocusName = new_focus ? new_focus->getName() : "none";
		#endif

		// If we've got a default keyboard focus, and the caller is
		// releasing keyboard focus, move to the default.
		if (mDefaultKeyboardFocus != NULL && mKeyboardFocus == NULL)
		{
			mDefaultKeyboardFocus->setFocus(TRUE);
		}

		LLView* focus_subtree = mKeyboardFocus;
		LLView* viewp = mKeyboardFocus;
		// find root-most focus root
		while(viewp)
		{
			if (viewp->isFocusRoot())
			{
				focus_subtree = viewp;
			}
			viewp = viewp->getParent();
		}

		
		if (focus_subtree)
		{
			mFocusHistory[focus_subtree->mViewHandle] = mKeyboardFocus ? mKeyboardFocus->mViewHandle : LLViewHandle::sDeadHandle; 
		}
	}
	
	if (lock)
	{
		lockFocus();
	}
}

void LLFocusMgr::setDefaultKeyboardFocus(LLUICtrl* default_focus)
{
	mDefaultKeyboardFocus = default_focus;
}

// Returns TRUE is parent or any descedent of parent has keyboard focus.
BOOL LLFocusMgr::childHasKeyboardFocus(const LLView* parent ) const
{
	LLView* focus_view = mKeyboardFocus;
	while( focus_view )
	{
		if( focus_view == parent )
		{
			return TRUE;
		}
		focus_view = focus_view->getParent();
	}
	return FALSE;
}

// Returns TRUE is parent or any descedent of parent is the mouse captor.
BOOL LLFocusMgr::childHasMouseCapture( LLView* parent )
{
	if( mMouseCaptor && mMouseCaptor->isView() )
	{
		LLView* captor_view = (LLView*)mMouseCaptor;
		while( captor_view )
		{
			if( captor_view == parent )
			{
				return TRUE;
			}
			captor_view = captor_view->getParent();
		}
	}
	return FALSE;
}

void LLFocusMgr::removeKeyboardFocusWithoutCallback( LLView* focus )
{
	// should be ok to unlock here, as you have to know the locked view
	// in order to unlock it
	if (focus == mLockedView)
	{
		mLockedView = NULL;
		mKeyboardLockedFocusLostCallback = NULL;
	}

	if( mKeyboardFocus == focus )
	{
		mKeyboardFocus = NULL;
		mKeyboardFocusLostCallback = NULL;
		#ifdef _DEBUG
			mKeyboardFocusName = "none";
		#endif
	}
}


void LLFocusMgr::setMouseCapture( LLMouseHandler* new_captor )
{
	//if (mFocusLocked)
	//{
	//	return;
	//}

	if( new_captor != mMouseCaptor )
	{
		LLMouseHandler* old_captor = mMouseCaptor;
		mMouseCaptor = new_captor;
		/*
		if (new_captor)
		{
			if ( new_captor->getName() == "Stickto")
			{
				llinfos << "New mouse captor: " << new_captor->getName() << llendl;
			}
			else
			{
				llinfos << "New mouse captor: " << new_captor->getName() << llendl;
			}
		}
		else
		{
			llinfos << "New mouse captor: NULL" << llendl;
		}
		*/

		if( old_captor )
		{
			old_captor->onMouseCaptureLost();
		}

		#ifdef _DEBUG
			mMouseCaptorName = new_captor ? new_captor->getName() : "none";
		#endif
	}
}

void LLFocusMgr::removeMouseCaptureWithoutCallback( LLMouseHandler* captor )
{
	//if (mFocusLocked)
	//{
	//	return;
	//}
	if( mMouseCaptor == captor )
	{
		mMouseCaptor = NULL;
		#ifdef _DEBUG
			mMouseCaptorName = "none";
		#endif
	}
}


BOOL LLFocusMgr::childIsTopCtrl( LLView* parent )
{
	LLView* top_view = (LLView*)mTopCtrl;
	while( top_view )
	{
		if( top_view == parent )
		{
			return TRUE;
		}
		top_view = top_view->getParent();
	}
	return FALSE;
}



// set new_top = NULL to release top_view.
void LLFocusMgr::setTopCtrl( LLUICtrl* new_top  )
{
	if( new_top != mTopCtrl )
	{
		mTopCtrl = new_top;

		#ifdef _DEBUG
			mTopCtrlName = new_top ? new_top->getName() : "none";
		#endif
	}
}

void LLFocusMgr::removeTopCtrlWithoutCallback( LLUICtrl* top_view )
{
	if( mTopCtrl == top_view )
	{
		mTopCtrl = NULL;
		#ifdef _DEBUG
			mTopCtrlName = "none";
		#endif
	}
}

void LLFocusMgr::lockFocus()
{
	mLockedView = mKeyboardFocus; 
	mKeyboardLockedFocusLostCallback = mKeyboardFocusLostCallback; 
}

void LLFocusMgr::unlockFocus()
{
	mLockedView = NULL; 
	mKeyboardLockedFocusLostCallback = NULL;
}

F32 LLFocusMgr::getFocusFlashAmt()
{
	return clamp_rescale(getFocusTime(), 0.f, FOCUS_FADE_TIME, mFocusWeight, 0.f);
}

LLColor4 LLFocusMgr::getFocusColor()
{
	LLColor4 focus_color = lerp(LLUI::sColorsGroup->getColor( "FocusColor" ), LLColor4::white, getFocusFlashAmt());
	// de-emphasize keyboard focus when app has lost focus (to avoid typing into wrong window problem)
	if (!mAppHasFocus)
	{
		focus_color.mV[VALPHA] *= 0.4f;
	}
	return focus_color;
}

void LLFocusMgr::triggerFocusFlash()
{
	mFocusTimer.reset();
	mFocusWeight = 1.f;
}

void LLFocusMgr::setAppHasFocus(BOOL focus) 
{ 
	if (!mAppHasFocus && focus)
	{
		triggerFocusFlash();
	}
	
	// release focus from "top ctrl"s, which generally hides them
	if (!focus && mTopCtrl && mTopCtrl->hasFocus())
	{
		mTopCtrl->setFocus(FALSE);
	}
	mAppHasFocus = focus; 
}

LLUICtrl* LLFocusMgr::getLastFocusForGroup(LLView* subtree_root)
{
	if (subtree_root)
	{
		focus_history_map_t::iterator found_it = mFocusHistory.find(subtree_root->mViewHandle);
		if (found_it != mFocusHistory.end())
		{
			// found last focus for this subtree
			return static_cast<LLUICtrl*>(LLView::getViewByHandle(found_it->second));
		}
	}
	return NULL;
}

void LLFocusMgr::clearLastFocusForGroup(LLView* subtree_root)
{
	if (subtree_root)
	{
		mFocusHistory.erase(subtree_root->mViewHandle);
	}
}
