// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>
#include <QTime>

#include "Renderer.h"
#include "VideoDecoder.h"
#include "MapImageReader.h"
#include "VideoStabilizer.h"
#include "VideoEncoder.h"
#include "VideoWindow.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

Renderer::Renderer()
{
}

bool Renderer::initialize(VideoDecoder* videoDecoder, MapImageReader* mapImageReader, VideoStabilizer* videoStabilizer, VideoEncoder* videoEncoder, VideoWindow* videoWindow, Settings* settings)
{
	qDebug("Initializing Renderer");

	this->videoDecoder = videoDecoder;
	this->videoStabilizer = videoStabilizer;
	this->videoEncoder = videoEncoder;
	this->videoWindow = videoWindow;

	videoPanel = Panel();
	videoPanel.textureWidth = videoDecoder->getVideoInfo().frameWidth;
	videoPanel.textureHeight = videoDecoder->getVideoInfo().frameHeight;
	videoPanel.texelWidth = 1.0 / videoPanel.textureWidth;
	videoPanel.texelHeight = 1.0 / videoPanel.textureHeight;
	videoPanel.userScale = settings->appearance.videoPanelScale;

	mapPanel = Panel();
	mapPanel.textureWidth = mapImageReader->getMapImage().width();
	mapPanel.textureHeight = mapImageReader->getMapImage().height();
	mapPanel.texelWidth = 1.0 / mapPanel.textureWidth;
	mapPanel.texelHeight = 1.0 / mapPanel.textureHeight;

	selectedPanelPtr = &videoPanel;
	mapPanelRelativeWidth = settings->appearance.mapPanelWidth;

	const double movingAverageAlpha = 0.1;
	averageFps.reset();
	averageFps.setAlpha(movingAverageAlpha);
	averageFrameTime.reset();
	averageFrameTime.setAlpha(movingAverageAlpha);
	averageDecodeTime.reset();
	averageDecodeTime.setAlpha(movingAverageAlpha);
	averageStabilizeTime.reset();
	averageStabilizeTime.setAlpha(movingAverageAlpha);
	averageRenderTime.reset();
	averageRenderTime.setAlpha(movingAverageAlpha);
	averageEncodeTime.reset();
	averageEncodeTime.setAlpha(movingAverageAlpha);
	averageSpareTime.reset();
	averageSpareTime.setAlpha(movingAverageAlpha);

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

	mapPanel.texture = new QOpenGLTexture(mapImageReader->getMapImage());
	mapPanel.texture->bind();
	mapPanel.texture->setMinificationFilter(QOpenGLTexture::Linear);
	mapPanel.texture->setMagnificationFilter(QOpenGLTexture::Linear);
	mapPanel.texture->setWrapMode(QOpenGLTexture::ClampToEdge);
	mapPanel.texture->release();

	paintDevice = new QOpenGLPaintDevice();
	painter = new QPainter();
	painter->begin(paintDevice);
	painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	painter->end();

	return true;
}

bool Renderer::loadShaders(Panel* panel, const QString& shaderName)
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

void Renderer::loadBuffer(Panel* panel, GLfloat* buffer, int size)
{
	panel->buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	panel->buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
	panel->buffer->create();
	panel->buffer->bind();
	panel->buffer->allocate(buffer, sizeof(GLfloat) * size);
	panel->buffer->release();
}

