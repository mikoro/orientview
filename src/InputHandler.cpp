// Copyright © 2014 Mikko Ronkainen <firstname@mikkoronkainen.com>
// License: GPLv3, see the LICENSE file.

#include "InputHandler.h"
#include "VideoWindow.h"
#include "Renderer.h"
#include "VideoDecoder.h"
#include "VideoDecoderThread.h"
#include "VideoStabilizer.h"
#include "RouteManager.h"
#include "RenderOnScreenThread.h"
#include "Settings.h"

using namespace OrientView;

void InputHandler::initialize(VideoWindow* videoWindow, Renderer* renderer, VideoDecoder* videoDecoder, VideoDecoderThread* videoDecoderThread, VideoStabilizer* videoStabilizer, RouteManager* routeManager, RenderOnScreenThread* renderOnScreenThread, Settings* settings)
{
	this->videoWindow = videoWindow;
	this->renderer = renderer;
	this->videoDecoder = videoDecoder;
	this->videoDecoderThread = videoDecoderThread;
	this->videoStabilizer = videoStabilizer;
	this->routeManager = routeManager;
	this->renderOnScreenThread = renderOnScreenThread;
	this->settings = settings;
}

void InputHandler::handleInput(double frameTime)
{
	if (videoWindow->keyIsDownOnce(Qt::Key_F1))
		renderer->toggleShowInfoPanel();

	if (videoWindow->keyIsDownOnce(Qt::Key_F2))
	{
		switch (editMode)
		{
			case EditMode::None: editMode = EditMode::Video; break;
			case EditMode::Video: editMode = EditMode::Map; break;
			case EditMode::Map: editMode = EditMode::MapWidth; break;
			case EditMode::MapWidth: editMode = EditMode::None; break;
			default: break;
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F3))
	{
		switch (renderer->getRenderMode())
		{
			case RenderMode::All: renderer->setRenderMode(RenderMode::Video); break;
			case RenderMode::Video: renderer->setRenderMode(RenderMode::Map); break;
			case RenderMode::Map: renderer->setRenderMode(RenderMode::All); break;
			default: break;
		}

		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F4))
		videoStabilizer->toggleEnabled();

	if (!videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDownOnce(Qt::Key_Space))
		renderOnScreenThread->togglePaused();
	else if (videoWindow->keyIsDown(Qt::Key_Control) && keyIsDownWithRepeat(Qt::Key_Space, advanceOneFrameRepeatHandler))
	{
		if (!renderOnScreenThread->getIsPaused())
			renderOnScreenThread->togglePaused();

		renderOnScreenThread->advanceOneFrame();
		videoWindow->keyIsDownOnce(Qt::Key_Space); // clear key state
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

	bool shouldRequestFullClear = false;

	if (videoWindow->keyIsDown(Qt::Key_PageUp))
	{
		routeManager->getDefaultRoute().startOffset += translateVelocity * 0.1;
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_PageDown))
	{
		routeManager->getDefaultRoute().startOffset -= translateVelocity * 0.1;
		routeManager->requestFullUpdate();
	}

	if (editMode == EditMode::None)
	{
		if (keyIsDownWithRepeat(Qt::Key_Left, seekBackwardRepeatHandler))
		{
			videoDecoder->seekRelative(-seekAmount);
			videoDecoderThread->signalFrameRead();
			renderOnScreenThread->advanceOneFrame();
			videoStabilizer->reset();
		}

		if (keyIsDownWithRepeat(Qt::Key_Right, seekForwardRepeatHandler))
		{
			videoDecoder->seekRelative(seekAmount);
			videoDecoderThread->signalFrameRead();
			renderOnScreenThread->advanceOneFrame();
			videoStabilizer->reset();
		}
	}

	if (editMode == EditMode::Video || editMode == EditMode::Map)
	{
		Panel* targetPanel = nullptr;

		switch (editMode)
		{
			case EditMode::Video: targetPanel = &renderer->getVideoPanel(); break;
			case EditMode::Map: targetPanel = &renderer->getMapPanel(); break;
			default: break;
		}

		assert(targetPanel != nullptr);

		if (videoWindow->keyIsDown(Qt::Key_R))
		{
			targetPanel->userX = 0.0;
			targetPanel->userY = 0.0;
			targetPanel->userAngle = 0.0;
			targetPanel->userScale = 1.0;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_W))
		{
			targetPanel->userAngle += rotateVelocity;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_S))
		{
			targetPanel->userAngle -= rotateVelocity;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Q))
		{
			targetPanel->userScale *= (1.0 + frameTime / scaleConstant);
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_A))
		{
			targetPanel->userScale *= (1.0 - frameTime / scaleConstant);
			shouldRequestFullClear = true;
		}
	}

	if (editMode == EditMode::Video)
	{
		Panel& videoPanel = renderer->getVideoPanel();

		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			videoPanel.userX -= translateVelocity;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			videoPanel.userX += translateVelocity;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Up))
		{
			videoPanel.userY += translateVelocity;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Down))
		{
			videoPanel.userY -= translateVelocity;
			shouldRequestFullClear = true;
		}
	}

	if (editMode == EditMode::Map)
	{
		Panel& mapPanel = renderer->getMapPanel();

		translateVelocity *= (-1.0 / (mapPanel.scale * mapPanel.userScale));

		double angle = (mapPanel.angle + mapPanel.userAngle) * M_PI / 180.0;
		double deltaX = cos(angle) * translateVelocity;
		double deltaY = sin(angle) * translateVelocity;

		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			mapPanel.userX -= deltaX;
			mapPanel.userY += deltaY;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			mapPanel.userX += deltaX;
			mapPanel.userY -= deltaY;
			shouldRequestFullClear = true;
		}

		deltaX = sin(angle) * translateVelocity;
		deltaY = cos(angle) * translateVelocity;

		if (videoWindow->keyIsDown(Qt::Key_Up))
		{
			mapPanel.userX += deltaX;
			mapPanel.userY += deltaY;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Down))
		{
			mapPanel.userX -= deltaX;
			mapPanel.userY -= deltaY;
			shouldRequestFullClear = true;
		}
	}

	if (editMode == EditMode::MapWidth)
	{
		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			renderer->getMapPanel().relativeWidth -= translateVelocity * 0.001;
			shouldRequestFullClear = true;
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			renderer->getMapPanel().relativeWidth += translateVelocity * 0.001;
			shouldRequestFullClear = true;
		}

		renderer->getMapPanel().relativeWidth = std::max(0.0, std::min(renderer->getMapPanel().relativeWidth, 1.0));
	}

	if (shouldRequestFullClear)
		renderer->requestFullClear();
}

EditMode InputHandler::getEditMode() const
{
	return editMode;
}

bool InputHandler::keyIsDownWithRepeat(int key, RepeatHandler& repeatHandler)
{
	bool isDown = false;

	if (videoWindow->keyIsDown(key))
	{
		if (repeatHandler.hasBeenReleased || (repeatHandler.firstRepeatTimer.elapsed() > firstRepeatDelay))
		{
			if (repeatHandler.repeatTimer.elapsed() > repeatDelay)
			{
				isDown = true;
				repeatHandler.repeatTimer.restart();
			}

			repeatHandler.hasBeenReleased = false;
		}
	}
	else
	{
		repeatHandler.firstRepeatTimer.restart();
		repeatHandler.hasBeenReleased = true;
	}

	return isDown;
}
