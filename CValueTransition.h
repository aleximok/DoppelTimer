#ifndef CVALUETRANSITION_H
#define CVALUETRANSITION_H

//
//	class CValueTransition
//

class CValueTransition
{
public:

	constexpr static int kDefaultSteps		= 100;			// default animation steps
	constexpr static int kMaxDelta			= 20;			// max percent of delta start/finish sin (x+/-d) shift

	CValueTransition ();

	CValueTransition (double inStartValue, double inFinishValue, int inSteps = kDefaultSteps,
			int inLeftDelta = 0, int inRightDelta = 0);

	void setTransition (double inStartValue, double inFinishValue, int inSteps,
			int inLeftDelta = 0, int inRightDelta = 0);

	void setTransition (double inStartValue, double inFinishValue);
	void setSteps (int inSteps);

	// Negative delta smoothes start/finish of animation, positive make it sharper
	void setDeltas (int inLeftDelta, int inRightDelta);

	double currentValue () const;
	double nextValue ();

	void resetSteps ()
	{
		mCurrentStep = 0;
	}

	bool isDone () const
	{
		return mCurrentStep >= mSteps;
	}

protected:

	double		mStartValue;
	double		mFinishValue;

	double		mLeftDelta;
	double		mRightDelta;

	int				mSteps;
	int				mCurrentStep;

	double		mLeftX;
	double		mRightX;
	double		mLeftSX;
	double		mRightSX;
};

#endif // CVALUETRANSITION_H
