// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>
#include <QTime>

#include "VideoRenderer.h"
#include "VideoDecoder.h"
#include "QuickRouteJpegReader.h"
#include "VideoStabilizer.h"
#include "VideoEncoder.h"
#include "VideoWindow.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

VideoRenderer::VideoRenderer()
{
}

bool VideoRenderer::initialize(VideoDecoder* videoDecoder, QuickRouteJpegReader* quickRouteJpegReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, VideoWindow* videoWindow, Settings* settings)
{
	qDebug("Initializing VideoRenderer");

	this->videoDecoder = videoDecoder;
	this->videoStabilizer = videoStabilizer;
	this->videoEncoder = videoEncoder;
	this->videoWindow = videoWindow;

	this->videoFrameWidth = videoDecoder->getVideoInfo().frameWidth;
	this->videoFrameHeight = videoDecoder->getVideoInfo().frameHeight;
	this->mapImageWidth = quickRouteJpegReader->getMapImage().width();
	this->mapImageHeight = quickRouteJpegReader->getMapImage().height();
	this->mapPanelRelativeWidth = settings->appearance.mapPanelWidth;

	mapPanelScale = 1.0;
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

	if ((vertexPositionAttribute = shaderProgram->attributeLocation("vertexPosition")) == -1)
	{
		qWarning("Could not find vertexPosition attribute");
		return false;
	}

	if ((vertexTextureCoordinateAttribute = shaderProgram->attributeLocation("vertexTextureCoordinate")) == -1)
	{
		qWarning("Could not find vertexTextureCoordinate attribute");
		return false;
	}

	if ((textureSamplerUniform = shaderProgram->uniformLocation("textureSampler")) == -1)
	{
		qWarning("Could not find textureSampler uniform");
		return false;
	}

	GLfloat videoPanelBufferData[] =
	{
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	videoPanelBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	videoPanelBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	videoPanelBuffer->create();
	videoPanelBuffer->bind();
	videoPanelBuffer->allocate(videoPanelBufferData, sizeof(GLfloat) * 20);
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

	GLfloat mapPanelBufferData[] =
	{
		-(float)mapImageWidth / 2.0f, -(float)mapImageHeight / 2.0f, 0.0f,
		(float)mapImageWidth / 2.0f, -(float)mapImageHeight / 2.0f, 0.0f,
		(float)mapImageWidth / 2.0f, (float)mapImageHeight / 2.0f, 0.0f,
		-(float)mapImageWidth / 2.0f, (float)mapImageHeight / 2.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	mapPanelBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	mapPanelBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	mapPanelBuffer->create();
	mapPanelBuffer->bind();
	mapPanelBuffer->allocate(mapPanelBufferData, sizeof(GLfloat) * 20);
	mapPanelBuffer->release();

	mapPanelTexture = new QOpenGLTexture(quickRouteJpegReader->getMapImage());
	mapPanelTexture->bind();
	mapPanelTexture->setMinificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setMagnificationFilter(QOpenGLTexture::Linear);
	mapPanelTexture->setBorderColor(1.0f, 1.0f, 1.0f, 1.0f);
	mapPanelTexture->setWrapMode(QOpenGLTexture::ClampToBorder);
	mapPanelTexture->release();

	paintDevice = new QOpenGLPaintDevice();
	painter = new QPainter();

	return true;
}

void VideoRenderer::shutdown()
{
	qDebug("Shutting down VideoRenderer");

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

	if (shaderProgram != nullptr)
	{
		delete shaderProgram;
		shaderProgram = nullptr;
	}
}

void VideoRenderer::startRendering(double windowWidth, double windowHeight, double frameTime)
{
	renderTimer.restart();

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->frameTime = frameTime;

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

void VideoRenderer::renderVideoPanel()
{
	videoPanelVertexMatrix.setToIdentity();

	if (!flipOutput)
	{
		videoPanelVertexMatrix.ortho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		videoPanelVertexMatrix.ortho(0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	}

	//videoPanelVertexMatrix.translate(-videoStabilizer->getX(), videoStabilizer->getY(), 0);
	//videoPanelVertexMatrix.rotate(90.0, 0, 0, 1);

	shaderProgram->bind();
	shaderProgram->setUniformValue(textureSamplerUniform, 0);
	shaderProgram->setUniformValue(vertexMatrixUniform, videoPanelVertexMatrix);

	videoPanelBuffer->bind();
	videoPanelTexture->bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(vertexTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 12));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	videoPanelBuffer->release();
	videoPanelTexture->release();
	shaderProgram->release();
}

void VideoRenderer::renderMapPanel()
{	
	mapPanelVertexMatrix.setToIdentity();

	if (!flipOutput)
	{
		mapPanelVertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	}
	else
	{
		mapPanelVertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);
	}

	if (videoWindow->keyIsDown(Qt::Key_Q))
		mapPanelScale *= (1.0 + frameTime / 500);

	if (videoWindow->keyIsDown(Qt::Key_A))
		mapPanelScale *= (1.0 - frameTime / 500);

	mapPanelVertexMatrix.scale(mapPanelScale);

	shaderProgram->bind();
	shaderProgram->setUniformValue(textureSamplerUniform, 0);
	shaderProgram->setUniformValue(vertexMatrixUniform, mapPanelVertexMatrix);

	mapPanelBuffer->bind();
	mapPanelTexture->bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(vertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(vertexTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 12));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	mapPanelBuffer->release();
	mapPanelTexture->release();
	shaderProgram->release();
}

void VideoRenderer::renderInfoPanel(double spareTime)
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
