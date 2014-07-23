// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QTime>

#include "VideoRenderer.h"

using namespace OrientView;

VideoRenderer::VideoRenderer()
{
}

VideoRenderer::~VideoRenderer()
{
	shutdown();
}

bool VideoRenderer::initialize()
{
	qDebug("Initializing VideoRenderer");

	initializeOpenGLFunctions();

	qDebug("Compiling shaders");

	shaderProgram = std::unique_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram());

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/basic2.vert"))
		return false;

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/basic2.frag"))
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

void VideoRenderer::shutdown()
{
	qDebug("Shutting down VideoRenderer");

	if (shaderProgram != nullptr)
		shaderProgram.reset(nullptr);
}

void VideoRenderer::render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
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
}
