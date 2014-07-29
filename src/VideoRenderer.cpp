// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>

#include "VideoRenderer.h"
#include "VideoDecoder.h"
#include "QuickRouteJpegReader.h"
#include "VideoStabilizer.h"
#include "VideoEncoder.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

VideoRenderer::VideoRenderer()
{
}

bool VideoRenderer::initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, Settings* settings)
{
	qDebug("Initializing VideoRenderer");

	this->videoDecoder = videoDecoder;
	this->videoStabilizer = videoStabilizer;
	this->videoEncoder = videoEncoder;

	this->videoFrameWidth = videoDecoder->getVideoInfo().frameWidth;
	this->videoFrameHeight = videoDecoder->getVideoInfo().frameHeight;
	this->mapImageWidth = quickRouteJpegReader->getMapImage().width();
	this->mapImageHeight = quickRouteJpegReader->getMapImage().height();
	this->mapPanelWidth = settings->appearance.mapPanelWidth;

	averageRenderTime = 0.0;

	initializeOpenGLFunctions();

	qDebug("Compiling shaders");

	shaderProgram = new QOpenGLShaderProgram();

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, settings->shaders.vertexShaderPath))
		return false;

	if (!shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, settings->shaders.fragmentShaderPath))
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

	paintDevice = new QOpenGLPaintDevice();
	painter = new QPainter();

	videoPanelBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	videoPanelBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
	videoPanelBuffer->create();
	videoPanelBuffer->bind();
	videoPanelBuffer->allocate(sizeof(GLfloat) * 16);
	videoPanelBuffer->release();

	videoPanelTexture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	videoPanelTexture->create();
	videoPanelTexture->bind();
	videoPanelTexture->setSize(videoFrameWidth, videoFrameHeight);
	videoPanelTexture->setFormat(QOpenGLTexture::RGBA8_UNorm);
	videoPanelTexture->setMinificationFilter(QOpenGLTexture::Linear);
	videoPanelTexture->setMagnificationFilter(QOpenGLTexture::Linear);
	videoPanelTexture->setWrapMode(QOpenGLTexture::ClampToEdge);
	videoPanelTexture->allocateStorage();
	videoPanelTexture->release();

	mapPanelBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	mapPanelBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
	mapPanelBuffer->create();
	mapPanelBuffer->bind();
	mapPanelBuffer->allocate(sizeof(GLfloat) * 16);
	mapPanelBuffer->release();

	mapPanelTexture = new QOpenGLTexture(quickRouteJpegReader->getMapImage());
	mapPanelTexture->bind();
	mapPanelTexture->setMinificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setMagnificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
	mapPanelTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
	mapPanelTexture->release();

	return true;
}

