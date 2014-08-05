// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include <QOpenGLPixelTransferOptions>
#include <QTime>

#include "Renderer.h"
#include "VideoWindow.h"
#include "VideoDecoder.h"
#include "QuickRouteReader.h"
#include "MapImageReader.h"
#include "VideoStabilizer.h"
#include "InputHandler.h"
#include "Settings.h"
#include "FrameData.h"

using namespace OrientView;

Renderer::Renderer()
{
}

bool Renderer::initialize(VideoWindow* videoWindow, VideoDecoder* videoDecoder, QuickRouteReader* quickRouteReader, MapImageReader* mapImageReader, VideoStabilizer* videoStabilizer, InputHandler* inputHandler, Settings* settings)
{
	qDebug("Initializing Renderer");

	this->videoStabilizer = videoStabilizer;
	this->inputHandler = inputHandler;

	videoPanel = Panel();
	videoPanel.textureWidth = videoDecoder->getFrameWidth();
	videoPanel.textureHeight = videoDecoder->getFrameHeight();
	videoPanel.texelWidth = 1.0 / videoPanel.textureWidth;
	videoPanel.texelHeight = 1.0 / videoPanel.textureHeight;
	videoPanel.userScale = settings->appearance.videoPanelScale;
	videoPanel.clearColor = settings->appearance.videoPanelBackgroundColor;
	videoPanel.clearEnabled = !settings->stabilizer.disableVideoClear;

	mapPanel = Panel();
	mapPanel.textureWidth = mapImageReader->getMapImage().width();
	mapPanel.textureHeight = mapImageReader->getMapImage().height();
	mapPanel.texelWidth = 1.0 / mapPanel.textureWidth;
	mapPanel.texelHeight = 1.0 / mapPanel.textureHeight;
	mapPanel.clearColor = settings->appearance.mapPanelBackgroundColor;

	renderMode = RenderMode::ALL;
	mapPanelRelativeWidth = settings->appearance.mapPanelWidth;
	windowWidth = videoWindow->width();
	windowHeight = videoWindow->height();

	showInfoPanel = settings->appearance.showInfoPanel;

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

	routePath = new QPainterPath();
	int routePointCount = quickRouteReader->getRouteData().routePoints.size();

	if (routePointCount >= 2)
	{
		for (int i = 0; i < routePointCount; ++i)
		{
			RoutePoint rp = quickRouteReader->getRouteData().routePoints.at(i);

			double x = rp.position.x() * 10000.0;
			double y = rp.position.y() * 10000.0;

			if (i == 0)
				routePath->moveTo(x, y);
			else
				routePath->lineTo(x, y);
		}
	}

	glClearColor(videoPanel.clearColor.redF(), videoPanel.clearColor.greenF(), videoPanel.clearColor.blueF(), 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

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

	if (routePath != nullptr)
	{
		delete routePath;
		routePath = nullptr;
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

void Renderer::startRendering(double windowWidth, double windowHeight, double frameTime, double spareTime, double decoderTime, double stabilizerTime, double encoderTime)
{
	renderTimer.restart();

	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->frameTime = frameTime;

	averageFps.addMeasurement(1000.0 / frameTime);
	averageFrameTime.addMeasurement(frameTime);
	averageDecodeTime.addMeasurement(decoderTime);
	averageStabilizeTime.addMeasurement(stabilizerTime);
	averageRenderTime.addMeasurement(lastRenderTime);
	averageEncodeTime.addMeasurement(encoderTime);
	averageSpareTime.addMeasurement(spareTime);

	paintDevice->setSize(QSize(windowWidth, windowHeight));

	glViewport(0, 0, windowWidth, windowHeight);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void Renderer::uploadFrameData(FrameData* frameData)
{
	QOpenGLPixelTransferOptions options;

	options.setRowLength(frameData->rowLength / 4);
	options.setImageHeight(frameData->height);
	options.setAlignment(1);

	videoPanel.texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, frameData->data, &options);
}

void Renderer::renderAll()
{
	if (renderMode == RenderMode::ALL || renderMode == RenderMode::VIDEO)
		renderVideoPanel();

	if (renderMode == RenderMode::ALL || renderMode == RenderMode::MAP)
		renderMapPanel();

	if (showInfoPanel)
		renderInfoPanel();
}

void Renderer::renderVideoPanel()
{
	videoPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
		videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	else
		videoPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);

	double offsetX = 0.0;

	if (renderMode != RenderMode::VIDEO)
		offsetX += (windowWidth / 2.0) - (((1.0 - mapPanelRelativeWidth) * windowWidth) / 2.0);

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

	if (videoPanel.clearEnabled)
	{
		glClearColor(videoPanel.clearColor.redF(), videoPanel.clearColor.greenF(), videoPanel.clearColor.blueF(), 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	renderPanel(&videoPanel);
}

void Renderer::renderMapPanel()
{
	mapPanel.vertexMatrix.setToIdentity();

	if (!flipOutput)
		mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2, 0.0f, 1.0f);
	else
		mapPanel.vertexMatrix.ortho(-windowWidth / 2, windowWidth / 2, windowHeight / 2, -windowHeight / 2, 0.0f, 1.0f);

	mapPanel.scale = windowWidth / mapPanel.textureWidth;

	if (mapPanel.scale * mapPanel.textureHeight > windowHeight)
		mapPanel.scale = windowHeight / mapPanel.textureHeight;

	mapPanel.scale *= mapPanel.userScale;

	mapPanel.vertexMatrix.translate(mapPanel.x + mapPanel.userX, mapPanel.y + mapPanel.userY);
	mapPanel.vertexMatrix.rotate(mapPanel.angle + mapPanel.userAngle, 0.0f, 0.0f, 1.0f);
	mapPanel.vertexMatrix.scale(mapPanel.scale);

	int mapBorderX = (int)(mapPanelRelativeWidth * windowWidth + 0.5);

	if (renderMode != RenderMode::MAP)
		glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, mapBorderX, (int)windowHeight);

	if (mapPanel.clearEnabled)
	{
		glClearColor(mapPanel.clearColor.redF(), mapPanel.clearColor.greenF(), mapPanel.clearColor.blueF(), 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}

	renderPanel(&mapPanel);
	renderRoute();

	glDisable(GL_SCISSOR_TEST);

	painter->begin(paintDevice);
	painter->setPen(QColor(0, 0, 0));
	painter->drawLine(mapBorderX, 0, mapBorderX, (int)windowHeight);
	painter->end();
}

void Renderer::renderInfoPanel()
{
	QFont font = QFont("DejaVu Sans", 8, QFont::Bold);
	QFontMetrics metrics(font);

	int textX = 10;
	int textY = 6;
	int lineHeight = metrics.height();
	int lineSpacing = metrics.lineSpacing() + 1;
	int lineWidth1 = metrics.boundingRect("stabilize:").width();
	int lineWidth2 = metrics.boundingRect("999.99 ms").width();
	int rightPartMargin = 12;
	int backgroundRadius = 10;
	int backgroundWidth = textX + backgroundRadius + lineWidth1 + rightPartMargin + lineWidth2 + 2;
	int backgroundHeight = lineSpacing * 11 + textY + 3;

	QColor textColor = QColor(255, 255, 255, 200);
	QColor textGreenColor = QColor(0, 255, 0, 200);
	QColor textRedColor = QColor(255, 0, 0, 200);

	painter->begin(paintDevice);
	painter->setPen(QColor(0, 0, 0));
	painter->setBrush(QBrush(QColor(20, 20, 20, 220)));
	painter->drawRoundedRect(-backgroundRadius, -backgroundRadius, backgroundWidth, backgroundHeight, backgroundRadius, backgroundRadius);

	painter->setPen(textColor);
	painter->setFont(font);
	painter->drawText(textX, textY, lineWidth1, lineHeight, 0, "fps:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "frame:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "decode:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "stabilize:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "render:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "encode:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "spare:");

	textY += lineSpacing;

	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "selected:");
	painter->drawText(textX, textY += lineSpacing, lineWidth1, lineHeight, 0, "render:");

	textX += lineWidth1 + rightPartMargin;
	textY = 6;

	painter->drawText(textX, textY, lineWidth2, lineHeight, 0, QString::number(averageFps.getAverage(), 'f', 2));
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageFrameTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageDecodeTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageStabilizeTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageRenderTime.getAverage(), 'f', 2)));
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageEncodeTime.getAverage(), 'f', 2)));

	if (averageSpareTime.getAverage() < 0)
		painter->setPen(textRedColor);
	else if (averageSpareTime.getAverage() > 0)
		painter->setPen(textGreenColor);

	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, QString("%1 ms").arg(QString::number(averageSpareTime.getAverage(), 'f', 2)));
	
	QString selectedText;
	QString renderText;

	switch (inputHandler->getSelectedPanel())
	{
		case SelectedPanel::NONE: selectedText = "none"; break;
		case SelectedPanel::VIDEO: selectedText = "video"; break;
		case SelectedPanel::MAP: selectedText = "map"; break;
		default: selectedText = "unknown"; break;
	}

	switch (renderMode)
	{
		case RenderMode::ALL: renderText = "both"; break;
		case RenderMode::VIDEO: renderText = "video"; break;
		case RenderMode::MAP: renderText = "map"; break;
		default: renderText = "unknown"; break;
	}

	textY += lineSpacing;

	painter->setPen(textColor);
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, selectedText);
	painter->drawText(textX, textY += lineSpacing, lineWidth2, lineHeight, 0, renderText);

	painter->end();
}

