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

	videoPanel.textureWidth = videoDecoder->getVideoInfo().frameWidth;
	videoPanel.textureHeight = videoDecoder->getVideoInfo().frameHeight;
	videoPanel.texelWidth = 1.0 / videoPanel.textureWidth;
	videoPanel.texelHeight = 1.0 / videoPanel.textureHeight;
	mapPanel.textureWidth = quickRouteJpegReader->getMapImage().width();
	mapPanel.textureHeight = quickRouteJpegReader->getMapImage().height();
	mapPanel.texelWidth = 1.0 / mapPanel.textureWidth;
	mapPanel.texelHeight = 1.0 / mapPanel.textureHeight;

	mapPanelRelativeWidth = settings->appearance.mapPanelWidth;
	mapPanelScale = 1.0;
	mapPanelX = 0.0;
	mapPanelY = 0.0;
	averageRenderTime = 0.0;

	initializeOpenGLFunctions();

	if (!loadShaders(&videoPanel, settings->shaders.videoPanelShader))
		return false;

	if (!loadShaders(&mapPanel, settings->shaders.mapPanelShader))
		return false;

	// 1 2
	// 4 3
	GLfloat videoPanelBuffer[] =
	{
		-(float)videoPanel.textureWidth / 2, (float)videoPanel.textureHeight / 2, 0.0f, // 1
		(float)videoPanel.textureWidth / 2, (float)videoPanel.textureHeight / 2, 0.0f, // 2
		(float)videoPanel.textureWidth / 2, -(float)videoPanel.textureHeight / 2, 0.0f, // 3
		-(float)videoPanel.textureWidth / 2, -(float)videoPanel.textureHeight / 2, 0.0f, // 4

		0.0f, 0.0f, // 1
		1.0f, 0.0f, // 2
		1.0f, 1.0f, // 3
		0.0f, 1.0f  // 4
	};

	// 1 2
	// 4 3
	GLfloat mapPanelBuffer[] =
	{
		-(float)mapPanel.textureWidth / 2, (float)mapPanel.textureHeight / 2, 0.0f, // 1
		(float)mapPanel.textureWidth / 2, (float)mapPanel.textureHeight / 2, 0.0f, // 2
		(float)mapPanel.textureWidth / 2, -(float)mapPanel.textureHeight / 2, 0.0f, // 3
		-(float)mapPanel.textureWidth / 2, -(float)mapPanel.textureHeight / 2, 0.0f, // 4

		0.0f, 0.0f, // 1
		1.0f, 0.0f, // 2
		1.0f, 1.0f, // 3
		0.0f, 1.0f  // 4
	};

	loadBuffer(&videoPanel, videoPanelBuffer, 20);
	loadBuffer(&mapPanel, mapPanelBuffer, 20);

	videoPanel.texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	videoPanel.texture->create();
	videoPanel.texture->bind();
	videoPanel.texture->setSize(videoPanel.textureWidth, videoPanel.textureHeight);
	videoPanel.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
	videoPanel.texture->setMinificationFilter(QOpenGLTexture::Linear);
	videoPanel.texture->setMagnificationFilter(QOpenGLTexture::Linear);
	videoPanel.texture->setWrapMode(QOpenGLTexture::ClampToEdge);
	videoPanel.texture->allocateStorage();
	videoPanel.texture->release();

	mapPanel.texture = new QOpenGLTexture(quickRouteJpegReader->getMapImage());
	mapPanel.texture->bind();
	mapPanel.texture->setMinificationFilter(QOpenGLTexture::Linear);
	mapPanel.texture->setMagnificationFilter(QOpenGLTexture::Linear);
	mapPanel.texture->setWrapMode(QOpenGLTexture::ClampToEdge);
	mapPanel.texture->release();

	paintDevice = new QOpenGLPaintDevice();
	painter = new QPainter();

	return true;
}