void VideoRenderer::shutdown()
{
	qDebug("Shutting down VideoRenderer");

	if (mapPanelTexture != nullptr)
	{
		delete mapPanelTexture;
		mapPanelTexture = nullptr;
	}

	if (mapPanelBuffer != nullptr)
	{
		delete mapPanelBuffer;
		mapPanelBuffer = nullptr;
	}

	if (videoPanelTexture != nullptr)
	{
		delete videoPanelTexture;
		videoPanelTexture = nullptr;
	}

	if (videoPanelBuffer != nullptr)
	{
		delete videoPanelBuffer;
		videoPanelBuffer = nullptr;
	}

	if (painter != nullptr)
	{
		delete painter;
		painter = nullptr;
	}

	if (paintDevice != nullptr)
	{
		delete paintDevice;
		paintDevice = nullptr;
	}

	if (shaderProgram != nullptr)
	{
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}

void VideoRenderer::startRendering(int windowWidth, int windowHeight)
{
	renderTimer.restart();

	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void VideoRenderer::uploadFrameData(FrameData* frameData)
{
	QOpenGLPixelTransferOptions options;

	options.setRowLength(frameData->rowLength / 4);
	options.setImageHeight(frameData->height);
	options.setAlignment(1);

	videoPanelTexture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData->data, &options);
}

void VideoRenderer::renderVideoPanel(int windowWidth, int windowHeight)
{
	float mapPanelWidthInverse = 1.0f - mapPanelWidth;
	float scaledWindowWidth = mapPanelWidthInverse * windowWidth;

	float videoPanelTop = 1.0f;
	float videoPanelBottom = 0.0f;
	float videoPanelLeft = mapPanelWidth;
	float videoPanelRight = 1.0f;

	// try to fit the video panel on the screen as big as possible and keep the video aspect ratio
	float videoAspectRatio = (float)videoFrameWidth / videoFrameHeight;
	float newVideoWidth = windowHeight * videoAspectRatio;
	float newVideoHeight = scaledWindowWidth / videoAspectRatio;

	// scale horizontally
	if (newVideoWidth < scaledWindowWidth)
		videoPanelLeft = mapPanelWidth + ((1.0f - (newVideoWidth / scaledWindowWidth)) * mapPanelWidthInverse);

	// scale vertically
	if (newVideoHeight < windowHeight)
		videoPanelBottom = 1.0f - (newVideoHeight / windowHeight);

	// center horizontally
	float halfLeft = (videoPanelLeft - mapPanelWidth) / 2.0f;
	videoPanelLeft -= halfLeft;
	videoPanelRight -= halfLeft;

	// center vertically
	float halfFromBottom = videoPanelBottom / 2.0f;
	videoPanelBottom -= halfFromBottom;
	videoPanelTop -= halfFromBottom;

	GLfloat videoPanelBufferData[] =
	{
		videoPanelLeft, videoPanelBottom,
		videoPanelRight, videoPanelBottom,
		videoPanelRight, videoPanelTop,
		videoPanelLeft, videoPanelTop,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	videoPanelBuffer->bind();
	videoPanelBuffer->write(0, videoPanelBufferData, sizeof(GLfloat) * 16);

	videoPanelVertexMatrix.setToIdentity();
	videoPanelTextureMatrix.setToIdentity();

	if (!flipOutput)
	{
		videoPanelVertexMatrix.ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		videoPanelTextureMatrix.setToIdentity();
	}
	else
	{
		videoPanelVertexMatrix.ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		videoPanelTextureMatrix.setToIdentity();
	}

	//videoPanelVertexMatrix.translate(-videoStabilizer->getX(), videoStabilizer->getY(), 0);
	//videoPanelVertexMatrix.rotate(90.0, 0, 0, 1);

	shaderProgram->bind();
	shaderProgram->setUniformValue(textureSamplerUniform, 0);
	shaderProgram->setUniformValue(vertexMatrixUniform, videoPanelVertexMatrix);
	shaderProgram->setUniformValue(textureMatrixUniform, videoPanelTextureMatrix);

	videoPanelTexture->bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(textureCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 8));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	videoPanelBuffer->release();
	videoPanelTexture->release();
	shaderProgram->release();
}

void VideoRenderer::renderMapPanel(int windowWidth, int windowHeight)
{
	float mapAspectRatio = (float)mapImageWidth / mapImageHeight;
	float mapPanelTextureRight = (mapPanelWidth * windowWidth) / mapImageWidth;
	float mapPanelTextureTop = windowHeight / (float)mapImageHeight;

	GLfloat mapPanelBufferData[] =
	{
		0.0f, 0.0f,
		mapPanelWidth, 0.0f,
		mapPanelWidth, 1.0f,
		0.0f, 1.0f,

		0.0f, mapPanelTextureTop,
		mapPanelTextureRight, mapPanelTextureTop,
		mapPanelTextureRight, 0.0f,
		0.0f, 0.0f
	};

	mapPanelBuffer->bind();
	mapPanelBuffer->write(0, mapPanelBufferData, sizeof(GLfloat) * 16);

	mapPanelVertexMatrix.setToIdentity();
	mapPanelTextureMatrix.setToIdentity();

	if (!flipOutput)
	{
		mapPanelVertexMatrix.ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
		mapPanelTextureMatrix.setToIdentity();
	}
	else
	{
		mapPanelVertexMatrix.ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
		mapPanelTextureMatrix.setToIdentity();
	}

	shaderProgram->bind();
	shaderProgram->setUniformValue(textureSamplerUniform, 0);
	shaderProgram->setUniformValue(vertexMatrixUniform, mapPanelVertexMatrix);
	shaderProgram->setUniformValue(textureMatrixUniform, mapPanelTextureMatrix);

	mapPanelTexture->bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertexCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(textureCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 8));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	mapPanelBuffer->release();
	mapPanelTexture->release();
	shaderProgram->release();
}

void VideoRenderer::renderInfoPanel(int windowWidth, int windowHeight, double frameTime, double spareTime)
{
	paintDevice->setSize(QSize(windowWidth, windowHeight));
	painter->begin(paintDevice);

	int textX = 10;
	int textY = 6;
	int lineHeight = 17;
	int textWidth = 170;
	int textHeight = 141;

	painter->setPen(QColor(0, 0, 0));
	painter->setBrush(QBrush(QColor(20, 20, 20, 220)));
	painter->drawRoundedRect(-10, -10, textWidth, textHeight, 10, 10);

	painter->setPen(QColor(255, 255, 255, 200));
	painter->setFont(QFont("DejaVu Sans", 10, QFont::Normal));
	painter->drawText(textX, textY, textWidth, textHeight, 0, "fps:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "frame:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "decode:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "stabilize:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "render:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "encode:");
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, "spare:");

	textX = 80;
	textY = 6;

	painter->drawText(textX, textY, textWidth, textHeight, 0, QString::number(1000.0 / frameTime, 'f', 2));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(frameTime, 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(videoDecoder->getAverageDecodeTime(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(videoStabilizer->getAverageProcessTime(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageRenderTime, 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(videoEncoder->getAverageEncodeTime(), 'f', 2)));

	if (spareTime < 0)
		painter->setPen(QColor(255, 0, 0, 200));
	else if (spareTime > 0)
		painter->setPen(QColor(0, 255, 0, 200));

	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(spareTime, 'f', 2)));

	painter->end();
}

void VideoRenderer::stopRendering()
{
	averageRenderTime = renderTimer.nsecsElapsed() / 1000000.0;
}

void VideoRenderer::setFlipOutput(bool value)
{
	paintDevice->setPaintFlipped(value);
	flipOutput = value;
}
