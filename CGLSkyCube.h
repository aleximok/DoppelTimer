#ifndef CGLSKYCUBE_H
#define CGLSKYCUBE_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>

class QOpenGLShaderProgram;
class QOpenGLTexture;

//
//	class CGLSkyCube
//

class CGLSkyCube : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:

	explicit CGLSkyCube (QVector<QString> &inImageFiles, QWidget *parent = nullptr);
	virtual ~CGLSkyCube ();

	QSize minimumSizeHint () const;

	void rotateBy (float xAngle, float yAngle, float zAngle);
	void setPosition (float xAngle, float yAngle, float zAngle, float inZoom);
	void setZoom (float inZoom);

signals:

	void clicked ();

protected:

	void initializeGL ();
	void paintGL ();

	void setTextures ();

protected:

	QPoint	mLastPos;
	float		mXAngle;
	float		mYAngle;
	float		mZAngle;
	float		mZoom;

	QVector<QString> &mImageFiles;

	QOpenGLTexture *mTexture;
	QOpenGLShaderProgram *mShader;
	QOpenGLBuffer mVbo;
};

#endif // CGLSKYCUBE_H
