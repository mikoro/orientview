// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QTime>
#include <QTimer>
#include <QThread>

#include "VideoWindow.h"

using namespace OrientView;

namespace
{
	class WorkThread : public QThread
	{
	public:

		WorkThread(VideoWindow& videoWindow) : videoWindow(videoWindow)
		{
		}

		void run() 
		{
			while (true)
			{
				videoWindow.renderOnScreen();
				QThread::msleep(25);
			}
		}

		VideoWindow& videoWindow;
	};
}

VideoWindow::VideoWindow(QWindow* parent) : QWindow(parent)
{
	setSurfaceType(QWindow::OpenGLSurface);
	setTitle("OrientView - Video");
	resize(1280, 720);
	setModality(Qt::ApplicationModal);
	//setVisibility(QWindow::Visibility::Minimized);
	setIcon(QIcon(":/MainView/misc/orientview.ico"));
}

bool VideoWindow::initialize()
{
	qDebug("Creating OpenGL context");

	context = std::unique_ptr<QOpenGLContext>(new QOpenGLContext());
	context->setFormat(requestedFormat());

	if (!context->create())
	{
		qWarning("Could not create OpenGL context");
		return false;
	}

	if (!context->makeCurrent(this))
	{
		qWarning("Could not make context current");
		return false;
	}

	initializeOpenGLFunctions();

	qDebug("Compiling shaders");

	shaderProgram = std::unique_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram(this));

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/basic1.vert"))
		return false;

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/basic1.frag"))
		return false;

	if (!shaderProgram->link())
		return false;

	if ((positionAttribute = shaderProgram->attributeLocation("position")) == -1)
		return false;

	if ((colorAttribute = shaderProgram->attributeLocation("color")) == -1)
		return false;

	if ((matrixUniform = shaderProgram->uniformLocation("matrix")) == -1)
		return false;

	return true;
}

void VideoWindow::start()
{
	//WorkThread* workerThread = new WorkThread(*this);
	//connect(workerThread, &WorkThread::finished, workerThread, &QObject::deleteLater);
	//context->doneCurrent();
	//context->moveToThread(workerThread);
	//workerThread->start();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
	timer->start(10);
}

void VideoWindow::timerUpdate()
{
	renderOnScreen();
}

void VideoWindow::renderOnScreen()
{
	if (!isExposed())
		return;
	
	context->makeCurrent(this);

	glViewport(0, 0, width(), height());

	glClear(GL_COLOR_BUFFER_BIT);

	shaderProgram->bind();

	QMatrix4x4 matrix;
	matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	matrix.translate(0, 0, -2);
	matrix.rotate(0.1f * QTime::currentTime().msecsSinceStartOfDay(), 0, 1, 0);

	shaderProgram->setUniformValue(matrixUniform, matrix);

	GLfloat vertices[] = {
		0.0f, 0.707f,
		-0.5f, -0.5f,
		0.5f, -0.5f
	};

	GLfloat colors[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f
	};

	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0, colors);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	shaderProgram->release();

	context->swapBuffers(this);
}