bool VideoRenderer::loadShaders(Panel* panel, const QString& shaderName)
{
	panel->program = new QOpenGLShaderProgram();

	if (!panel->program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString("data/shaders/%1.vert").arg(shaderName)))
		return false;

	if (!panel->program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString("data/shaders/%1.frag").arg(shaderName)))
		return false;

	if (!panel->program->link())
		return false;

	if ((panel->vertexMatrixUniform = panel->program->uniformLocation("vertexMatrix")) == -1)
		qWarning("Could not find vertexMatrix uniform");

	if ((panel->vertexPositionAttribute = panel->program->attributeLocation("vertexPosition")) == -1)
		qWarning("Could not find vertexPosition attribute");

	if ((panel->vertexTextureCoordinateAttribute = panel->program->attributeLocation("vertexTextureCoordinate")) == -1)
		qWarning("Could not find vertexTextureCoordinate attribute");

	if ((panel->textureSamplerUniform = panel->program->uniformLocation("textureSampler")) == -1)
		qWarning("Could not find textureSampler uniform");

	panel->textureWidthUniform = panel->program->uniformLocation("textureWidth");
	panel->textureHeightUniform = panel->program->uniformLocation("textureHeight");
	panel->texelWidthUniform = panel->program->uniformLocation("texelWidth");
	panel->texelHeightUniform = panel->program->uniformLocation("texelHeight");

	return true;
}

void VideoRenderer::loadBuffer(Panel* panel, GLfloat* buffer, int size)
{
	panel->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	panel->buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	panel->buffer->create();
	panel->buffer->bind();
	panel->buffer->allocate(buffer, sizeof(GLfloat) * size);
	panel->buffer->release();
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

	if (mapPanel.texture != nullptr)
	{
		delete mapPanel.texture;
		mapPanel.texture = nullptr;
	}

	if (videoPanel.texture != nullptr)
	{
		delete videoPanel.texture;
		videoPanel.texture = nullptr;
	}

	if (mapPanel.buffer != nullptr)
	{
		delete mapPanel.buffer;
		mapPanel.buffer = nullptr;
	}

	if (videoPanel.buffer != nullptr)
	{
		delete videoPanel.buffer;
		videoPanel.buffer = nullptr;
	}

	if (mapPanel.program != nullptr)
	{
		delete mapPanel.program;
		mapPanel.program = nullptr;
	}

	if (videoPanel.program != nullptr)
	{
		delete videoPanel.program;
		videoPanel.program = nullptr;
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

	videoPanel.texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData->data, &options);
}

void VideoRenderer::renderVideoPanel()
{
	videoPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
	{
		videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	}
	else
	{
		//videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);
	}

	videoPanel.vertexMatrix.rotate(-videoStabilizer->getAngle(), 0.0f, 0.0f, 1.0f);
	videoPanel.vertexMatrix.translate(-videoStabilizer->getX() * videoPanel.textureWidth, videoStabilizer->getY() * videoPanel.textureHeight, 0.0f);
	//videoPanel.vertexMatrix.scale(2.0f);

	renderPanel(&videoPanel);
}

void VideoRenderer::renderMapPanel()
{	
	mapPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
	{
		mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	}
	else
	{
		//mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);
	}

	if (videoWindow->keyIsDown(Qt::Key_Q))
		mapPanelScale *= (1.0 + frameTime / 500);

	if (videoWindow->keyIsDown(Qt::Key_A))
		mapPanelScale *= (1.0 - frameTime / 500);

	if (videoWindow->keyIsDown(Qt::Key_Left))
		mapPanelX -= 1.0 * frameTime * (1.0 / mapPanelScale);

	if (videoWindow->keyIsDown(Qt::Key_Right))
		mapPanelX += 1.0 * frameTime * (1.0 / mapPanelScale);

	if (videoWindow->keyIsDown(Qt::Key_Up))
		mapPanelY += 1.0 * frameTime * (1.0 / mapPanelScale);

	if (videoWindow->keyIsDown(Qt::Key_Down))
		mapPanelY -= 1.0 * frameTime * (1.0 / mapPanelScale);

	mapPanel.vertexMatrix.scale(mapPanelScale);
	mapPanel.vertexMatrix.translate(-mapPanelX, -mapPanelY);

	renderPanel(&mapPanel);
}

void VideoRenderer::renderPanel(Panel* panel)
{
	panel->program->bind();
	panel->program->setUniformValue(panel->vertexMatrixUniform, panel->vertexMatrix);
	panel->program->setUniformValue(panel->textureSamplerUniform, 0);
	panel->program->setUniformValue(panel->textureWidthUniform, (float)panel->textureWidth);
	panel->program->setUniformValue(panel->textureHeightUniform, (float)panel->textureHeight);
	panel->program->setUniformValue(panel->texelWidthUniform, (float)panel->texelWidth);
	panel->program->setUniformValue(panel->texelHeightUniform, (float)panel->texelHeight);

	panel->buffer->bind();
	panel->texture->bind();

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(panel->vertexPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(panel->vertexTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(GLfloat) * 12));
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	panel->texture->release();
	panel->buffer->release();
	panel->program->release();
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
