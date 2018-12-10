#ifndef CSKYSCREENSAVER_H
#define CSKYSCREENSAVER_H

#include <QWidget>
#include <QTimer>
#include <QRandomGenerator>

#include "CGLSkyCube.h"
#include "CValueTransition.h"

//
//	class CSkyScreensaver
//

class CSkyScreensaver : public QWidget
{
	Q_OBJECT

public:

	CSkyScreensaver ();
	virtual ~CSkyScreensaver ();

protected:

	virtual void closeEvent (QCloseEvent *event);
	virtual void mousePressEvent (QMouseEvent *event);
	virtual void keyPressEvent (QKeyEvent *event);

	virtual void hideEvent (QHideEvent *event);

public slots:

	void activate ();
	void deactivate ();

	void onTimer ();

protected:

	CGLSkyCube *mGlWidget;
	QTimer		mTimer;
	QRandomGenerator	mRandomG;

	float							mX, mY, mZ, mZoom;
	CValueTransition		mZoomTrans, mXTrans, mZTrans;
};


#endif // CSKYSCREENSAVER_H
