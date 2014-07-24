// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QTime>

#include "VideoRenderer.h"

using namespace OrientView;

VideoRenderer::VideoRenderer()
{
}

bool VideoRenderer::initialize(int videoWidth, int videoHeight)
{
	qDebug("Initializing VideoRenderer");

	this->videoWidth = videoWidth;
	this->videoHeight = videoHeight;

	initializeOpenGLFunctions();

	qDebug("Compiling shaders");

	shaderProgram = std::unique_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram());

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, "data/shaders/basic.vert"))
		return false;

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, "data/shaders/basic.frag"))
		return false;

	if (!shaderProgram->link())
		return false;

	if ((vertexMatrixUniform = shaderProgram->uniformLocation("vertexMatrix")) == -1)
	{
		qWarning("Could not find vertexMatrix uniform");
		return false;
	}

	if ((textureMatrixUniform = shaderProgram->uniformLocation("textureMatrix")) == -1)
	{
		qWarning("Could not find textureMatrix uniform");
		return false;
	}

	if ((textureSamplerUniform = shaderProgram->uniformLocation("textureSampler")) == -1)
	{
		qWarning("Could not find textureSampler uniform");
		return false;
	}

	if ((vertexCoordAttribute = shaderProgram->attributeLocation("vertexCoord")) == -1)
	{
		qWarning("Could not find vertexCoord attribute");
		return false;
	}

	if ((textureCoordAttribute = shaderProgram->attributeLocation("textureCoord")) == -1)
	{
		qWarning("Could not find textureCoord attribute");
		return false;
	}

	videoPanelBuffer = std::unique_ptr<QOpenGLBuffer>(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
	videoPanelBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
	videoPanelBuffer->create();
	videoPanelBuffer->bind();
	videoPanelBuffer->allocate(sizeof(GLfloat) * 16);
	videoPanelBuffer->release();

	videoPanelTexture = std::unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QOpenGLTexture::Target2D));
	videoPanelTexture->create();
	videoPanelTexture->bind();
	videoPanelTexture->setSize(videoWidth, videoHeight);
	videoPanelTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
	videoPanelTexture->allocateStorage();
	videoPanelTexture->release();

	mapPanelBuffer = std::unique_ptr<QOpenGLBuffer>(new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer));
	mapPanelBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
	mapPanelBuffer->create();
	mapPanelBuffer->bind();
	mapPanelBuffer->allocate(sizeof(GLfloat) * 16);
	mapPanelBuffer->release();

	mapPanelTexture = std::unique_ptr<QOpenGLTexture>(new QOpenGLTexture(QImage("map.jpg")));
	mapPanelTexture->setMinificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setMagnificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
	mapPanelTexture->release();

	return true;
}

void VideoRenderer::shutdown()
{
	qDebug("Shutting down VideoRenderer");

	if (videoPanelTexture != nullptr)
		videoPanelTexture.reset(nullptr);

	if (mapPanelTexture != nullptr)
		mapPanelTexture.reset(nullptr);

	if (mapPanelBuffer != nullptr)
		mapPanelBuffer.reset(nullptr);

	if (videoPanelBuffer != nullptr)
		videoPanelBuffer.reset(nullptr);

	if (shaderProgram != nullptr)
		shaderProgram.reset(nullptr);
}

void VideoRenderer::update(int windowWidth, int windowHeight)
{
	float videoPanelTop = 1.0f;
	float videoPanelBottom = 0.0f;
	float videoPanelLeft = 0.0f;

	// try to fit the video panel on the screen as big as possible and keep the video aspect ratio
	float videoAspectRatio = (float)videoWidth / videoHeight;
	float newVideoHeight = windowWidth / videoAspectRatio;
	float newVideoWidth = windowHeight * videoAspectRatio;

	if (newVideoHeight < windowHeight)
		videoPanelBottom = 1.0f - (newVideoHeight / windowHeight);
	else
		videoPanelLeft = 1.0f - (newVideoWidth / windowWidth);

	// center video panel vertically
	float halfFromBottom = videoPanelBottom / 2.0f;
	videoPanelBottom -= halfFromBottom;
	videoPanelTop -= halfFromBottom;

	GLfloat videoPanelBufferData[] =
	{
		videoPanelLeft, videoPanelBottom,
		1.0f, videoPanelBottom,
		1.0f, videoPanelTop,
		videoPanelLeft, videoPanelTop,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	videoPanelBuffer->bind();
	videoPanelBuffer->write(0, videoPanelBufferData, sizeof(GLfloat) * 16);
	videoPanelBuffer->release();

	GLfloat mapPanelBufferData[] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	mapPanelBuffer->bind();
	mapPanelBuffer->write(0, mapPanelBufferData, sizeof(GLfloat) * 16);
	mapPanelBuffer->release();

	vertexMatrix.setToIdentity();
	textureMatrix.setToIdentity();

	vertexMatrix.ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
}

void VideoRenderer::render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shaderProgram->bind();

	shaderProgram->setUniformValue(vertexMatrixUniform, vertexMatrix);
	shaderProgram->setUniformValue(textureMatrixUniform, textureMatrix);
	shaderProgram->setUniformValue(textureSamplerUniform, 0);

	videoPanelBuffer->bind();
	videoPanelTexture->bind();

	glVertexAttribPointer(vertexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(textureCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 8));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	videoPanelBuffer->release();
	videoPanelTexture->release();

	shaderProgram->release();
}

QOpenGLTexture* VideoRenderer::getVideoPanelTexture()
{
	return videoPanelTexture.get();
}