void Renderer::shutdown()
{
	qDebug("Shutting down Renderer");

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

void Renderer::startRendering(double windowWidth, double windowHeight, double frameTime)
{
	renderTimer.restart();

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->frameTime = frameTime;

	paintDevice->setSize(QSize(windowWidth, windowHeight));

	glViewport(0, 0, windowWidth, windowHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	if (videoWindow->keyIsDownOnce(Qt::Key_F2))
	{
		switch (selectedPanel)
		{
			case SelectedPanel::VIDEO:
			{
				selectedPanel = SelectedPanel::MAP;
				selectedPanelPtr = &mapPanel;
				break;
			}

			case SelectedPanel::MAP:
			{
				selectedPanel = SelectedPanel::VIDEO;
				selectedPanelPtr = &videoPanel;
				break;
			}

			default: break;
		}
	}

	if (videoWindow->keyIsDown(Qt::Key_R))
	{
		selectedPanelPtr->userX = 0.0;
		selectedPanelPtr->userY = 0.0;
		selectedPanelPtr->userAngle = 0.0;
		selectedPanelPtr->userScale = 1.0;
	}

	if (videoWindow->keyIsDown(Qt::Key_W))
		selectedPanelPtr->userAngle += 0.1 * frameTime;

	if (videoWindow->keyIsDown(Qt::Key_S))
		selectedPanelPtr->userAngle -= 0.1 * frameTime;

	if (videoWindow->keyIsDown(Qt::Key_Q))
		selectedPanelPtr->userScale *= (1.0 + frameTime / 500);

	if (videoWindow->keyIsDown(Qt::Key_A))
		selectedPanelPtr->userScale *= (1.0 - frameTime / 500);

	if (videoWindow->keyIsDown(Qt::Key_Left))
		selectedPanelPtr->userX -= 1.0 * frameTime;

	if (videoWindow->keyIsDown(Qt::Key_Right))
		selectedPanelPtr->userX += 1.0 * frameTime;

	if (videoWindow->keyIsDown(Qt::Key_Up))
		selectedPanelPtr->userY += 1.0 * frameTime;

	if (videoWindow->keyIsDown(Qt::Key_Down))
		selectedPanelPtr->userY -= 1.0 * frameTime;
}

void Renderer::uploadFrameData(FrameData* frameData)
{
	QOpenGLPixelTransferOptions options;

	options.setRowLength(frameData->rowLength / 4);
	options.setImageHeight(frameData->height);
	options.setAlignment(1);

	videoPanel.texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData->data, &options);
}

void Renderer::renderVideoPanel()
{
	videoPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
		videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	else
		videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);

	double offsetX = (windowWidth / 2.0) - (((1.0 - mapPanelRelativeWidth) * windowWidth) / 2.0);

	videoPanel.scale = ((1.0 - mapPanelRelativeWidth) * windowWidth) / videoPanel.textureWidth;

	if (videoPanel.scale * videoPanel.textureHeight > windowHeight)
		videoPanel.scale = windowHeight / videoPanel.textureHeight;

	videoPanel.scale *= videoPanel.userScale;

	videoPanel.vertexMatrix.translate(
		offsetX + videoPanel.x + videoPanel.userX + videoStabilizer->getX() * videoPanel.textureWidth * videoPanel.scale,
		videoPanel.y + videoPanel.userY - videoStabilizer->getY() * videoPanel.textureHeight * videoPanel.scale,
		0.0f);

	videoPanel.vertexMatrix.rotate(videoPanel.angle + videoPanel.userAngle - videoStabilizer->getAngle(), 0.0f, 0.0f, 1.0f);
	videoPanel.vertexMatrix.scale(videoPanel.scale);

	renderPanel(&videoPanel);
}

void Renderer::renderMapPanel()
{
	mapPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
		mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	else
		mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);

	mapPanel.vertexMatrix.translate(mapPanel.x + mapPanel.userX, mapPanel.y + mapPanel.userY);
	mapPanel.vertexMatrix.rotate(mapPanel.angle + mapPanel.userAngle, 0.0f, 0.0f, 1.0f);
	mapPanel.vertexMatrix.scale(mapPanel.scale * mapPanel.userScale);

	int mapBorderX = (int)(mapPanelRelativeWidth * windowWidth + 0.5);

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, mapBorderX, (int)windowHeight);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	renderPanel(&mapPanel);
	glDisable(GL_SCISSOR_TEST);

	painter->begin(paintDevice);
	painter->setPen(QColor(0, 0, 0));
	painter->drawLine(mapBorderX, 0, mapBorderX, (int)windowHeight);
	painter->end();
}

void Renderer::renderPanel(Panel* panel)
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

void Renderer::renderInfoPanel(double spareTime)
{
	averageFps.addMeasurement(1000.0 / frameTime);
	averageFrameTime.addMeasurement(frameTime);
	averageDecodeTime.addMeasurement(videoDecoder->getLastDecodeTime());
	averageStabilizeTime.addMeasurement(videoStabilizer->getLastProcessTime());
	averageRenderTime.addMeasurement(lastRenderTime);
	averageEncodeTime.addMeasurement(videoEncoder->getLastEncodeTime());
	averageSpareTime.addMeasurement(spareTime);

	int textX = 10;
	int textY = 6;
	int lineHeight = 17;
	int textWidth = 175;
	int textHeight = 141;

	painter->begin(paintDevice);
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

	textX = 85;
	textY = 6;

	painter->drawText(textX, textY, textWidth, textHeight, 0, QString::number(averageFps.getAverage(), 'f', 2));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageFrameTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageDecodeTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageStabilizeTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageRenderTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageEncodeTime.getAverage(), 'f', 2)));

	if (spareTime < 0)
		painter->setPen(QColor(255, 0, 0, 200));
	else if (spareTime > 0)
		painter->setPen(QColor(0, 255, 0, 200));

	painter->drawText(textX, textY += lineHeight, textWidth, textHeight, 0, QString("%1 ms").arg(QString::number(averageSpareTime.getAverage(), 'f', 2)));
	painter->end();
}

void Renderer::stopRendering()
{
	lastRenderTime = renderTimer.nsecsElapsed() / 1000000.0;
}

void Renderer::setFlipOutput(bool value)
{
	paintDevice->setPaintFlipped(value);
	flipOutput = value;
}