void Renderer::renderRoute()
{
	QPen pen;
	pen.setColor(QColor(200, 0, 0, 128));
	pen.setWidth(15);
	pen.setCapStyle(Qt::PenCapStyle::RoundCap);
	pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);

	QMatrix m;
	m.translate(windowWidth / 2.0 + mapPanel.x + mapPanel.userX, windowHeight / 2.0 - mapPanel.y - mapPanel.userY);
	m.scale(mapPanel.scale, mapPanel.scale);
	m.rotate(-(mapPanel.angle + mapPanel.userAngle));

	painter->begin(paintDevice);

	if (renderMode != RenderMode::MAP)	
	{
		painter->setClipping(true);
		painter->setClipRect(0, 0, (int)(mapPanelRelativeWidth * windowWidth + 0.5), (int)windowHeight);
	}
	else
		painter->setClipping(false);
	
	painter->setPen(pen);
	painter->setWorldMatrix(m);
	painter->drawPath(*routePath);
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

void Renderer::stopRendering()
{
	lastRenderTime = renderTimer.nsecsElapsed() / 1000000.0;
}

Panel* Renderer::getVideoPanel()
{
	return &videoPanel;
}

Panel* Renderer::getMapPanel()
{
	return &mapPanel;
}

RenderMode Renderer::getRenderMode()
{
	return renderMode;
}

void Renderer::setRenderMode(RenderMode mode)
{
	renderMode = mode;
}

void Renderer::setFlipOutput(bool value)
{
	paintDevice->setPaintFlipped(value);
	flipOutput = value;
}

void Renderer::toggleShowInfoPanel()
{
	showInfoPanel = !showInfoPanel;
}
