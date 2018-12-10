#include "CValueTransition.h"

#include <qmath.h>


//
//	class CValueTransition
//

CValueTransition::CValueTransition () :
	mStartValue (0.0),
	mFinishValue (0.0),
	mLeftDelta (0),
	mRightDelta (0),
	mSteps (kDefaultSteps),
	mCurrentStep (kDefaultSteps)
{
}


CValueTransition::CValueTransition (double inStartValue, double inFinishValue,
		int inSteps, int inLeftDelta, int inRightDelta) :
	mStartValue (inStartValue),
	mFinishValue (inFinishValue),
	mLeftDelta (inLeftDelta),
	mRightDelta (inRightDelta),
	mSteps (inSteps),
	mCurrentStep (0)
{
	Q_ASSERT (inSteps > 0);

	mLeftX = -M_PI_2;
	mRightX = M_PI_2;
	mLeftSX = -1;
	mRightSX = 1;
}


void
CValueTransition::setTransition (double inStartValue, double inFinishValue,
		int inSteps, int inLeftDelta, int inRightDelta)
{
	Q_ASSERT (inSteps > 0  &&  inLeftDelta >= -kMaxDelta  &&  inLeftDelta <= kMaxDelta  &&
			  inRightDelta >= -kMaxDelta  &&  inRightDelta <= kMaxDelta);

	mStartValue = inStartValue;
	mFinishValue = inFinishValue;
	mSteps = inSteps;
	mLeftDelta = inLeftDelta;
	mRightDelta = inRightDelta;

	mLeftX = -M_PI_2 + M_PI_2 * mLeftDelta/100.0;
	mRightX = M_PI_2 - M_PI_2 * mRightDelta/100.0;
	mLeftSX = qFastSin (mLeftX);
	mRightSX = qFastSin (mRightX);

	resetSteps ();
}


void
CValueTransition::setTransition (double inStartValue, double inFinishValue)
{
	mStartValue = inStartValue;
	mFinishValue = inFinishValue;

	resetSteps ();
}


void
CValueTransition::setSteps (int inSteps)
{
	Q_ASSERT (inSteps > 0);
	inSteps = (inSteps <= 0) ? kDefaultSteps : inSteps;

	resetSteps ();
}


void
CValueTransition::setDeltas (int inLeftDelta, int inRightDelta)
{
	Q_ASSERT (inLeftDelta >= -kMaxDelta  &&  inLeftDelta <= kMaxDelta  &&
			  inRightDelta >= -kMaxDelta  &&  inRightDelta <= kMaxDelta);

	mLeftDelta = inLeftDelta;
	mRightDelta = inRightDelta;

	mLeftX = -M_PI_2 + M_PI_2 * mLeftDelta/100.0;
	mRightX = M_PI_2 - M_PI_2 * mRightDelta/100.0;
	mLeftSX = qFastSin (mLeftX);
	mRightSX = qFastSin (mRightX);
}


double
CValueTransition::currentValue () const
{
	if (std::abs (mStartValue - mFinishValue) < 0.00001)
	{
		return mStartValue;		// no transition, just line
	}

	return mStartValue + (mFinishValue - mStartValue) *
			(qFastSin (mLeftX + (mRightX - mLeftX) * mCurrentStep / (mSteps - 1)) - mLeftSX) / (mRightSX - mLeftSX) ;
}


double
CValueTransition::nextValue ()
{
	double v = currentValue ();
	mCurrentStep ++;

	return v;
}
