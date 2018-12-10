#include "CSkyScreensaver.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QWindow>
#include <QBoxLayout>
#include <QKeyEvent>

// Animation transition params

constexpr float kLowZoom = 45;
constexpr float kHighZoom = 60;
constexpr int kLowX = -25;
constexpr int kHighX = 25;
constexpr int kMaxZ = 30;
constexpr float kMaxY = 0.25;
constexpr float kRSpeedBound = 0.05;


QVector<QString> kCubeImages =
{
	"img/posx.jpg",
	"img/negx.jpg",
	"img/posy.jpg",
	"img/negy.jpg",
	"img/posz.jpg",
	"img/negz.jpg"
};


//
//	class CSkyScreensaver
//

CSkyScreensaver::CSkyScreensaver ()
	: QWidget (),
	  mRandomG (QRandomGenerator::securelySeeded ()),
	  mX (0.0f),
	  mY (0.0f),
	  mZ (0.0f),
	  mZoom (60)
{
	QVBoxLayout *wLayout = new QVBoxLayout (this);
	wLayout->setMargin (0);
	mGlWidget = new CGLSkyCube (kCubeImages);
	wLayout->addWidget (mGlWidget);

	setWindowFlags (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	setCursor (Qt::BlankCursor);

	connect (&mTimer, SIGNAL (timeout ()), this, SLOT (onTimer ()));
	mTimer.setSingleShot (false);
	mTimer.setInterval (10);

	mXTrans.setDeltas (-20, -20);
	mZTrans.setDeltas (-20, -20);
}


CSkyScreensaver::~CSkyScreensaver ()
{
}


void
CSkyScreensaver::closeEvent (QCloseEvent */*event*/)
{
	hide ();
}


void
CSkyScreensaver::mousePressEvent (QMouseEvent *event)
{
	// Deactivate on mouse click

	deactivate ();

	QWidget::mousePressEvent (event);
}


void
CSkyScreensaver::keyPressEvent (QKeyEvent *event)
{
	// Deactivate on 'escape'

	if (event->key () == Qt::Key_Escape)
	{
		deactivate ();
	}

	QWidget::keyPressEvent (event);
}


void
CSkyScreensaver::hideEvent (QHideEvent *event)
{
	mTimer.stop ();

	QWidget::hideEvent (event);
}


void
CSkyScreensaver::activate ()
{
	show ();
	raise ();

	QWindow *w = windowHandle ();
	if (w != nullptr)
	{
		// bring window to front/unminimize on windows
		w->requestActivate ();
	}

	move (0, 0);
	resize (QApplication::desktop ()->screenGeometry ().size ());

	setWindowState (Qt::WindowFullScreen);
	mTimer.start ();
}


void
CSkyScreensaver::deactivate ()
{
	hide ();
}


void
CSkyScreensaver::onTimer ()
{
	if (not isVisible ())
	{
		return;
	}

	if (mXTrans.isDone ())
	{
		mXTrans.setTransition (mX, mRandomG.bounded (kLowX, kHighX), mRandomG.bounded (500, 1500));
	}

	if (mZTrans.isDone ())
	{
		mZTrans.setTransition (mZ, mRandomG.bounded (-kMaxZ, kMaxZ), mRandomG.bounded (800, 2500));
	}

	mX = mXTrans.nextValue ();
	mZ = mZTrans.nextValue ();

	double rSpeed = kMaxY * (( (kHighX - mX)/(double)(kHighX - kLowX) ) * std::abs (mZ) / (double)kMaxZ);
	mY += (mZ > 0) ? rSpeed : -rSpeed;

	if (mZoomTrans.isDone ())
	{
		if (2 * mZoom > kLowZoom + kHighZoom)
		{
			mZoomTrans.setTransition (mZoom, kLowZoom, 1000);
		}
		else
		{
			mZoomTrans.setTransition (mZoom, kHighZoom, 1500);
		}
	}

	mZoom = mZoomTrans.nextValue ();

	mGlWidget->setPosition (mX, mY, mZ, mZoom);
}
