// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "InputHandler.h"
#include "VideoWindow.h"
#include "Renderer.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "RenderOnScreenThread.h"
#include "Settings.h"

using namespace OrientView;

bool InputHandler::initialize(VideoWindow* videoWindow, Renderer* renderer, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, RenderOnScreenThread* renderOnScreenThread, Settings* settings)
{
	qDebug("Initializing InputHandler");

	this->videoWindow = videoWindow;
	this->renderer = renderer;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->renderOnScreenThread = renderOnScreenThread;
	this->settings = settings;

	selectedPanel = SelectedPanel::NONE;

	return true;
}

void InputHandler::handleInput(double frameTime)
{
	if (videoWindow->keyIsDownOnce(Qt::Key_F1))
		renderer->toggleShowInfoPanel();

	if (videoWindow->keyIsDownOnce(Qt::Key_F2))
	{
		switch (selectedPanel)
		{
			case SelectedPanel::NONE: selectedPanel = SelectedPanel::VIDEO; break;
			case SelectedPanel::VIDEO: selectedPanel = SelectedPanel::MAP; break;
			case SelectedPanel::MAP: selectedPanel = SelectedPanel::NONE; break;
			default: break;
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F3))
	{
		switch (renderer->getRenderMode())
		{
			case RenderMode::ALL: renderer->setRenderMode(RenderMode::VIDEO); break;
			case RenderMode::VIDEO: renderer->setRenderMode(RenderMode::MAP); break;
			case RenderMode::MAP: renderer->setRenderMode(RenderMode::ALL); break;
			default: break;
		}

		renderer->requestFullClear();
	}

	int seekAmount = settings->inputHandler.normalSeekAmount;
	double translateVelocity = settings->inputHandler.normalTranslateVelocity;
	double rotateVelocity = settings->inputHandler.normalRotateVelocity;
	double scaleConstant = settings->inputHandler.normalScaleConstant;

	if (videoWindow->keyIsDown(Qt::Key_Control))
	{
		seekAmount = settings->inputHandler.smallSeekAmount;
		translateVelocity = settings->inputHandler.slowTranslateVelocity;
		rotateVelocity = settings->inputHandler.slowRotateVelocity;
		scaleConstant = settings->inputHandler.smallScaleConstant;
	}

	if (videoWindow->keyIsDown(Qt::Key_Shift))
	{
		seekAmount = settings->inputHandler.largeSeekAmount;
		translateVelocity = settings->inputHandler.fastTranslateVelocity;
		rotateVelocity = settings->inputHandler.fastRotateVelocity;
		scaleConstant = settings->inputHandler.largeScaleConstant;
	}

	if (videoWindow->keyIsDown(Qt::Key_Alt))
		seekAmount = settings->inputHandler.veryLargeSeekAmount;

	translateVelocity *= frameTime;
	rotateVelocity *= frameTime;

	if (!videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDownOnce(Qt::Key_Space))
		renderOnScreenThread->togglePaused();
	else if (videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDownOnce(Qt::Key_Space) && renderOnScreenThread->isPaused())
		renderOnScreenThread->advanceOneFrame();

	if (selectedPanel == SelectedPanel::NONE)
	{
		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			videoDecoder->seekRelative(-seekAmount);
			videoDecoderThread->signalFrameRead();
			renderOnScreenThread->advanceOneFrame();
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			videoDecoder->seekRelative(seekAmount);
			videoDecoderThread->signalFrameRead();
			renderOnScreenThread->advanceOneFrame();
		}
	}
	else
	{
		Panel* targetPanel = nullptr;
		bool panelHasMoved = false;

		switch (selectedPanel)
		{
			case SelectedPanel::VIDEO: targetPanel = renderer->getVideoPanel(); break;
			case SelectedPanel::MAP: targetPanel = renderer->getMapPanel(); break;
			default: break;
		}

		if (videoWindow->keyIsDown(Qt::Key_R))
		{
			targetPanel->userX = 0.0;
			targetPanel->userY = 0.0;
			targetPanel->userAngle = 0.0;
			targetPanel->userScale = 1.0;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			targetPanel->userX -= translateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			targetPanel->userX += translateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Up))
		{
			targetPanel->userY += translateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Down))
		{
			targetPanel->userY -= translateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_W))
		{
			targetPanel->userAngle += rotateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_S))
		{
			targetPanel->userAngle -= rotateVelocity;
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Q))
		{
			targetPanel->userScale *= (1.0 + frameTime / scaleConstant);
			panelHasMoved = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_A))
		{
			targetPanel->userScale *= (1.0 - frameTime / scaleConstant);
			panelHasMoved = true;
		}

		if (panelHasMoved = true)
			renderer->requestFullClear();
	}
}

SelectedPanel InputHandler::getSelectedPanel() const
{
	return selectedPanel;
}
