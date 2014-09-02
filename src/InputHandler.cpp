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

	seekBackwardRepeatHandler.firstRepeatTimer.start();
	seekBackwardRepeatHandler.repeatTimer.start();

	seekForwardRepeatHandler.firstRepeatTimer.start();
	seekForwardRepeatHandler.repeatTimer.start();

	advanceOneFrameRepeatHandler.firstRepeatTimer.start();
	advanceOneFrameRepeatHandler.repeatTimer.start();
}

void InputHandler::handleInput(double frameTime)
{
	Panel& mapPanel = renderer->getMapPanel();
	Panel& videoPanel = renderer->getVideoPanel();
	Route& defaultRoute = routeManager->getDefaultRoute();

	if (videoWindow->keyIsDownOnce(Qt::Key_F1))
		renderer->toggleShowInfoPanel();

	if (videoWindow->keyIsDownOnce(Qt::Key_F2))
	{
		switch (scrollMode)
		{
			case ScrollMode::None: scrollMode = ScrollMode::Map; break;
			case ScrollMode::Map: scrollMode = ScrollMode::Video; break;
			case ScrollMode::Video: scrollMode = ScrollMode::None; break;
			default: scrollMode = ScrollMode::None;
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F3))
	{
		switch (renderer->getRenderMode())
		{
			case RenderMode::All: renderer->setRenderMode(RenderMode::Map); break;
			case RenderMode::Map: renderer->setRenderMode(RenderMode::Video); break;
			case RenderMode::Video: renderer->setRenderMode(RenderMode::All); break;
			default: renderer->setRenderMode(RenderMode::All);
		}

		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F4))
	{
		switch (defaultRoute.routeRenderMode)
		{
			case RouteRenderMode::None: defaultRoute.routeRenderMode = RouteRenderMode::Discreet; break;
			case RouteRenderMode::Discreet: defaultRoute.routeRenderMode = RouteRenderMode::Highlight; break;
			case RouteRenderMode::Highlight: defaultRoute.routeRenderMode = RouteRenderMode::Pace; break;
			case RouteRenderMode::Pace: defaultRoute.routeRenderMode = RouteRenderMode::None; break;
			default: defaultRoute.routeRenderMode = RouteRenderMode::Discreet;
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F5))
	{
		switch (defaultRoute.tailRenderMode)
		{
			case RouteRenderMode::None: defaultRoute.tailRenderMode = RouteRenderMode::Discreet; break;
			case RouteRenderMode::Discreet: defaultRoute.tailRenderMode = RouteRenderMode::Highlight; break;
			case RouteRenderMode::Highlight: defaultRoute.tailRenderMode = RouteRenderMode::None; break;
			default: defaultRoute.tailRenderMode = RouteRenderMode::None;
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F6))
	{
		switch (routeManager->getViewMode())
		{
			case ViewMode::FixedSplit: routeManager->setViewMode(ViewMode::RunnerCentered); break;
			case ViewMode::RunnerCentered: routeManager->setViewMode(ViewMode::RunnerCenteredSplitOriented); break;
			case ViewMode::RunnerCenteredSplitOriented: routeManager->setViewMode(ViewMode::FixedSplit); break;
			default: routeManager->setViewMode(ViewMode::FixedSplit);
		}
	}

	if (videoWindow->keyIsDownOnce(Qt::Key_F7))
		defaultRoute.showRunner = !defaultRoute.showRunner;

	if (videoWindow->keyIsDownOnce(Qt::Key_F8))
		defaultRoute.showControls = !defaultRoute.showControls;

	if (videoWindow->keyIsDownOnce(Qt::Key_F9))
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

	double seekAmount = settings->inputHandler.normalSeekAmount;
	double translateSpeed = settings->inputHandler.normalTranslateSpeed;
	double rotateSpeed = settings->inputHandler.normalRotateSpeed;
	double scaleSpeed = settings->inputHandler.normalScaleSpeed;
	double timeOffset = settings->inputHandler.normalTimeOffset;

	if (videoWindow->keyIsDown(Qt::Key_Control))
	{
		seekAmount = settings->inputHandler.smallSeekAmount;
		translateSpeed = settings->inputHandler.slowTranslateSpeed;
		rotateSpeed = settings->inputHandler.slowRotateSpeed;
		scaleSpeed = settings->inputHandler.slowScaleSpeed;
		timeOffset = settings->inputHandler.smallTimeOffset;
	}

	if (videoWindow->keyIsDown(Qt::Key_Shift))
	{
		seekAmount = settings->inputHandler.largeSeekAmount;
		translateSpeed = settings->inputHandler.fastTranslateSpeed;
		rotateSpeed = settings->inputHandler.fastRotateSpeed;
		scaleSpeed = settings->inputHandler.fastScaleSpeed;
		timeOffset = settings->inputHandler.largeTimeOffset;
	}

	if (videoWindow->keyIsDown(Qt::Key_Alt))
	{
		seekAmount = settings->inputHandler.veryLargeSeekAmount;
		translateSpeed = settings->inputHandler.veryFastTranslateSpeed;
		rotateSpeed = settings->inputHandler.veryFastRotateSpeed;
		scaleSpeed = settings->inputHandler.veryFastScaleSpeed;
		timeOffset = settings->inputHandler.veryLargeTimeOffset;
	}

	translateSpeed *= frameTime;
	rotateSpeed *= frameTime;
	scaleSpeed *= frameTime;
	timeOffset *= frameTime / 33.367; // time offsets have been calibrated for 30 fps

	if (videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDown(Qt::Key_1))
	{
		mapPanel.userX = 0.0;
		mapPanel.userY = 0.0;
		mapPanel.userAngle = 0.0;
		mapPanel.userScale = 1.0;
		mapPanel.relativeWidth = 0.25;

		renderer->requestFullClear();
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDown(Qt::Key_2))
	{
		videoPanel.userX = 0.0;
		videoPanel.userY = 0.0;
		videoPanel.userAngle = 0.0;
		videoPanel.userScale = 1.0;

		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDown(Qt::Key_3))
	{
		defaultRoute.userScale = 1.0;
	}

	if (videoWindow->keyIsDown(Qt::Key_Control) && videoWindow->keyIsDown(Qt::Key_4))
	{
		defaultRoute.controlTimeOffset = 0.0;
		defaultRoute.runnerTimeOffset = 0.0;

		routeManager->requestFullUpdate();
	}

	if (scrollMode == ScrollMode::None)
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

	if (scrollMode == ScrollMode::Map)
	{
		double scaledTranslateSpeed = translateSpeed * (-1.0 / (mapPanel.scale * mapPanel.userScale));

		double angle = (mapPanel.angle + mapPanel.userAngle + routeManager->getAngle()) * M_PI / 180.0;
		double deltaX = cos(angle) * scaledTranslateSpeed;
		double deltaY = sin(angle) * scaledTranslateSpeed;

		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			mapPanel.userX -= deltaX;
			mapPanel.userY += deltaY;
			renderer->requestFullClear();
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			mapPanel.userX += deltaX;
			mapPanel.userY -= deltaY;
			renderer->requestFullClear();
		}

		deltaX = sin(angle) * scaledTranslateSpeed;
		deltaY = cos(angle) * scaledTranslateSpeed;

		if (videoWindow->keyIsDown(Qt::Key_Up))
		{
			mapPanel.userX += deltaX;
			mapPanel.userY += deltaY;
			renderer->requestFullClear();
		}

		if (videoWindow->keyIsDown(Qt::Key_Down))
		{
			mapPanel.userX -= deltaX;
			mapPanel.userY -= deltaY;
			renderer->requestFullClear();
		}
	}

	if (scrollMode == ScrollMode::Video)
	{
		if (videoWindow->keyIsDown(Qt::Key_Left))
		{
			videoPanel.userX -= translateSpeed;
			renderer->requestFullClear();
		}

		if (videoWindow->keyIsDown(Qt::Key_Right))
		{
			videoPanel.userX += translateSpeed;
			renderer->requestFullClear();
		}

		if (videoWindow->keyIsDown(Qt::Key_Up))
		{
			videoPanel.userY += translateSpeed;
			renderer->requestFullClear();
		}

		if (videoWindow->keyIsDown(Qt::Key_Down))
		{
			videoPanel.userY -= translateSpeed;
			renderer->requestFullClear();
		}
	}

	if (videoWindow->keyIsDown(Qt::Key_Q))
	{
		mapPanel.userScale *= (1.0 + scaleSpeed);
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_A))
	{
		mapPanel.userScale *= (1.0 - scaleSpeed);
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_W))
	{
		mapPanel.userAngle += rotateSpeed;
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_S))
	{
		mapPanel.userAngle -= rotateSpeed;
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_E))
	{
		mapPanel.relativeWidth += translateSpeed * 0.001;
		mapPanel.relativeWidth = std::max(0.0, std::min(mapPanel.relativeWidth, 1.0));
		renderer->requestFullClear();
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_D))
	{
		mapPanel.relativeWidth -= translateSpeed * 0.001;
		mapPanel.relativeWidth = std::max(0.0, std::min(mapPanel.relativeWidth, 1.0));
		renderer->requestFullClear();
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_R))
	{
		videoPanel.userScale *= (1.0 + scaleSpeed);
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_F))
	{
		videoPanel.userScale *= (1.0 - scaleSpeed);
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_T))
	{
		videoPanel.userAngle += rotateSpeed;
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_G))
	{
		videoPanel.userAngle -= rotateSpeed;
		renderer->requestFullClear();
	}

	if (videoWindow->keyIsDown(Qt::Key_Y))
	{
		defaultRoute.userScale *= (1.0 + scaleSpeed);
		defaultRoute.userScale = std::max(0.001, defaultRoute.userScale);
	}

	if (videoWindow->keyIsDown(Qt::Key_H))
	{
		defaultRoute.userScale *= (1.0 - scaleSpeed);
		defaultRoute.userScale = std::max(0.001, defaultRoute.userScale);
	}

	if (videoWindow->keyIsDown(Qt::Key_PageUp))
	{
		defaultRoute.runnerTimeOffset += timeOffset;
	}

	if (videoWindow->keyIsDown(Qt::Key_PageDown))
	{
		defaultRoute.runnerTimeOffset -= timeOffset;
	}

	if (videoWindow->keyIsDown(Qt::Key_Home))
	{
		defaultRoute.controlTimeOffset += timeOffset;
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_End))
	{
		defaultRoute.controlTimeOffset -= timeOffset;
		routeManager->requestFullUpdate();
	}

	if (videoWindow->keyIsDown(Qt::Key_Insert))
	{
		defaultRoute.tailLength += timeOffset;
		defaultRoute.tailLength = std::max(0.0, defaultRoute.tailLength);
	}

	if (videoWindow->keyIsDown(Qt::Key_Delete))
	{
		defaultRoute.tailLength -= timeOffset;
		defaultRoute.tailLength = std::max(0.0, defaultRoute.tailLength);
	}
}

ScrollMode InputHandler::getScrollMode() const
{
	return scrollMode;
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
