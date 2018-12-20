#include "CGLSkyCube.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QApplication>
#include <QDir>

//
//	class CGLSkyCube
//

constexpr char sVShaderSource [] =
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"out vec3 TexCoords;\n"
	"uniform mat4 projection;\n"
	"uniform mat4 view;\n"
	"void main()\n"
	"{\n"
	"	TexCoords = aPos;\n"
	"	vec4 pos = projection * view * vec4(aPos, 1.0);\n"
	"	gl_Position = pos.xyww;\n"
	"}\n";

constexpr char sFShaderSource [] =
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"in vec3 TexCoords;\n"
	"uniform samplerCube skybox;\n"
	"void main()\n"
	"{\n"
	"	FragColor = texture(skybox, TexCoords);\n"
	"}\n";


CGLSkyCube::CGLSkyCube (QVector<QString> &inImageFiles, QWidget *parent) :
	QOpenGLWidget (parent),
	mXAngle (0.0f),
	mYAngle (0.0f),
	mZAngle (0.0f),
	mZoom (60.0f),
	mImageFiles (inImageFiles)
{
	Q_ASSERT (mImageFiles.size () == 6);
}


CGLSkyCube::~CGLSkyCube ()
{
	makeCurrent ();
	mVbo.destroy ();
	mTexture.reset (nullptr);
	mShader.reset (nullptr);
	doneCurrent ();
}


QSize
CGLSkyCube::minimumSizeHint () const
{
	return QSize (100, 100);
}


void
CGLSkyCube::rotateBy (float xAngle, float yAngle, float zAngle)
{
	mXAngle += xAngle;
	mYAngle += yAngle;
	mZAngle += zAngle;

	update ();
}


void
CGLSkyCube::setPosition (float xAngle, float yAngle, float zAngle, float inZoom)
{
	mXAngle = xAngle;
	mYAngle = yAngle;
	mZAngle = zAngle;
	mZoom = inZoom;

	update ();
}


void
CGLSkyCube::setZoom (float inZoom)
{
	mZoom = inZoom;

	update ();
}


void
CGLSkyCube::initializeGL ()
{
	initializeOpenGLFunctions ();

	mShader = std::make_unique<QOpenGLShaderProgram>();

	QOpenGLShader *vShader = new QOpenGLShader (QOpenGLShader::Vertex, this);
	vShader->compileSourceCode(sVShaderSource);

	QOpenGLShader *fShader = new QOpenGLShader (QOpenGLShader::Fragment, this);
	fShader->compileSourceCode (sFShaderSource);

	mShader->addShader (vShader);
	mShader->addShader (fShader);
	mShader->bindAttributeLocation ("TexCoords", 0);
	mShader->link ();

	mShader->bind ();
	mShader->setUniformValue ("skybox", 0);

	setTextures ();
}


void
CGLSkyCube::paintGL ()
{
	glClearColor (0.1f, 0.1f, 0.1f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	QMatrix4x4 view;
	view.rotate (mXAngle, 1.0f, 0.0f, 0.0f);
	view.rotate (mZAngle, 0.0f, 0.0f, 1.0f);
	view.rotate (mYAngle, 0.0f, 1.0f, 0.0f);

	QMatrix4x4 projection;
	projection.perspective (mZoom, width () / (float) height (), 0.1f, 100.0f);

	mShader->setUniformValue ("view", view);
	mShader->setUniformValue ("projection", projection);
	mShader->enableAttributeArray (0);
	mShader->setAttributeBuffer (0, GL_FLOAT, 0, 3, 3 * sizeof (GLfloat));

	glDepthFunc (GL_LEQUAL);
	glActiveTexture (GL_TEXTURE0);
	mTexture->bind ();
	glDrawArrays (GL_TRIANGLES, 0, 36);
	glDepthFunc (GL_LESS);
}


void
CGLSkyCube::setTextures ()
{
	GLfloat skyboxVertices [] =
	{
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// +X (right)
	// -X (left)
	// +Y (top)
	// -Y (bottom)
	// +Z (front)
	// -Z (back)

	QDir dir (QCoreApplication::applicationDirPath ());
	QImage imgPx  = QImage (QFileInfo (dir, mImageFiles [0]).absoluteFilePath ()).
			convertToFormat (QImage::Format_RGBA8888);

	mTexture = std::make_unique<QOpenGLTexture> (QOpenGLTexture::TargetCubeMap);
	mTexture->create ();
	mTexture->setSize (imgPx.width (), imgPx.height (), imgPx.depth ());
	mTexture->setFormat (QOpenGLTexture::RGBA8_UNorm);
	mTexture->allocateStorage ();

	mTexture->setData (0, 0, QOpenGLTexture::CubeMapPositiveX,
		QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void*) imgPx.constBits (), 0);

	for (int i = 1; i < 6; i++)
	{
		QImage img = QImage (QFileInfo (dir, mImageFiles [i]).absoluteFilePath ())	.
				convertToFormat (QImage::Format_RGBA8888);

		mTexture->setData (0, 0, (QOpenGLTexture::CubeMapFace)(QOpenGLTexture::CubeMapPositiveX + i),
			QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, (const void*) img.constBits (), 0);
	}

	mTexture->setWrapMode (QOpenGLTexture::ClampToEdge);
	mTexture->setMinificationFilter (QOpenGLTexture::LinearMipMapLinear);
	mTexture->setMagnificationFilter (QOpenGLTexture::LinearMipMapLinear);

	mVbo.create ();
	mVbo.bind ();
	mVbo.allocate (&skyboxVertices[0], sizeof (skyboxVertices) /*36 * 3 * sizeof (GLfloat)*/);
}
